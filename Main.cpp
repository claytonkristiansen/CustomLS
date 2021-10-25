#include <string>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <filesystem>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <ctime>
#include <sys/sysmacros.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <algorithm>    // std::sort
#include <iomanip>

namespace fs = std::filesystem;

using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::stringstream;

#define PATH_BUFFER_SIZE 200

string GetRoot(string path)
{
    stringstream ss("");
    for(char c : path)
    {
        if(ss.str() != "" && (c == '/' || c == '\\'))
        {
            return ss.str();
        }
        else if(c != '/' && c != '\\')
        {
            ss << c;
        }
    }
    return ss.str();
}

string GetName(string path)
{
    stringstream ss("");
    int i = path.size() - 1;
    for(; i > 0; --i)
    {
        if(path[i - 1] == '/' || path[i - 1] == '\\')
        {
            break;
        }
    }
    for(; i < path.size(); ++i)
    {
        ss << path[i];
    }
    return ss.str();
}

bool compareFunction(string sL, string sR)
{
    return sL < sR;
}

vector<string> GetFilesAlphabetically(string dir)
{
    vector<string> fileNames;
    DIR *dr;
    struct dirent *en;
    dr = opendir(dir.c_str()); //open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) 
        {
            if(en->d_name[0] != '.'
            && en->d_name != "..")
            {
                fileNames.push_back(en->d_name);
            }
        }
        closedir(dr); //close all directory
    }
    std::sort(fileNames.begin(), fileNames.end(), compareFunction);
    return fileNames;
}

string PropertiesOfFile(struct stat &sb, const string &path)
{
    bool isSoftLink = false;
    stringstream ss;
    //Field 1
    if(fs::is_symlink(path))        //Is link
    {
        ss << "l";
        isSoftLink = true;
    }
    else if((sb.st_mode & S_IFMT) == S_IFREG)             //Is regular file
    {
        ss << "-";
    }
    else if((sb.st_mode & S_IFMT) == S_IFDIR)        //Is directory
    {
        ss << "d";
    }
    
    //Field 2
    if((S_IRUSR & sb.st_mode) == S_IRUSR) ss << "r";
    else ss << "-";
    if((S_IWUSR & sb.st_mode) == S_IWUSR) ss << "w";
    else ss << "-";
    if((S_IXUSR & sb.st_mode) == S_IXUSR) ss << "x";
    else ss << "-";
    //Field 3
    if((S_IRGRP & sb.st_mode) == S_IRGRP) ss << "r";
    else ss << "-";
    if((S_IWGRP & sb.st_mode) == S_IWGRP) ss << "w";
    else ss << "-";
    if((S_IXGRP & sb.st_mode) == S_IXGRP) ss << "x";
    else ss << "-";
    //Field 4
    if((S_IROTH & sb.st_mode) == S_IROTH) ss << "r";
    else ss << "-";
    if((S_IWOTH & sb.st_mode) == S_IWOTH) ss << "w";
    else ss << "-";
    if((S_IXOTH & sb.st_mode) == S_IXOTH) ss << "x";
    else ss << "-";
    //Field 5
    ss << " " << sb.st_nlink << " ";
    //Field 6
    ss << getpwuid(sb.st_uid)->pw_name << " ";
    //Field 7
    ss << getpwuid(sb.st_gid)->pw_name << " ";
    //Field 8
    ss << std::setw(8) << sb.st_size;
    //Field 9
    char buf[100];
    string timeString = asctime(localtime(&sb.st_atim.tv_sec));
    timeString.pop_back();
    ss << timeString << " ";
    //Field 10
    ss << GetName(path);
    //Optional soft link Field
    if(isSoftLink)
    {
        ss << " -> " << fs::read_symlink(path);
    }   

    return ss.str();
}

void CommandLS(string path)
{
    char workingDirBuf[PATH_BUFFER_SIZE];
    getcwd(workingDirBuf, PATH_BUFFER_SIZE);
    string workingDir(workingDirBuf);
    string systemRoot = GetRoot(workingDir);
    string pathRoot = GetRoot(path);
    string filePath = "";
    
    if(pathRoot == systemRoot)      //Absolute path
    {
        filePath = path;
    }
    else if(path != "")             //Relative path
    {
        filePath = workingDir;
        filePath.append("/" + path);
    }
    else if(path == "." || path == "")          //Current directory
    {
        filePath = workingDir;
    }


    struct stat sb;
    if(stat(filePath.c_str(), &sb) == -1)   //Doesn't exist
    {
        cout << "ls: cannot access" << filePath << ": No such file or directory\n";
    }
    else if(S_ISDIR(sb.st_mode))     //Is a directory
    {        
        // DIR *dr;
        // struct dirent *en;
        // dr = opendir(filePath.c_str()); //open all directory
        // if (dr) {
        //     while ((en = readdir(dr)) != NULL) 
        //     {
        //         if(en->d_name[0] != '.'
        //         && en->d_name != "..")
        //         {
                    
        //         }
        //     }
        //     closedir(dr); //close all directory
        // }
        for(string s : GetFilesAlphabetically(filePath))
        {
            //Look at properties and print accordingly
            string subFilePath = filePath + "/" + s;
            struct stat sb;
            stat(subFilePath.c_str(), &sb);
            cout << PropertiesOfFile(sb, subFilePath) << "\n";
        }
    }
    else                        //Is not a directory
    {   
        cout << PropertiesOfFile(sb, filePath) << "\n";
    }

    
    


    
}



int main(int argc, char* argv[])
{
    int opt;
    string path = "";
    if(argc > 2 && argv[2] != NULL)
    {
        path == argv[2];
    }
    CommandLS(path);


    return 0;
}