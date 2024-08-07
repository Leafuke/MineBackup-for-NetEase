#include <iostream> 
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <thread>
#include <sstream>
#include <conio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <windows.h> 
using namespace std;
string Gpath,Gpath2[100];
void sprint(string s,int time)//延迟输出 
{
	int len=s.size();
	for(int i=0;i<len;i++)
	{
		if(s[i]=='*')
		{
			Sleep(100);
			continue;
		}
		printf("%c",s[i]);
		Sleep(time);
	}
}

inline void neglect(int x)//读取忽略 x 行 
{
	int num;
	char ch;
	while(num<x)
	{
		ch=getchar();
		if(ch=='\n') ++num;
	}
	/*string tmp;
	int num=0;
	while(num<x)
	{
		getline(cin,tmp);
		++num;
	 } */
}

bool isDirectory(const std::string& path)//寻找文件夹 
{
#ifdef _WIN32
    DWORD attr = GetFileAttributes(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    DIR* dir = opendir(path.c_str());
    if (dir)
    {
        closedir(dir);
        return true;
    }
    return false;
#endif
}

//列出子文件夹 
void listSubdirectories(const std::string& folderPath, std::vector<std::string>& subdirectories)
{
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((folderPath + "\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
            {
                subdirectories.push_back(findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(folderPath.c_str());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                subdirectories.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
}
//获取文件夹最近修改时间 
string getModificationDate(const std::string& filePath)//Folder modification date
{
    string modificationDate;
#ifdef _WIN32
    WIN32_FIND_DATAA fileData;
    HANDLE hFile = FindFirstFileA(filePath.c_str(), &fileData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FILETIME ft = fileData.ftLastWriteTime;
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

        modificationDate = buffer;

        FindClose(hFile);
    }
#else
    struct stat info;
    if (stat(filePath.c_str(), &info) != 0)
        return modificationDate;

    time_t t = info.st_mtime;
    struct tm* tm = localtime(&t);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

    modificationDate = buffer;
#endif

    return modificationDate;
}
string temp[100];
//列出文件夹内的文件 
void ListFiles(const std::string& folderPath) {
    std::string searchPath = folderPath + "\\*.*";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
	int i=0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            	temp[++i]=findData.cFileName;
                cout<< i << ".  " << findData.cFileName << std::endl;
            }
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }
}
//预读取（直到: 
void Qread()
{
	char ch;
	ch=getchar();
	while(ch!=':') ch=getchar();
	return ;
}
//获取注册表的值 
string GetRegistryValue(const std::string& keyPath, const std::string& valueName)
{
    HKEY hKey;
    string valueData;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		char buffer[1024];
		DWORD dataSize = sizeof(buffer);
		if (RegGetValueA(hKey, NULL, valueName.c_str(), RRF_RT_ANY, NULL, buffer, &dataSize) == ERROR_SUCCESS) {
			valueData = buffer;
		}
		RegCloseKey(hKey);
	}
    return valueData;
}
struct names{
	string real,alias;
	int x;
}name[100];
string rname2[20],Bpath,command,yasuo,lv;//存档真实名 备份文件夹路径 cmd指令 7-Zip路径 压缩等级 
bool prebf,ontop,choice,echos;//回档前备份 工具箱置顶 手动选择 回显 
int limitnum;
HWND hwnd;
struct File {
    string name;
    time_t modifiedTime;
};
//判断文件是否被占用 
bool isFileLocked(const string& filePath)
{
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_SHARE_READ, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        // 如果文件正被另一个进程以独占方式打开，则会返回ERROR_SHARING_VIOLATION错误。
        if (error == ERROR_SHARING_VIOLATION)
            return true;
        // 其他错误情况需要根据实际情况处理
        else
            return false;
    }
    else
    {
        CloseHandle(hFile);
        // 文件成功打开且关闭，表明当前没有被占用（至少在这个瞬间）
        return false;
    }
}

//存档文件夹路径预处理 
int PreSolve(string s)
{
	int len=s.size(),tt=0;
	for(int i=0;i<len;++i)
	{
		if(s[i]=='$')
		{
			++tt;
			continue;
		}
		Gpath2[tt]+=s[i];
	}
	return tt;
}
//检验别名合理性
bool checkupName(string Name)
{
	int len=Name.size();
	for(int i=0;i<len;++i)
		if(Name[i]=='\\' || Name[i]=='/' || Name[i]==':' || Name[i]=='*' || Name[i]=='?' || Name[i]=='"' || Name[i]=='<' || Name[i]=='>' || Name[i]=='|')
			return false;
	return true;
}
//检测备份数量
void checkup(string folderPath,int limit)
{
	if(limit==0) return ;
	DIR* directory = opendir(folderPath.c_str());
    if (!directory) return ;
    File files;
    struct dirent* entry;
    int checknum=0;
    while ((entry = readdir(directory))) {
    	string fileName = entry->d_name;
		string filePath = folderPath + fileName;
		struct stat fileStat;
		stat(filePath.c_str(), &fileStat);
    	if (S_ISREG(fileStat.st_mode)) {
    		++checknum;//如果是常规文件，统计总备份数 
    	}
    }
    closedir(directory);
    struct dirent* entry2;
    while (checknum > limit)
    {
    	directory = opendir(folderPath.c_str());//放外面会造成读取重复，只会删除一次，后面都找不到 
		bool fl=0;
		while ((entry2 = readdir(directory))) {
		    string fileName = entry2->d_name;
		    string filePath = folderPath + fileName;
		    struct stat fileStat;
		    if(!fl) files.modifiedTime=fileStat.st_mtime,fl=1; //重置files 
		    if (stat(filePath.c_str(), &fileStat) != -1) {
		    	if (S_ISREG(fileStat.st_mode)) {
			        File file;
			        file.modifiedTime = fileStat.st_mtime;
					if(file.modifiedTime <= files.modifiedTime)
					{
						files.modifiedTime=file.modifiedTime;
						files.name=fileName;
					}
				}
		    }
		}
		string command="del \"" + folderPath + files.name + "\"";
		system(command.c_str());
		--checknum;
		closedir(directory);
	}
	return ;
}
//备份函数 
//备份函数 
void Backup(int bf,bool echo)
{
	string folderName = Bpath + "\\" + name[bf].alias; // Set folder name
	// Create a folder using mkdir ()
	mkdir(folderName.c_str());
	
	bool isFileLock = 0;
	if(isFileLocked(name[bf].real+"\\region\\r.0.0.mca"))
	{
		isFileLock = true; 
		printf("检测到该存档为打开状态，已在该存档下创建临时文件夹。\n\n正在将存档文件夹中所有文件复制到[1临时文件夹]中，然后开始备份\n此过程中请不要随意点击\n");
		command = "start \"\"  \"" + name[bf].real + "\"";//这样打开不会报错 
		system(command.c_str());
		Sleep(2000);
	    keybd_event(0x11, 0, 0, 0);//Ctrl
		keybd_event(0x41, 0, 0, 0);//A
		keybd_event(0x41, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x43, 0, 0, 0);//C
		keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);
		Sleep(500);
		string folderName2 = name[bf].real + "\\1临时文件夹";
		mkdir(folderName2.c_str());
		command = "start \"\"  \"" + folderName2 + "\"";
		system(command.c_str());
		Sleep(500);
		keybd_event(0x11, 0, 0, 0);//Ctrl
		keybd_event(0x56, 0, 0, 0);//V
		keybd_event(0x56, 0, KEYEVENTF_KEYUP, 0);//V 
		keybd_event(0x11, 0, KEYEVENTF_KEYUP, 0);//Ctrl
		Sleep(2000);
		keybd_event(0x12, 0, 0, 0);//Alt 
		keybd_event(0x53, 0, 0, 0);//Skip
		keybd_event(0x53, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x12, 0, KEYEVENTF_KEYUP, 0);
	    // 获取复制进度窗口的句柄
//	    Sleep(10);
	    HWND hForegroundWindow = GetForegroundWindow();
	    cout << "正在复制..." << endl;
	    // 循环检查窗口是否还存在
	    int sumtime=0;
	    while (true) {
	        // 等待一段时间再检查，避免高CPU占用
	        Sleep(2000); // 等待2秒
	        sumtime+=2;
	        // 检查窗口是否仍然有效 或者 复制太快了根本没抓到窗口 
	        if (!IsWindow(hForegroundWindow) || sumtime > 10)
	        {
	        	cout << "文件复制完成" << endl;
	        	break; // 窗口关闭，退出循环
			}
	    }
		Sleep(1000); 
		keybd_event(0x01, 0, 0, 0);//左键 
		keybd_event(0x01, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x12, 0, 0, 0);//Alt 
		keybd_event(0x73, 0, 0, 0);//F4
		Sleep(100);
		keybd_event(0x01, 0, 0, 0);//左键 
		keybd_event(0x01, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x73, 0, 0, 0);//F4
		keybd_event(0x73, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(0x12, 0, KEYEVENTF_KEYUP, 0);
	}
	else //记录一下备份时区块修改时间，方便以后快速压缩的构建 
	{
		//现在在QuickBackup中才记录 
	}
	
	time_t now = time(0);
    tm *ltm = localtime(&now);
    string com=asctime(ltm),tmp="";
    
    for(int j=0;j<com.size();++j)
    	if(j>=11 && j<=18)
    		if(j==13 || j==16) tmp+="-";
    		else tmp+=com[j];
    tmp="["+tmp+"]"+name[bf].alias;
    
    string Real = name[bf].real;
    if(isFileLock)
    	Real+="\\1临时文件夹";
    
	if(echo) command=yasuo+" a -t7z"+" -mx="+lv+" "+tmp+" \""+Real+"\"\\*";
	else command=yasuo+" a -t7z"+" -mx="+lv+" "+tmp+" \""+Real+"\"\\* > nul 2>&1";
//	cout<< endl << command <<endl;//debug 
	system(command.c_str());
	if(echo) command="move "+tmp+".7z "+folderName;
	else command="move "+tmp+".7z "+folderName+" > nul 2>&1";
	system(command.c_str());
	checkup(folderName+"\\",limitnum);
	
	if(isFileLock)
	{
		command = "rmdir /S /Q \"" + Real + "\"";
		system(command.c_str());
	}
//	freopen("CON","w",stdout);
	return ;
}

//处理可以从.config里读取的存档文件夹名称
string FindName(string target)
{
	/*版本映射
    "1007010": "1.7.10",
    "1008000": "1.8",
    "1008008": "1.8.8",
    "1008009": "1.8.9",
    "1009004": "1.9.4",
    "1010002": "1.10.2",
    "1011002": "1.11.2",
    "1012002": "1.12.2",
    "1013002": "1.13.2",
    "1014003": "1.14.3",
    "1015000": "1.15",
    "1016000": "1.16",
    "1018000": "1.18",
    "100000000": "基岩版"
	*/
	string result="";
	string findpath=GetRegistryValue("Software\\Netease\\MCLauncher", "DownloadPath") + "\\Game";
	std::vector<std::string> subdirectories;
	listSubdirectories(findpath, subdirectories);
	for (const auto& folderName : subdirectories)
	{
		if(folderName[0]!='.') //说明这是邮箱文件夹，但是可能有多个，所以要循环
		{
			if(target[0] == 'L') //说明是联机存档 
				findpath = findpath + "\\" + folderName + "\\LanGameHost";
			else 
				findpath = findpath + "\\" + folderName + "\\MCGame";
			std::vector<std::string> subdirectories2;
			listSubdirectories(findpath, subdirectories2);
			for (const auto& folderName : subdirectories2)//要找遍每一个文件夹 
			{
				findpath = findpath + "\\" + folderName;
				DIR* directory = opendir(findpath.c_str());
			    File files;
			    struct dirent* entry;
			    while ((entry = readdir(directory))) {
			        string fileName = entry->d_name;
			        string filePath = findpath + "\\" + fileName;
			        struct stat fileStat;
			        if (stat(filePath.c_str(), &fileStat) != -1) {
			            if (S_ISREG(fileStat.st_mode)) { // Only regular files are processed
							ifstream file;
							file.open(filePath);
							string s;
							int lines=0;
							while (getline(file, s))
							{
								++lines;
								if(lines==2)
									result+=s;
								if(lines==3)
									result+=s;
								if(lines==5)
								{
									result += "存档版本：";
									for(int i = 17;i < s.size();++i)
									{
										if(s[i] == '0' && s[i-1] != '0')
											result += '.';
										else if(s[i] == '0'){}
										else result += s[i];
									}
									if(result.back() == '.')
										result += "基岩版";
								}
								if(lines==25)
								{
									bool iftarget=true;
									//判断target和当前是否一致 
									for(int j=0;j<target.size();++j){
										if(target[j]!=s[j+11])
										{
											iftarget=false;
											break;
										}
									}
									if(iftarget)
										return result;
									else
										break;
								}
							}
							file.close();
			            }
			        }
			    }
			    closedir(directory);
			}
		}
	}
	freopen("CON","r",stdin);
	return "未匹配原始名称";
}

//初始设置/更新设置 
void SetConfig(string filename, bool ifreset, int summ)
{
	//现在将创建配置文件整合为一个函数 SetConfig() 
	freopen("CON","r",stdin);
	printf("\n正在建立配置文件......\n"); 
	ofstream newFile(filename);
	if(ifreset)
	{
		printf("请输入存档文件夹的储存路径 (多个文件夹路径间用$分隔): ");
		getline(cin,Gpath);
		printf("请输入存档备份存储路径:");
		getline(cin,Bpath);
		summ=PreSolve(Gpath);
	}
	else
	{
		Gpath="";
		for(int i=0;i<summ;++i)
        	Gpath+=Gpath2[i],Gpath+="$";
        Gpath+=Gpath2[summ];
	}	
	
    if (newFile.is_open()) {
    	newFile << "使用的配置文件序号:0" << endl;
    	newFile << "存档文件夹路径:" << Gpath << endl;//new
        newFile << "存档备份存储路径:" << Bpath << endl;
		string keyPath = "Software\\7-Zip"; 
		string valueName = "Path";
		string softw=GetRegistryValue(keyPath, valueName),softww="";
		for(int i=0;i<softw.size();++i)
			if(softw[i]==' ') softww+='"',softww+=' ',softww+='"';
			else softww+=softw[i];
        newFile << "压缩软件路径:" << softww+"7z.exe" << endl;
        newFile << "备份前先还原:0" << endl;
        newFile << "工具箱置顶:0" << endl;
        newFile << "手动选择还原(默认选最新):0" << endl;
        newFile << "过程显示:1" << endl;
        newFile << "压缩等级:5" << endl;
        newFile << "保留的备份数量(0表示不限制):0" << endl; 
	}
	printf("\n有以下存档文件夹:\n\n"); 
	for(int i=0;i<=summ;++i)
	{
		bool ifalias=true; // 是否手动设置别名 
		cout << endl; 
		std::vector<std::string> subdirectories;
		listSubdirectories(Gpath2[i], subdirectories);
	    for (const auto& folderName : subdirectories)
	    {
			std::string NGpath=Gpath2[i]+"\\"+folderName;
	        std::string modificationDate = getModificationDate(NGpath);
	        std::cout << "存档名称: " << folderName << endl;
	        std::cout << "详细信息——" << FindName(folderName) << endl;
	        std::cout << "最近游玩时间: " << modificationDate << endl;
	        std::cout << "-----------" << endl;
	    }
	    Sleep(1000);
	    sprint("你是否希望给所有存档设置别名？(0/1)\n\n",30);
	    cin>>ifalias; 
	    if(ifalias) sprint("接下来，你需要给这些文件夹起一个易于你自己理解的别名。\n\n",30);
		else sprint("那么将自动以存档文件夹名为别名，如果需要修改别名，请在“设置”中手动修改。\n",30);
		for (const auto& folderName : subdirectories)
	    {
	        string alias;
	        B:
	        if(ifalias)
			{
				cout << "请输入以下存档的别名(可以是一段描述) " << folderName << endl;
	        	cin >> alias;
			}
			else alias = folderName;
	        if(!checkupName(alias))
			{
				printf("文件夹名称不能包含符号 \\  /  :  *  ?  \"  <  >  | ，请重新命名");
				goto B;
			}
			newFile << folderName << endl << alias << endl;
	    }
	    newFile << "$" << endl;
	}
    newFile << "*" << endl;
    newFile.close();
    return ;
}



//创建备份文件 
void CreateConfig()
{
	printf("\n你需要创建 (1)一般配置 还是 (2)全自动配置\n\n");
	char ch=getch();
	string folderName,filename = "config1.ini";
	string i="1";
    ifstream file(filename);
    while(true)
    {
    	i[0]+=1;
    	filename="config"+i+".ini";
    	ifstream file(filename);
    	if(!file.is_open()) break;
	}
	if(ch=='1')
	{
		printf("\n正在创建名为 %s 的配置文件\n",&filename[0]);     
    	ofstream newFile(filename);
    	printf("请输入存档文件夹的储存路径 (多个文件夹路径间用$分隔): ");
		getline(cin,Gpath);
		printf("请输入备份存储文件夹路径:");
		getline(cin,Bpath);
		for(int i=0;i<=10;++i)
			Gpath2[i]="";
		int summ=PreSolve(Gpath);
        if (newFile.is_open()) {
        	newFile << "Auto:0" << endl;
        	newFile << "所有存档路径:" << Gpath2[0];
        	if(summ>1) newFile << '$'; 
        	for(int i=1;i<summ;++i)
        		newFile << Gpath2[i] << '$';
        	if(summ!=0) newFile << Gpath2[summ] << endl;
        	else newFile << endl;
            newFile << "备份存储路径:" << Bpath << endl;
			string keyPath = "Software\\7-Zip"; 
			string valueName = "Path";
			string softw=GetRegistryValue(keyPath, valueName),softww="";
			for(int i=0;i<softw.size();++i)
				if(softw[i]==' ') softww+='"',softww+=' ',softww+='"';
				else softww+=softw[i];
            newFile << "压缩软件路径:" << softww+"7z.exe" << endl;
            newFile << "还原前先备份:0" << endl;
            newFile << "工具箱置顶:0" << endl;
            newFile << "手动选择备份:0" << endl;
            newFile << "过程显示:1" << endl;
            newFile << "压缩等级(越高，压缩率越低，但速度越慢):5" << endl;
            newFile << "保留的备份数量(0表示不限制):0" << endl; 
    	}
    	printf("\n有以下存档:\n\n"); 
    	for(int i=0;i<=summ;++i)
    	{
    		cout << endl; 
    		std::vector<std::string> subdirectories;
			listSubdirectories(Gpath2[i], subdirectories);
		    for (const auto& folderName : subdirectories)
		    {
				std::string NGpath=Gpath2[i]+"\\"+folderName;
		        std::string modificationDate = getModificationDate(NGpath);
		        std::cout << "存档名称: " << folderName << endl;
		        std::cout << "详细信息——" << FindName(folderName) << endl;
		        std::cout << "最近游玩时间 " << modificationDate << endl;
		        std::cout << "-----------" << endl;
		    }
		    Sleep(2000);
		    sprint("接下来，你需要给这些文件夹起一个易于你自己理解的别名。\n\n",50);
			for (const auto& folderName : subdirectories)
		    {
		        string alias;
		        cout << "请给以下存档设置别名(可以是一段描述) " << folderName << endl;
		        cin >> alias;
				newFile << folderName << endl << alias << endl;
		    }
		    newFile << "$" << endl;
		}
	    newFile << "*" << endl;
	    newFile.close();
	    sprint("配置文件创建完毕！！！\n",10);
        return ;
	}
	else if(ch=='2')
	{
		ofstream newFile(filename);
		newFile << "AUTO:1" << endl;
		int configs;
		printf("需要调用的配置文件序号(从中获取存档名称和别名):\n");
		cin>>configs;
		newFile << "Use Config:" << configs << endl;
		printf("需要备份第几个存档:");
		cin>>configs;
		newFile << "BF:" << configs << endl;
		printf("你需要 (1)定时备份 还是 (2)间隔备份\n");
		ch=getch();
		if(ch=='1'){
			printf("输入你要在什么时间备份: 1.请输入月份，然后回车(输入0表示每个月):");
			int mon,day,hour,min;
			scanf("%d",&mon);
			printf("2.请输入日期，然后回车(输入0表示每天):");
			scanf("%d",&day);
			printf("3.请输入小时，然后回车(输入0表示每小时):");
			scanf("%d",&hour);
			printf("4.请输入分钟，然后回车:");
			scanf("%d",&min);
			newFile << "Mode:1\nTime:" << mon << " " << day << " " << hour << " " << min << endl;
		} 
		else if(ch=='2'){
			printf("输入你要间隔多少分钟备份:");
			int detime;
			scanf("%d",&detime);
			newFile << "Mode:2\nTime:" << detime << endl;
		}
		else{
			printf("\nError\n");
			return ;
		}
		printf("是否开启免打扰模式(0/1):");
		cin>>configs;
		newFile << "Inter:" << configs << "\n*";
		sprint("配置文件创建完毕！！！\n",10);
		return ;
	}
	else
	{
		printf("\n\nError\n\n");
		return ;
	}
}


//处理路径中的空格 
string QuoteFilePathIfNeeded(const string& filePath) {  
    ostringstream quotedPath;  
    string currentPart;  
    bool inQuotes = false;  
  
    for (size_t i = 0; i < filePath.length(); ++i) {  
        if (filePath[i] == '\\' && !inQuotes) {  
            // 如果当前部分包含空格，并且我们不在引号内，则添加引号  
            if (!currentPart.empty() && currentPart.find(' ') != std::string::npos) {  
                quotedPath << '"' << currentPart << '"';  
            } else {  
                quotedPath << currentPart;  
            }  
            quotedPath << '\\'; // 添加反斜杠  
            currentPart.clear(); // 重置当前部分  
        } else {  
            currentPart += filePath[i]; // 累加当前字符到当前部分  
        }  
  
        // 如果到达字符串末尾，并且当前部分包含空格，且不在引号内，则添加引号  
        if (i == filePath.length() - 1 && !currentPart.empty() && currentPart.find(' ') != std::string::npos && !inQuotes) {  
            quotedPath << '"' << currentPart << '"';  
        }  
  
        // 如果遇到引号，则切换inQuotes状态，并跳过这个引号（假设路径中的引号是不应该出现的）  
        if (filePath[i] == '"' && !inQuotes) {  
            inQuotes = true;  
            // 注意：通常我们不希望路径中包含引号，这里假设路径错误，并忽略这个引号  
            // 如果路径中原本就应该有引号，则需要修改逻辑来正确处理这种情况  
        } else if (filePath[i] == '"' && inQuotes) {  
            inQuotes = false;  
            // 同样，忽略路径中不应该出现的引号  
        }  
    }  
  
    return quotedPath.str();  
}

void Main()
{
//	SetConsoleOutputCP(CP_UTF8);
	string folderName;
	string filename = "config.ini";
    ifstream file(filename);
    if (!file.is_open()) {
    	sprint("-----欢迎使用网易我的世界存档备份程序-----\n",50);
		sprint("           by mc_ortime (本消息在第一次打开时显示)\n",50);
		sprint("声明：本程序基于我的开源代码 Minebackup(MIT协议) 修改而来，禁止商用或恶意使用。\n",10);
		sprint("你可以加入我的世界小工具交流群490861436进行问题交流和反馈\n",10);
		printf("\n开发测试版本 正在建立配置文件......\n"); 
    	ofstream newFile(filename);
//    	printf("请输入存档文件夹的储存路径 (多个文件夹路径间用$分隔): ");
		Gpath=GetRegistryValue("Software\\Netease\\MCLauncher", "DownloadPath")+"\\Game\\.minecraft\\saves$C:\\Users\\" + (string)getenv("USERNAME") + "\\Appdata\\Roaming\\MinecraftPE_Netease\\minecraftWorlds";
		DIR* directory = opendir("D:");
		if(directory)
		{
			Bpath="D:\\MineBackup备份",printf("为您自动设置备份存档存在D盘，若要更改，请以后设置。\n");
			mkdir("D:\\MineBackup备份"); 
		}
		else
		{
			Bpath="C:\\MineBackup备份",printf("为您自动设置备份存档存在C盘，若要更改，请以后设置。\n");
			mkdir("C:\\MineBackup备份"); 
		}
		int summ=PreSolve(Gpath);
        if (newFile.is_open()) {
        	newFile << "使用的配置文件序号:0" << endl;
        	newFile << "存档文件夹路径:" << Gpath << endl;//new
            newFile << "存档备份存储路径:" << Bpath << endl;
			string keyPath = "Software\\Netease\\MCLauncher"; 
			string valueName = "InstallLocation";
			string softw = GetRegistryValue(keyPath, valueName)+"\\ext\\7z\\7z.exe";
			string softww = QuoteFilePathIfNeeded(softw + "\\233");//末尾后莫名其妙少一点 
			//路径中不能包含空格 
			softww.pop_back(); //最后还不能有“\” 
            newFile << "压缩软件路径:" << softww << endl;
            newFile << "备份前先还原:0" << endl;
            newFile << "工具箱置顶:0" << endl;
            newFile << "手动选择还原(默认选最新):0" << endl;
            newFile << "过程显示:1" << endl;
            newFile << "压缩等级:5" << endl;
            newFile << "保留的备份数量(0表示不限制):0" << endl; 
    	}
    	printf("\n有以下存档文件夹:\n\n"); 
    	for(int i=0;i<=summ;++i)
    	{
    		bool ifalias=true;
    		cout << endl; 
    		std::vector<std::string> subdirectories;
			listSubdirectories(Gpath2[i], subdirectories);
		    for (const auto& folderName : subdirectories)
		    {
				std::string NGpath=Gpath2[i]+"\\"+folderName;
		        std::string modificationDate = getModificationDate(NGpath);
		        std::cout << "存档名称: " << folderName << endl;
		        std::cout << "详细信息——" << FindName(folderName) << endl;
		        std::cout << "最近游玩时间: " << modificationDate << endl;
		        std::cout << "-----------" << endl;
		    }
		    Sleep(2000);
		    sprint("你是否希望给所有存档设置别名？(0/1)\n\n",30);
		    cin>>ifalias; 
		    if(ifalias) sprint("接下来，你需要给这些文件夹起一个易于你自己理解的别名。\n\n",30);
			else sprint("那么将自动以存档文件夹名或者读取信息为别名，如果需要修改别名，请在“设置”中手动修改。\n",30);
			for (const auto& folderName : subdirectories)
		    {
		        string alias;
		        B:
		        if(ifalias)
				{
					cout << "请输入以下存档的别名(可以是一段描述) " << folderName << endl;
		        	cin >> alias;
				}
				else
				{
					alias = folderName + "---";
					string temp=FindName(folderName);
					for(int i=15;i<temp.size();++i)
					{
						if(temp[i]=='"') break;
						alias+=temp[i];
					}
				}
		        if(!checkupName(alias))
				{
					printf("文件夹名称不能包含符号 \\  /  :  *  ?  \"  <  >  | ，请重新命名");
					goto B;
				}
				newFile << folderName << endl << alias << endl;
		    }
		    newFile << "$" << endl;
		}
	    newFile << "*" << endl;
	    newFile.close();
	    printf("结束！可以关闭程序！\n");
        return ;
    }
    
    
    
    freopen("config.ini","r",stdin);
    Qread();
    string temps,tempss;
    int configs;
    char configss[100];
	scanf("%d",&configs);
	if(configs!=0)
	{
		temps=to_string(configs); //only c++11
		temps="config"+temps+".ini";
		strcpy(configss,temps.c_str());
		freopen(configss,"r",stdin);
		Qread();
		int auto1;
		cin>>auto1;
		if(auto1)
		{
			int mode,usecf,bfnum,ifinter;
			Qread();
			cin>>usecf;
			Qread();
			cin>>bfnum;
			Qread();
			cin>>mode;
			int bftime,month,day,hour,min;
			if(mode==1)
			{
				Qread(); 
				scanf("%d %d %d %d",&month,&day,&hour,&min);//这里如果读入错误，后面就正确…… 
				//2023.12.31解决，配置文件后面多一串东西就行 
			}
			else if(mode==2)
			{
				Qread();
				cin>>bftime;
			}
			Qread();
			cin>>ifinter;
			if(usecf==0) // 使用一般配置中的存档路径 
			{
				freopen("config.ini","r",stdin);
//				getline(cin,tmp1);// Problem Here
				neglect(1); 
				Qread();
				char inputs[1000];
				for(int i=0;;++i)
				{
					inputs[i]=getchar();
					if(inputs[i]=='\n'){
						inputs[i]='\0';break;
					}
				}
				tempss=inputs;
			    int summ=PreSolve(tempss);
			    Qread();
			    memset(inputs,'\0',sizeof(inputs));
			    for(int i=0;;++i)
				{
					inputs[i]=getchar();
					if(inputs[i]=='\n'){
						inputs[i]='\0';break;
					}
				}
				Bpath=inputs;
				Qread();
				memset(inputs,'\0',sizeof(inputs));
			    for(int i=0;;++i)
				{
					inputs[i]=getchar();
					if(inputs[i]=='\n'){
						inputs[i]='\0';break;
					}
				}
				yasuo=inputs;
				neglect(4);
				Qread();
				memset(inputs,'\0',sizeof(inputs));
				inputs[0]=getchar();
				lv=inputs;
				Qread();
				cin>>limitnum;
			    int i=0,ttt=0;//存档数量 存档所在存档文件夹序号 
			    inputs[0]=getchar();// addition
			    while(true)
			    {
					memset(inputs,'\0',sizeof(inputs));
					for(int i=0;;++i)
					{
						inputs[i]=getchar();
						if(inputs[i]=='\n'){
							inputs[i]='\0';break;
						}
					}
					name[++i].real=inputs;
			    	if(name[i].real[0]=='*') break;
			    	else if(name[i].real[0]=='$')
			    	{
			    		++ttt,--i;
			    		continue;
					}
			    	name[i].real=Gpath2[ttt]+"\\"+name[i].real;
			    	name[i].x=ttt;
			    	memset(inputs,'\0',sizeof(inputs));
					for(int i=0;;++i)
					{
						inputs[i]=getchar();
						if(inputs[i]=='\n'){
							inputs[i]='\0';break;
						}
					}
					name[i].alias=inputs;
				}
				//检测存档文件夹是否有更新 
				int ix=0;
				bool ifnew=0; 
				for(int i=0;i<=summ;++i)
				{
					std::vector<std::string> subdirectories;
					listSubdirectories(Gpath2[i], subdirectories);
				    for (const auto& folderName : subdirectories)
				    {
				    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //与实际一致则不更新 注意，这里的检测是有缺陷的 
				    		continue;
						--ix;//因为多出一个，所以比对时-1，但未能处理减少情况 
				    	//不一致，则列出
				    	ifnew=true;
						printf("\n检测到新的存档如下：\n");
						string NGpath=Gpath2[i]+"\\"+folderName;
				        string modificationDate = getModificationDate(NGpath);
				        cout << "存档名称: " << folderName << endl;
				        std::cout << "详细信息——" << FindName(folderName) << endl;
				        cout << "最近游玩时间: " << modificationDate << endl;
				        cout << "-----------" << endl;
				    }
				}
				if(ifnew) //如果有更新，询问是否更新配置文件
				{
					printf("请问是否更新配置文件？(0/1)\n");
					char ch;
					ch=getch();
					printf("\n请手动更新，在对应位置以后添加新存档的“真实名”和“别名”\n"); 
					if(ch=='1')
						system("start config.ini");
				}
			}
			else
			{
				string temps=to_string(usecf);
			    char configss[10];
				temps="config"+temps+".ini";
				for(int i=0;i<temps.size();++i)
					configss[i]=temps[i];
				freopen(configss,"r",stdin);
				string tmp;
				getline(cin,tmp);
				Qread();
			    getline(cin,temps);
			    int summ=PreSolve(temps);
			    Qread();
			    getline(cin,Bpath);
			    Qread();
			    getline(cin,yasuo);
			    neglect(4);
				Qread();
				getline(cin,lv);
				Qread();
				cin>>limitnum;
			    int i=0;
			    int ttt=0;
			    while(true)
			    {
			    	getline(cin,name[++i].real);
			    	if(name[i].real[0]=='*') break;
			    	else if(name[i].real[0]=='$')
			    	{
			    		++ttt,--i;
			    		continue;
					}
			    	name[i].real=Gpath2[ttt]+"\\"+name[i].real;
			    	name[i].x=ttt;
			    	getline(cin,name[i].alias);
				}
				//检测存档文件夹是否有更新 
				int ix=0;
				bool ifnew=0; 
				for(int i=0;i<=summ;++i)
				{
					std::vector<std::string> subdirectories;
					listSubdirectories(Gpath2[i], subdirectories);
				    for (const auto& folderName : subdirectories)
				    {
				    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //与实际一致则不更新 注意，这里的检测是有缺陷的 
				    		continue;
						--ix;//因为多出一个，所以比对时-1，但未能处理减少情况 
				    	//不一致，则列出
				    	ifnew=true;
						printf("\n检测到新的存档如下：\n");
						string NGpath=Gpath2[i]+"\\"+folderName;
				        string modificationDate = getModificationDate(NGpath);
				        cout << "存档名称: " << folderName << endl;
				        std::cout << "详细信息——" << FindName(folderName) << endl;
				        cout << "最近游玩时间: " << modificationDate << endl;
				        cout << "-----------" << endl;
				    }
				}
				if(ifnew) //如果有更新，询问是否更新配置文件
				{
					printf("请问是否更新配置文件？(0/1)\n");
					char ch;
					ch=getch();
					printf("\n请手动更新，在对应位置以后添加新存档的“真实名”和“别名”\n"); 
					string command="start "+temps;
					if(ch=='1')
						system(command.c_str());
				}
			}
			if(mode==1)
			{
				while(true)
				{
					// 获取当前时间
				    std::time_t now = std::time(nullptr);
				    std::tm* local_time = std::localtime(&now);
				
				    std::tm target_time = {0};
				    target_time.tm_year = local_time->tm_year; // 年份从1900年开始计算
				    if(month==0)  target_time.tm_mon = local_time->tm_mon;
				    else target_time.tm_mon = month - 1; // 月份从0开始计算
				    if(day==0) target_time.tm_mday = local_time->tm_mday;
				    else target_time.tm_mday = day;
				    if(hour==0) target_time.tm_hour = local_time->tm_hour;
				    else target_time.tm_hour = hour;
				    if(min==0) target_time.tm_min = local_time->tm_min;
				    else target_time.tm_min = min;
				    // 如果当前时间已经超过了目标时间，那么就不需要等待了
				    A: 
				    if (std::mktime(local_time) > std::mktime(&target_time)) {
				        std::cout << "现在的时间已经超过了目标时间" << std::endl;
						if(min!=0 && hour!=0 && day!=0 && month!=0) return ;
						if(min==0)
						{
							++target_time.tm_min;
							if(std::mktime(local_time) <= std::mktime(&target_time)) goto A ;
							--target_time.tm_min;
						}
						if(hour==0)
						{
							++target_time.tm_hour;
							if(std::mktime(local_time) <= std::mktime(&target_time)) goto A ;
							--target_time.tm_hour;
						}
						if(day==0)
						{
							++target_time.tm_mday;
							if(std::mktime(local_time) <= std::mktime(&target_time)) goto A ;
							--target_time.tm_mday;
						}
						if(month==0)
						{
							++target_time.tm_mon;
							if(std::mktime(local_time) <= std::mktime(&target_time)) goto A ;
							--target_time.tm_mon;
						}						
				    } else {
				        // 计算需要等待的时间（单位：秒）
				        std::time_t wait_time = std::difftime(std::mktime(&target_time), std::mktime(local_time));
				        // 等待指定的时间
				        std::this_thread::sleep_for(std::chrono::seconds(wait_time));
				        Backup(bfnum,false);
				    }
				}
			} 
			else if(mode==2)
			{
				while(true)
				{
				    // 让线程休眠指定的时间
				    std::this_thread::sleep_for(std::chrono::seconds(60*bftime));
				    Backup(bfnum,false);
				}
			}
		}
	}
    Qread();
    getline(cin,temps);
    int summ=PreSolve(temps);
    Qread();
    getline(cin,Bpath);
    Qread();
    getline(cin,yasuo);
    Qread();
    cin>>prebf;
    Qread();
    cin>>ontop;
    Qread();
	cin>>choice;
	Qread();
	cin>>echos;
	Qread();
	getline(cin,lv);
	Qread();
	cin>>limitnum;
    int i=0,ttt=0;//存档数量 存档所在存档文件夹序号 
    printf("有以下存档:\n\n"); 
    char ch=getchar();//DEBUG because getline ...//bug why?now ok?
    while(true)
    {
    	getline(cin,name[++i].real);
    	if(name[i].real[0]=='*') break;
    	else if(name[i].real[0]=='$')
    	{
    		++ttt,--i;
    		continue;
		}
    	name[i].real=Gpath2[ttt]+"\\"+name[i].real;
    	name[i].x=ttt;
    	getline(cin,name[i].alias);
    	printf("%d. ",i);
    	cout<<name[i].alias<<endl;
	}
	freopen("CON","r",stdin);
	//检测存档文件夹是否有更新 
	int ix=0;
	bool ifnew=0; 
	for(int i=0;i<=summ;++i)
	{
		std::vector<std::string> subdirectories;
		listSubdirectories(Gpath2[i], subdirectories);
	    for (const auto& folderName : subdirectories)
	    {
	    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //与实际一致则不更新 注意，这里的检测是有缺陷的 
	    		continue;
			--ix;//因为多出一个，所以比对时-1，但未能处理减少情况 
	    	//不一致，则列出
	    	ifnew=true;
			printf("\n检测到新的存档如下：\n");
			string NGpath=Gpath2[i]+"\\"+folderName;
	        string modificationDate = getModificationDate(NGpath);
	        cout << "存档名称: " << folderName << endl;
	        std::cout << "详细信息——" << FindName(folderName) << endl;
	        cout << "最近游玩时间: " << modificationDate << endl;
	        cout << "-----------" << endl;
	    }
	}
	if(ifnew) //如果有更新，询问是否更新配置文件
	{
		printf("请问是否更新配置文件？(0/1)\n");
		char ch;
		ch=getch();
		printf("\n请手动更新，在对应位置以后添加新存档的“真实名”和“别名”\n"); 
		if(ch=='1')
			system("start config.ini");
	}
	while(true)
	{
		printf("请问你要 (1)备份存档 (2)回档 (3)更新存档 (4) 自动备份 还是 (5) 创建配置文件 呢？ (按 1/2/3/4/5)\n");
		char ch;
		ch=getch();
		if(ch=='1')
		{
			printf("输入存档前的序号来完成备份:");
			int bf;
			scanf("%d",&bf);
			Backup(bf,echos);
			sprint("\n\n备份完成! ! !\n\n",40);
		}
		else if(ch=='2')
		{
			int i=0;
		    if(!choice) printf("输入存档前的序号来完成还原 (模式: 选取最新备份) : ");
		    else printf("输入存档前的序号来完成还原 (模式: 手动选择) :");
			int bf;
			scanf("%d",&bf);
			string folderPath=Bpath+"\\"+name[bf].alias+"\\";
			DIR* directory = opendir(folderPath.c_str());
		    if (!directory) {
		        printf("备份不存在，无法还原！你可以还没有进行过备份\n");
		        return ;
		    }
		    File files;
		    if(!choice)//Look for the latest backup
		    {
			    struct dirent* entry;
			    while ((entry = readdir(directory))) {
			        string fileName = entry->d_name;
			        string filePath = folderPath + fileName;
			        struct stat fileStat;
			        if (stat(filePath.c_str(), &fileStat) != -1) {
			            if (S_ISREG(fileStat.st_mode)) { // Only regular files are processed
			                File file;
			                file.name = fileName;
			                file.modifiedTime = fileStat.st_mtime;
							if(file.modifiedTime>files.modifiedTime)
							{
								files.modifiedTime=file.modifiedTime;
								files.name=file.name;
							}
								
			            }
			        }
			    }
			    closedir(directory);
			}
			else
			{
				string folderName2 = Bpath + "\\" + name[bf].alias;
				printf("以下是备份存档\n\n");
				ListFiles(folderName2);
				printf("输入备份前的序号来完成还原:");
				int bf2;
				scanf("%d",&bf2);
				files.name=temp[bf2];
			}
		    if(prebf)
		    	Backup(bf,false);
			command=yasuo+" x "+Bpath+"\\"+name[bf].alias+"\\"+files.name+" -o"+name[bf].real+" -y";
			system(command.c_str());
			sprint("\n\n还原成功! ! !\n\n",40);
		}
		else if(ch=='3')
		{
			freopen("CON","r",stdin);
        	ofstream cfile("config.ini");
        	cfile << "使用的配置文件序号:0\n";
        	cfile << "存档文件夹路径:" << Gpath2[0] << '$';
        	for(int i=1;i<summ;++i)
        		cfile << Gpath2[i] << '$';
        	cfile << Gpath2[summ] << endl;
        	cfile << "存档备份存储路径:" << Bpath << endl;
			string keyPath = "Software\\7-Zip"; 
			string valueName = "Path";
            cfile << "压缩软件路径:" << GetRegistryValue(keyPath, valueName)+"7z.exe" << endl;
            cfile << "备份前还原:" << prebf << endl;
            cfile << "工具箱置顶:" << ontop << endl;
            cfile << "手动选择备份:" << choice << endl;
            cfile << "过程显示:" << echos << endl;
            cfile << "压缩等级:" << lv << endl;
            cfile << "保留的备份数量(0表示不限制):" << limitnum << endl; 
        	printf("\n在文件夹中有以下存档: \n\n"); 
	    	for(int i=0;i<=summ;++i)
	    	{
	    		vector<string> subdirectories;
				listSubdirectories(Gpath2[i], subdirectories);
			    for (const auto& folderName : subdirectories)
			    {
					string NGpath=Gpath2[i]+"\\"+folderName;
			        string modificationDate = getModificationDate(NGpath);
			        cout << "存档名称: " << folderName << endl;
			        cout << "详细信息——" << FindName(folderName) << endl;
			        cout << "最近游玩时间: " << modificationDate << endl;
			        cout << "-----------" << endl;
			    }
			    Sleep(2000);
			    sprint("接下来，你需要给这些文件夹起一个易于你自己理解的别名。\n\n",50);
				for (const auto& folderName : subdirectories)
			    {
			        string alias;
			        cout << "请输入以下存档的别名(可以是一段描述) " << endl << folderName;
			        cin >> alias;
					cfile << folderName << endl << alias << endl;
			    }
			    cfile << "$" << endl;
			}
		    cfile << "*" << endl;
	    	puts("\n\n更新存档完成\n\n");
	    	cfile.close(); 
		}
		else if(ch=='4')
		{
			printf("请输入你要备份的存档序号:");
			int bf,tim;
			scanf("%d",&bf);
			printf("每隔几分钟进行备份: ");
			scanf("%d",&tim);
			printf("已进入自动备份模式，每隔 %d 分钟进行备份",tim);
			while(true)
			{
				Backup(bf,false);
				Sleep(tim*60000);
			}
		}
		else if(ch=='5')
		{
			CreateConfig();
		}
		else printf("请按键盘上的 1/2/3/4/5 键\n\n");
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

bool isRunning = true;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        int buttonId = LOWORD(wParam);

        // Handle button click events
        switch (buttonId)
        {
	        case 1:
        	{
        		int x=0;
	        	while(!Gpath2[x].empty())
	        	{
	        		command="start "+Gpath2[x++];
					system(command.c_str());
				}
	            break;
			}
	        case 2:
	            command="start "+Bpath;
				system(command.c_str());
	            break;
	        case 3:
	            command="start config.ini";
				system(command.c_str());
	            break;
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND h=GetForegroundWindow();
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ButtonWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                          
        wc.lpszClassName,          
        "Toolbox",                  
        WS_OVERLAPPEDWINDOW,          
        CW_USEDEFAULT, CW_USEDEFAULT, 
        480, 85,              
        NULL,                     
        NULL,                     
        hInstance,
        NULL
    );
    CreateWindow("button", "打开存档文件夹",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 10, 120, 35,
        hwnd, (HMENU)1, hInstance, NULL);

    CreateWindow("button", "打开备份文件夹",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        150, 10, 120, 35,
        hwnd, (HMENU)2, hInstance, NULL);

    CreateWindow("button", "设置",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        280, 10, 80, 35,
        hwnd, (HMENU)3, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);    
    std::thread MainThread(Main);
    MSG msg = {};
    
    //线程休眠，为了等待ontop读取完毕 
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if(ontop)
		SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);//Top the window
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    isRunning = false;
    MainThread.join();
    return 0;
}
