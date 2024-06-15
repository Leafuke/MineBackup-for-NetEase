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
void sprint(string s,int time)//�ӳ���� 
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

inline void neglect(int x)//��ȡ���� x �� 
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

bool isDirectory(const std::string& path)//Ѱ���ļ��� 
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

//�г����ļ��� 
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
//��ȡ�ļ�������޸�ʱ�� 
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
//�г��ļ����ڵ��ļ� 
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
//Ԥ��ȡ��ֱ��: 
void Qread()
{
	char ch;
	ch=getchar();
	while(ch!=':') ch=getchar();
	return ;
}
//��ȡע����ֵ 
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
string rname2[20],Bpath,command,yasuo,lv;//�浵��ʵ�� �����ļ���·�� cmdָ�� 7-Zip·�� ѹ���ȼ� 
bool prebf,ontop,choice,echos;//�ص�ǰ���� �������ö� �ֶ�ѡ�� ���� 
int limitnum;
HWND hwnd;
struct File {
    string name;
    time_t modifiedTime;
};
//�ж��ļ��Ƿ�ռ�� 
bool isFileLocked(const string& filePath)
{
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_SHARE_READ, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        // ����ļ�������һ�������Զ�ռ��ʽ�򿪣���᷵��ERROR_SHARING_VIOLATION����
        if (error == ERROR_SHARING_VIOLATION)
            return true;
        // �������������Ҫ����ʵ���������
        else
            return false;
    }
    else
    {
        CloseHandle(hFile);
        // �ļ��ɹ����ҹرգ�������ǰû�б�ռ�ã����������˲�䣩
        return false;
    }
}

