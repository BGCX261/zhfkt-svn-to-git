#include "stdafx.h"
#include "utility.h"

#ifdef WIN32
string path_split = "\\";
#else
string path_split = "/";
#endif

vector<string>& utility::split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> utility::split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}

string utility::get_single_name(string full_name)
{
    int point_num = full_name.length();
    int line_num = 0;

    for(int i=full_name.length()-1;i>=0;i--)
    {
        if(full_name[i]=='.')
        {
            point_num = i;
        }

        if(full_name[i]== path_split[0])
        {
            line_num = i;
            break;
        }
    }

    return full_name.substr(line_num+1,point_num-line_num-1);
}

string utility::int2string(int num)
{
    stringstream ss;
    string s;
    ss << num;
    ss >> s;

    return s;
}

int utility::string2int(string s)
{
    stringstream ss;
    int num;
    ss << s;
    ss >> num;

    return num;
}

int utility::is_folder_empty(string di)
{
	#ifdef WIN32
	
    const char *directory = di.c_str();

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    char pattern[MAX_PATH];

    // 开始查找
    strcpy(pattern, directory);
    strcat(pattern, "\\*.*");
    hFind = FindFirstFile(pattern, &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        return -1;
    } 
    else 
    {
        int j=0;
        do
        {
            j++;
            
            if(j==3)
            {
                FindClose(hFind);
                return 0;                
            }
        }
        while (FindNextFile(hFind, &FindFileData) != 0);
    }
    
    FindClose(hFind);
    // 查找结束
    
    return 1;
	
	#else


    DIR *dp;

	struct dirent *dirp;
	if((dp=opendir(di.c_str()))==NULL)
	{
		printf("can't open %s",di.c_str());
	}
	while (((dirp=readdir(dp))!=NULL))
	{
		string file_list_s(dirp->d_name);
		if(file_list_s==".." || file_list_s==".")
		{
			continue;
		}
		closedir(dp);
		return 0;
	}
	closedir(dp);

	return 1;	


	#endif


   
}

vector<string> utility::EnumFiles(string di)
{

	#ifdef WIN32
	
    const char *directory = di.c_str();

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    vector<string> result;
    char pattern[MAX_PATH];

    // 开始查找
    strcpy(pattern, directory);
    strcat(pattern, "\\*.*");
    hFind = FindFirstFile(pattern, &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        vector<string> null_v;
        return null_v;
    } 
    else 
    {
        do
        {
            result.push_back(FindFileData.cFileName);
        }
        while (FindNextFile(hFind, &FindFileData) != 0);
    }

    // 查找结束
    FindClose(hFind);




    vector<string> file_list;

    for(int i=0;i<result.size()-2;i++)
    {
        string file_list_s(result[i+2]);
        file_list.push_back(file_list_s);
    }

    return file_list;
	
	#else
	
	vector<string> file_list;


	DIR *dp;
	struct dirent *dirp;
	if((dp=opendir(di.c_str()))==NULL)
	{
		printf("can't open %s",di.c_str());
	}
	while (((dirp=readdir(dp))!=NULL))
	{
		string file_list_s(dirp->d_name);
		if(file_list_s==".." || file_list_s==".")
		{
			continue;
		}

		file_list.push_back(file_list_s);
	}
	closedir(dp);

	return file_list;

	#endif




}
