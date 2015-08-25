#ifndef __UTILITY__
#define __UTILITY__


#include <sstream>
#include <vector>


#ifdef WIN32
	
#include <Windows.h>
	
#else
	
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
	
#endif

using namespace std;

class utility
{
public:
    static int string2int(string s);
    static string int2string(int);
    static string get_single_name(string);
    static vector<string> split(const string &s, char delim);
    static vector<string> EnumFiles(string di);
    static int is_folder_empty(string di);

private:
    static vector<string> &split(const string &s, char delim, vector<string> &elems);
};

extern string path_split;

#endif /*__UTILITY__*/