//�浵�ļ���·��Ԥ���� 
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
//�������������
bool checkupName(string Name)
{
	int len=Name.size();
	for(int i=0;i<len;++i)
		if(Name[i]=='\\' || Name[i]=='/' || Name[i]==':' || Name[i]=='*' || Name[i]=='?' || Name[i]=='"' || Name[i]=='<' || Name[i]=='>' || Name[i]=='|')
			return false;
	return true;
}
//��ⱸ������
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
    		++checknum;//����ǳ����ļ���ͳ���ܱ����� 
    	}
    }
    closedir(directory);
    struct dirent* entry2;
    while (checknum > limit)
    {
    	directory = opendir(folderPath.c_str());//���������ɶ�ȡ�ظ���ֻ��ɾ��һ�Σ����涼�Ҳ��� 
		bool fl=0;
		while ((entry2 = readdir(directory))) {
		    string fileName = entry2->d_name;
		    string filePath = folderPath + fileName;
		    struct stat fileStat;
		    if(!fl) files.modifiedTime=fileStat.st_mtime,fl=1; //����files 
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
//���ݺ��� 
void Backup(int bf,bool echo)
{
	string folderName = Bpath + "\\" + name[bf].alias; // Set folder name
	// Create a folder using mkdir ()
	mkdir(folderName.c_str());
	bool isFileLock=0;
	
	if(isFileLocked(name[bf].real+"\\region\\r.0.0.mca"))
	{
		printf("��⵽�ô浵Ϊ��״̬�����ڸô浵�´�����ʱ�ļ��С�\n\n���Ƚ��浵�ļ����������ļ����Ƶ�[1��ʱ�ļ���]�У�\nȻ���� 1/0 ����ʼ/ȡ������\n");
		string folderName2 = name[bf].real + "\\1��ʱ�ļ���";
		mkdir(folderName2.c_str());
		command = "start " + name[bf].real;
		system(command.c_str());
		char ch;
		ch=getch();
		if(ch=='1')
			isFileLock=1;
		else return ;
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
    	Real+="\\1��ʱ�ļ���"; 
    
	if(echo) command=yasuo+" a -t7z -mx="+lv+" "+tmp+" \""+Real+"\"\\*";
	else command=yasuo+" a -t7z -bd -mx="+lv+" "+tmp+" \""+Real+"\"\\* > nul 2>&1";
	cout<< endl << command <<endl;//debug 
	system(command.c_str());
	if(echo) command="move "+tmp+".7z "+folderName;
	else command="move "+tmp+".7z "+folderName+" > nul 2>&1";
	system(command.c_str());
	checkup(folderName+"\\",limitnum);
	
	if(isFileLock)
	{
		command = "rmdir /S /Q " + Real;
		system(command.c_str());
	}
	
	return ;
}

//����ط��ػ������� 
string GetSpPath(string sp)
{
	freopen("temp","w",stdout);
	system(sp.c_str());
	ifstream tmp("temp");
	freopen("CON","w",stdout);
	string ans;
	getline(tmp, ans, '\n');
//    system("del temp"); �����ᵼ��freopen���� 
	return ans;
}

//��ʼ����/�������� 
void SetConfig(string filename, bool ifreset, int summ)
{
	//���ڽ����������ļ�����Ϊһ������ SetConfig() 
	freopen("CON","r",stdin);
	printf("\n���ڽ��������ļ�......\n"); 
	ofstream newFile(filename);
	if(ifreset)
	{
		printf("������浵�ļ��еĴ���·�� (����ļ���·������$�ָ�): ");
		getline(cin,Gpath);
		printf("������浵���ݴ洢·��:");
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
    	newFile << "ʹ�õ������ļ����:0" << endl;
    	newFile << "�浵�ļ���·��:" << Gpath << endl;//new
        newFile << "�浵���ݴ洢·��:" << Bpath << endl;
		string keyPath = "Software\\7-Zip"; 
		string valueName = "Path";
		string softw=GetRegistryValue(keyPath, valueName),softww="";
		for(int i=0;i<softw.size();++i)
			if(softw[i]==' ') softww+='"',softww+=' ',softww+='"';
			else softww+=softw[i];
        newFile << "ѹ�����·��:" << softww+"7z.exe" << endl;
        newFile << "����ǰ�Ȼ�ԭ:0" << endl;
        newFile << "�������ö�:0" << endl;
        newFile << "�ֶ�ѡ��ԭ(Ĭ��ѡ����):0" << endl;
        newFile << "������ʾ:1" << endl;
        newFile << "ѹ���ȼ�:5" << endl;
        newFile << "�����ı�������(0��ʾ������):0" << endl; 
	}
	printf("\n�����´浵�ļ���:\n\n"); 
	for(int i=0;i<=summ;++i)
	{
		bool ifalias=true; // �Ƿ��ֶ����ñ��� 
		cout << endl; 
		std::vector<std::string> subdirectories;
		listSubdirectories(Gpath2[i], subdirectories);
	    for (const auto& folderName : subdirectories)
	    {
			std::string NGpath=Gpath2[i]+"\\"+folderName;
	        std::string modificationDate = getModificationDate(NGpath);
	        std::cout << "�浵����: " << folderName << endl;
	        std::cout << "�������ʱ��: " << modificationDate << endl;
	        std::cout << "-----------" << endl;
	    }
	    Sleep(1000);
	    sprint("���Ƿ�ϣ�������д浵���ñ�����(0/1)\n\n",30);
	    cin>>ifalias; 
	    if(ifalias) sprint("������������Ҫ����Щ�ļ�����һ���������Լ����ı�����\n\n",30);
		else sprint("��ô���Զ��Դ浵�ļ�����Ϊ�����������Ҫ�޸ı��������ڡ����á����ֶ��޸ġ�\n",30);
		for (const auto& folderName : subdirectories)
	    {
	        string alias;
	        B:
	        if(ifalias)
			{
				cout << "���������´浵�ı���(������һ������) " << folderName << endl;
	        	cin >> alias;
			}
			else alias = folderName;
	        if(!checkupName(alias))
			{
				printf("�ļ������Ʋ��ܰ������� \\  /  :  *  ?  \"  <  >  | ������������");
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



//���������ļ� 
void CreateConfig()
{
	printf("\n����Ҫ���� (1)һ������ ���� (2)ȫ�Զ�����\n\n");
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
		printf("\n���ڴ�����Ϊ %s �������ļ�\n",&filename[0]);     
    	ofstream newFile(filename);
    	printf("������浵�ļ��еĴ���·�� (����ļ���·������$�ָ�): ");
		getline(cin,Gpath);
		printf("�����뱸�ݴ洢�ļ���·��:");
		getline(cin,Bpath);
		for(int i=0;i<=10;++i)
			Gpath2[i]="";
		int summ=PreSolve(Gpath);
        if (newFile.is_open()) {
        	newFile << "Auto:0" << endl;
        	newFile << "���д浵·��:" << Gpath2[0];
        	if(summ>1) newFile << '$'; 
        	for(int i=1;i<summ;++i)
        		newFile << Gpath2[i] << '$';
        	if(summ!=0) newFile << Gpath2[summ] << endl;
        	else newFile << endl;
            newFile << "���ݴ洢·��:" << Bpath << endl;
			string keyPath = "Software\\7-Zip"; 
			string valueName = "Path";
			string softw=GetRegistryValue(keyPath, valueName),softww="";
			for(int i=0;i<softw.size();++i)
				if(softw[i]==' ') softww+='"',softww+=' ',softww+='"';
				else softww+=softw[i];
            newFile << "ѹ�����·��:" << softww+"7z.exe" << endl;
            newFile << "��ԭǰ�ȱ���:0" << endl;
            newFile << "�������ö�:0" << endl;
            newFile << "�ֶ�ѡ�񱸷�:0" << endl;
            newFile << "������ʾ:1" << endl;
            newFile << "ѹ���ȼ�(Խ�ߣ�ѹ����Խ�ͣ����ٶ�Խ��):5" << endl;
            newFile << "�����ı�������(0��ʾ������):0" << endl; 
    	}
    	printf("\n�����´浵:\n\n"); 
    	for(int i=0;i<=summ;++i)
    	{
    		cout << endl; 
    		std::vector<std::string> subdirectories;
			listSubdirectories(Gpath2[i], subdirectories);
		    for (const auto& folderName : subdirectories)
		    {
				std::string NGpath=Gpath2[i]+"\\"+folderName;
		        std::string modificationDate = getModificationDate(NGpath);
		        std::cout << "�浵����: " << folderName << endl;
		        std::cout << "�������ʱ�� " << modificationDate << endl;
		        std::cout << "-----------" << endl;
		    }
		    Sleep(2000);
		    sprint("������������Ҫ����Щ�ļ�����һ���������Լ����ı�����\n\n",50);
			for (const auto& folderName : subdirectories)
		    {
		        string alias;
		        cout << "������´浵���ñ���(������һ������) " << folderName << endl;
		        cin >> alias;
				newFile << folderName << endl << alias << endl;
		    }
		    newFile << "$" << endl;
		}
	    newFile << "*" << endl;
	    newFile.close();
	    sprint("�����ļ�������ϣ�����\n",10);
        return ;
	}
	else if(ch=='2')
	{
		ofstream newFile(filename);
		newFile << "AUTO:1" << endl;
		int configs;
		printf("��Ҫ���õ������ļ����(���л�ȡ�浵���ƺͱ���):\n");
		cin>>configs;
		newFile << "Use Config:" << configs << endl;
		printf("��Ҫ���ݵڼ����浵:");
		cin>>configs;
		newFile << "BF:" << configs << endl;
		printf("����Ҫ (1)��ʱ���� ���� (2)�������\n");
		ch=getch();
		if(ch=='1'){
			printf("������Ҫ��ʲôʱ�䱸��: 1.�������·ݣ�Ȼ��س�(����0��ʾÿ����):");
			int mon,day,hour,min;
			scanf("%d",&mon);
			printf("2.���������ڣ�Ȼ��س�(����0��ʾÿ��):");
			scanf("%d",&day);
			printf("3.������Сʱ��Ȼ��س�(����0��ʾÿСʱ):");
			scanf("%d",&hour);
			printf("4.��������ӣ�Ȼ��س�:");
			scanf("%d",&min);
			newFile << "Mode:1\nTime:" << mon << " " << day << " " << hour << " " << min << endl;
		} 
		else if(ch=='2'){
			printf("������Ҫ������ٷ��ӱ���:");
			int detime;
			scanf("%d",&detime);
			newFile << "Mode:2\nTime:" << detime << endl;
		}
		else{
			printf("\nError\n");
			return ;
		}
		printf("�Ƿ��������ģʽ(0/1):");
		cin>>configs;
		newFile << "Inter:" << configs << "\n*";
		sprint("�����ļ�������ϣ�����\n",10);
		return ;
	}
	else
	{
		printf("\n\nError\n\n");
		return ;
	}
}

//������Դ�.config���ȡ�Ĵ浵�ļ�������
string FindName(string target)
{
	string result="";
	string findpath=GetRegistryValue("Software\\Netease\\MCLauncher", "DownloadPath") + "\\Game";
	std::vector<std::string> subdirectories;
	listSubdirectories(findpath, subdirectories);
	for (const auto& folderName : subdirectories)
	{
		if(folderName[0]!='.')
		{
			findpath = findpath + "\\" + folderName + "\\LanGameHost";
			break;
		}
	}
	std::vector<std::string> subdirectories2;
	listSubdirectories(findpath, subdirectories2);
	for (const auto& folderName : subdirectories2)
	{
		findpath = findpath + "\\" + folderName;
	}
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
					if(lines==25)
					{
						bool iftarget=true;
						//�ж�target�͵�ǰ�Ƿ�һ�� 
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
							return "δƥ��ԭʼ����";
					}
				}
				file.close();
            }
        }
    }
    closedir(directory);
	freopen("CON","r",stdin);
	return "δƥ��ԭʼ����";
}

//����·���еĿո� 
string QuoteFilePathIfNeeded(const string& filePath) {  
    ostringstream quotedPath;  
    string currentPart;  
    bool inQuotes = false;  
  
    for (size_t i = 0; i < filePath.length(); ++i) {  
        if (filePath[i] == '\\' && !inQuotes) {  
            // �����ǰ���ְ����ո񣬲������ǲ��������ڣ����������  
            if (!currentPart.empty() && currentPart.find(' ') != std::string::npos) {  
                quotedPath << '"' << currentPart << '"';  
            } else {  
                quotedPath << currentPart;  
            }  
            quotedPath << '\\'; // ��ӷ�б��  
            currentPart.clear(); // ���õ�ǰ����  
        } else {  
            currentPart += filePath[i]; // �ۼӵ�ǰ�ַ�����ǰ����  
        }  
  
        // ��������ַ���ĩβ�����ҵ�ǰ���ְ����ո��Ҳ��������ڣ����������  
        if (i == filePath.length() - 1 && !currentPart.empty() && currentPart.find(' ') != std::string::npos && !inQuotes) {  
            quotedPath << '"' << currentPart << '"';  
        }  
  
        // ����������ţ����л�inQuotes״̬��������������ţ�����·���е������ǲ�Ӧ�ó��ֵģ�  
        if (filePath[i] == '"' && !inQuotes) {  
            inQuotes = true;  
            // ע�⣺ͨ�����ǲ�ϣ��·���а������ţ��������·�����󣬲������������  
            // ���·����ԭ����Ӧ�������ţ�����Ҫ�޸��߼�����ȷ�����������  
        } else if (filePath[i] == '"' && inQuotes) {  
            inQuotes = false;  
            // ͬ��������·���в�Ӧ�ó��ֵ�����  
        }  
    }  
  
    return quotedPath.str();  
}

void Main()
{
	string folderName;
	string filename = "config.ini";
    ifstream file(filename);
    if (!file.is_open()) {
    	sprint("-----��ӭʹ�������ҵ�����浵���ݳ���-----\n",50);
		sprint("           by mc_ortime (����Ϣ�ڵ�һ�δ�ʱ��ʾ)\n",50);
		sprint("����������������ҵĿ�Դ���� Minebackup(MITЭ��) �޸Ķ�������ֹ���û����ʹ�á�\n",10);
		sprint("����Լ����ҵ�����С���߽���Ⱥ490861436�������⽻���ͷ���\n",10);
		printf("\n�������԰汾 ���ڽ��������ļ�......\n"); 
    	ofstream newFile(filename);
//    	printf("������浵�ļ��еĴ���·�� (����ļ���·������$�ָ�): ");
		Gpath=GetRegistryValue("Software\\Netease\\MCLauncher", "DownloadPath")+"/Game/.minecraft/saves$"+GetSpPath("echo %APPDATA%")+"/MinecraftPE_Netease/minecraftWorlds";
		DIR* directory = opendir("D:");
		if(directory)
		{
			Bpath="D:\\MineBackup����",printf("Ϊ���Զ����ñ��ݴ浵����D�̣���Ҫ���ģ����Ժ����á�\n");
			mkdir("D:\\MineBackup����"); 
		}
		else
		{
			Bpath="C:\\MineBackup����",printf("Ϊ���Զ����ñ��ݴ浵����C�̣���Ҫ���ģ����Ժ����á�\n");
			mkdir("C:\\MineBackup����"); 
		}
		int summ=PreSolve(Gpath);
        if (newFile.is_open()) {
        	newFile << "ʹ�õ������ļ����:0" << endl;
        	newFile << "�浵�ļ���·��:" << Gpath << endl;//new
            newFile << "�浵���ݴ洢·��:" << Bpath << endl;
			string keyPath = "Software\\Netease\\MCLauncher"; 
			string valueName = "InstallLocation";
			string softw = GetRegistryValue(keyPath, valueName)+"\\ext\\7z\\7z.exe";
			string softww = QuoteFilePathIfNeeded(softw + "\\233");//ĩβ��Ī��������һ�� 
			//·���в��ܰ����ո� 
			softww.pop_back(); //��󻹲����С�\�� 
            newFile << "ѹ�����·��:" << softww << endl;
            newFile << "����ǰ�Ȼ�ԭ:0" << endl;
            newFile << "�������ö�:0" << endl;
            newFile << "�ֶ�ѡ��ԭ(Ĭ��ѡ����):0" << endl;
            newFile << "������ʾ:1" << endl;
            newFile << "ѹ���ȼ�:5" << endl;
            newFile << "�����ı�������(0��ʾ������):0" << endl; 
    	}
    	printf("\n�����´浵�ļ���:\n\n"); 
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
		        std::cout << "�浵����: " << folderName << endl;
		        std::cout << "��ϸ��Ϣ����" << FindName(folderName) << endl;
		        std::cout << "�������ʱ��: " << modificationDate << endl;
		        std::cout << "-----------" << endl;
		    }
		    Sleep(2000);
		    sprint("���Ƿ�ϣ�������д浵���ñ�����(0/1)\n\n",30);
		    cin>>ifalias; 
		    if(ifalias) sprint("������������Ҫ����Щ�ļ�����һ���������Լ����ı�����\n\n",30);
			else sprint("��ô���Զ��Դ浵�ļ��������߶�ȡ��ϢΪ�����������Ҫ�޸ı��������ڡ����á����ֶ��޸ġ�\n",30);
			for (const auto& folderName : subdirectories)
		    {
		        string alias;
		        B:
		        if(ifalias)
				{
					cout << "���������´浵�ı���(������һ������) " << folderName << endl;
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
					printf("�ļ������Ʋ��ܰ������� \\  /  :  *  ?  \"  <  >  | ������������");
					goto B;
				}
				newFile << folderName << endl << alias << endl;
		    }
		    newFile << "$" << endl;
		}
	    newFile << "*" << endl;
	    newFile.close();
	    printf("���������Թرճ���\n");
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
				scanf("%d %d %d %d",&month,&day,&hour,&min);//�������������󣬺������ȷ���� 
				//2023.12.31����������ļ������һ���������� 
			}
			else if(mode==2)
			{
				Qread();
				cin>>bftime;
			}
			Qread();
			cin>>ifinter;
			if(usecf==0) // ʹ��һ�������еĴ浵·�� 
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
			    int i=0,ttt=0;//�浵���� �浵���ڴ浵�ļ������ 
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
				//���浵�ļ����Ƿ��и��� 
				int ix=0;
				bool ifnew=0; 
				for(int i=0;i<=summ;++i)
				{
					std::vector<std::string> subdirectories;
					listSubdirectories(Gpath2[i], subdirectories);
				    for (const auto& folderName : subdirectories)
				    {
				    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //��ʵ��һ���򲻸��� ע�⣬����ļ������ȱ�ݵ� 
				    		continue;
						--ix;//��Ϊ���һ�������Աȶ�ʱ-1����δ�ܴ��������� 
				    	//��һ�£����г�
				    	ifnew=true;
						printf("\n��⵽�µĴ浵���£�\n");
						string NGpath=Gpath2[i]+"\\"+folderName;
				        string modificationDate = getModificationDate(NGpath);
				        cout << "�浵����: " << folderName << endl;
				        cout << "�������ʱ��: " << modificationDate << endl;
				        cout << "-----------" << endl;
				    }
				}
				if(ifnew) //����и��£�ѯ���Ƿ���������ļ�
				{
					printf("�����Ƿ���������ļ���(0/1)\n");
					char ch;
					ch=getch();
					printf("\n���ֶ����£��ڶ�Ӧλ���Ժ�����´浵�ġ���ʵ�����͡�������\n"); 
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
				//���浵�ļ����Ƿ��и��� 
				int ix=0;
				bool ifnew=0; 
				for(int i=0;i<=summ;++i)
				{
					std::vector<std::string> subdirectories;
					listSubdirectories(Gpath2[i], subdirectories);
				    for (const auto& folderName : subdirectories)
				    {
				    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //��ʵ��һ���򲻸��� ע�⣬����ļ������ȱ�ݵ� 
				    		continue;
						--ix;//��Ϊ���һ�������Աȶ�ʱ-1����δ�ܴ��������� 
				    	//��һ�£����г�
				    	ifnew=true;
						printf("\n��⵽�µĴ浵���£�\n");
						string NGpath=Gpath2[i]+"\\"+folderName;
				        string modificationDate = getModificationDate(NGpath);
				        cout << "�浵����: " << folderName << endl;
				        cout << "�������ʱ��: " << modificationDate << endl;
				        cout << "-----------" << endl;
				    }
				}
				if(ifnew) //����и��£�ѯ���Ƿ���������ļ�
				{
					printf("�����Ƿ���������ļ���(0/1)\n");
					char ch;
					ch=getch();
					printf("\n���ֶ����£��ڶ�Ӧλ���Ժ�����´浵�ġ���ʵ�����͡�������\n"); 
					string command="start "+temps;
					if(ch=='1')
						system(command.c_str());
				}
			}
			if(mode==1)
			{
				while(true)
				{
					// ��ȡ��ǰʱ��
				    std::time_t now = std::time(nullptr);
				    std::tm* local_time = std::localtime(&now);
				
				    std::tm target_time = {0};
				    target_time.tm_year = local_time->tm_year; // ��ݴ�1900�꿪ʼ����
				    if(month==0)  target_time.tm_mon = local_time->tm_mon;
				    else target_time.tm_mon = month - 1; // �·ݴ�0��ʼ����
				    if(day==0) target_time.tm_mday = local_time->tm_mday;
				    else target_time.tm_mday = day;
				    if(hour==0) target_time.tm_hour = local_time->tm_hour;
				    else target_time.tm_hour = hour;
				    if(min==0) target_time.tm_min = local_time->tm_min;
				    else target_time.tm_min = min;
				    // �����ǰʱ���Ѿ�������Ŀ��ʱ�䣬��ô�Ͳ���Ҫ�ȴ���
				    A: 
				    if (std::mktime(local_time) > std::mktime(&target_time)) {
				        std::cout << "���ڵ�ʱ���Ѿ�������Ŀ��ʱ��" << std::endl;
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
				        // ������Ҫ�ȴ���ʱ�䣨��λ���룩
				        std::time_t wait_time = std::difftime(std::mktime(&target_time), std::mktime(local_time));
				        // �ȴ�ָ����ʱ��
				        std::this_thread::sleep_for(std::chrono::seconds(wait_time));
				        Backup(bfnum,false);
				    }
				}
			} 
			else if(mode==2)
			{
				while(true)
				{
				    // ���߳�����ָ����ʱ��
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
    int i=0,ttt=0;//�浵���� �浵���ڴ浵�ļ������ 
    printf("�����´浵:\n\n"); 
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
	//���浵�ļ����Ƿ��и��� 
	int ix=0;
	bool ifnew=0; 
	for(int i=0;i<=summ;++i)
	{
		std::vector<std::string> subdirectories;
		listSubdirectories(Gpath2[i], subdirectories);
	    for (const auto& folderName : subdirectories)
	    {
	    	if((Gpath2[i]+"\\"+folderName)==name[++ix].real) //��ʵ��һ���򲻸��� ע�⣬����ļ������ȱ�ݵ� 
	    		continue;
			--ix;//��Ϊ���һ�������Աȶ�ʱ-1����δ�ܴ��������� 
	    	//��һ�£����г�
	    	ifnew=true;
			printf("\n��⵽�µĴ浵���£�\n");
			string NGpath=Gpath2[i]+"\\"+folderName;
	        string modificationDate = getModificationDate(NGpath);
	        cout << "�浵����: " << folderName << endl;
	        cout << "�������ʱ��: " << modificationDate << endl;
	        cout << "-----------" << endl;
	    }
	}
	if(ifnew) //����и��£�ѯ���Ƿ���������ļ�
	{
		printf("�����Ƿ���������ļ���(0/1)\n");
		char ch;
		ch=getch();
		printf("\n���ֶ����£��ڶ�Ӧλ���Ժ�����´浵�ġ���ʵ�����͡�������\n"); 
		if(ch=='1')
			system("start config.ini");
	}
	while(true)
	{
		printf("������Ҫ (1)���ݴ浵 (2)�ص� (3)���´浵 (4) �Զ����� ���� (5) ���������ļ� �أ� (�� 1/2/3/4/5)\n");
		char ch;
		ch=getch();
		if(ch=='1')
		{
			printf("����浵ǰ���������ɱ���:");
			int bf;
			scanf("%d",&bf);
			Backup(bf,echos);
			sprint("\n\n�������! ! !\n\n",40);
		}
		else if(ch=='2')
		{
			int i=0;
		    if(!choice) printf("����浵ǰ���������ɻ�ԭ (ģʽ: ѡȡ���±���) : ");
		    else printf("����浵ǰ���������ɻ�ԭ (ģʽ: �ֶ�ѡ��) :");
			int bf;
			scanf("%d",&bf);
			string folderPath=Bpath+"\\"+name[bf].alias+"\\";
			DIR* directory = opendir(folderPath.c_str());
		    if (!directory) {
		        printf("���ݲ����ڣ��޷���ԭ������Ի�û�н��й�����\n");
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
				printf("�����Ǳ��ݴ浵\n\n");
				ListFiles(folderName2);
				printf("���뱸��ǰ���������ɻ�ԭ:");
				int bf2;
				scanf("%d",&bf2);
				files.name=temp[bf2];
			}
		    if(prebf)
		    	Backup(bf,false);
			command=yasuo+" x "+Bpath+"\\"+name[bf].alias+"\\"+files.name+" -o"+name[bf].real+" -y";
			system(command.c_str());
			sprint("\n\n��ԭ�ɹ�! ! !\n\n",40);
		}
		else if(ch=='3')
		{
			freopen("CON","r",stdin);
        	ofstream cfile("config.ini");
        	cfile << "ʹ�õ������ļ����:0\n";
        	cfile << "�浵�ļ���·��:" << Gpath2[0] << '$';
        	for(int i=1;i<summ;++i)
        		cfile << Gpath2[i] << '$';
        	cfile << Gpath2[summ] << endl;
        	cfile << "�浵���ݴ洢·��:" << Bpath << endl;
			string keyPath = "Software\\7-Zip"; 
			string valueName = "Path";
            cfile << "ѹ�����·��:" << GetRegistryValue(keyPath, valueName)+"7z.exe" << endl;
            cfile << "����ǰ��ԭ:" << prebf << endl;
            cfile << "�������ö�:" << ontop << endl;
            cfile << "�ֶ�ѡ�񱸷�:" << choice << endl;
            cfile << "������ʾ:" << echos << endl;
            cfile << "ѹ���ȼ�:" << lv << endl;
            cfile << "�����ı�������(0��ʾ������):" << limitnum << endl; 
        	printf("\n���ļ����������´浵: \n\n"); 
	    	for(int i=0;i<=summ;++i)
	    	{
	    		vector<string> subdirectories;
				listSubdirectories(Gpath2[i], subdirectories);
			    for (const auto& folderName : subdirectories)
			    {
					string NGpath=Gpath2[i]+"\\"+folderName;
			        string modificationDate = getModificationDate(NGpath);
			        cout << "�浵����: " << folderName << endl;
			        cout << "�������ʱ��: " << modificationDate << endl;
			        cout << "-----------" << endl;
			    }
			    Sleep(2000);
			    sprint("������������Ҫ����Щ�ļ�����һ���������Լ����ı�����\n\n",50);
				for (const auto& folderName : subdirectories)
			    {
			        string alias;
			        cout << "���������´浵�ı���(������һ������) " << endl << folderName;
			        cin >> alias;
					cfile << folderName << endl << alias << endl;
			    }
			    cfile << "$" << endl;
			}
		    cfile << "*" << endl;
	    	puts("\n\n���´浵���\n\n");
	    	cfile.close(); 
		}
		else if(ch=='4')
		{
			printf("��������Ҫ���ݵĴ浵���:");
			int bf,tim;
			scanf("%d",&bf);
			printf("ÿ�������ӽ��б���: ");
			scanf("%d",&tim);
			printf("�ѽ����Զ�����ģʽ��ÿ�� %d ���ӽ��б���",tim);
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
		else printf("�밴�����ϵ� 1/2/3/4/5 ��\n\n");
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
    CreateWindow("button", "�򿪴浵�ļ���",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 10, 120, 35,
        hwnd, (HMENU)1, hInstance, NULL);

    CreateWindow("button", "�򿪱����ļ���",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        150, 10, 120, 35,
        hwnd, (HMENU)2, hInstance, NULL);

    CreateWindow("button", "����",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        280, 10, 80, 35,
        hwnd, (HMENU)3, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);    
    std::thread MainThread(Main);
    MSG msg = {};
    
    //�߳����ߣ�Ϊ�˵ȴ�ontop��ȡ��� 
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
