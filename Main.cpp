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
        if(path[i - 1] == '/' || path[i - 1] == '\\') {break;}
    }
    for(; i < path.size(); ++i)
    {
        ss << path[i];
    }
    return ss.str();
}

string GoUpDirectory(string path)
{
    int i = path.size() - 1;
    for(; i > 0; --i)
    {
        if(path[i] == '/' || path[i] == '\\') {break;}
    }
    return path.substr(0, i);
}

bool compareFunction(string sL, string sR)
{
    std::transform(sL.begin(), sL.end(), sL.begin(), ::tolower);
    std::transform(sR.begin(), sR.end(), sR.begin(), ::tolower);
    return sL < sR;
}

//Gets the largest width for the different field for formatting purposes
void MaxWidths(string dir, int &numReferencesWidth, int &userNameWidth, int &groupNameWidth, int &dataSizeWidth)
{
    numReferencesWidth = 0;
    userNameWidth = 0;
    groupNameWidth = 0;
    dataSizeWidth = 0;
    DIR *dr;
    struct dirent *en;
    dr = opendir(dir.c_str()); //open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) 
        {
            if(en->d_name[0] != '.'
            && en->d_name != "..")
            {
                string subFilePath = dir + "/" + en->d_name;
                struct stat sb;
                lstat(subFilePath.c_str(), &sb);
                int numReferencesSize = string(std::to_string(sb.st_nlink)).size();
                int userSize = string(getpwuid(sb.st_uid)->pw_name).size();
                int groupSize = string(getpwuid(sb.st_gid)->pw_name).size();
                int dataSize = string(std::to_string((int)sb.st_size)).size();
                if(numReferencesSize > numReferencesWidth)
                {
                    numReferencesWidth = numReferencesSize;
                }
                if(userSize > userNameWidth)
                {
                    userNameWidth = userSize;
                }
                if(groupSize > groupNameWidth)
                {
                    groupNameWidth = groupSize;
                }
                if(dataSize > dataSizeWidth)
                {
                    dataSizeWidth = dataSize;
                }
            }
        }
        closedir(dr); //close all directory
    }
}

int NumBlocks(string dir)
{
    int blocks = 0;
    DIR *dr;
    struct dirent *en;
    dr = opendir(dir.c_str()); //open all directory
    if (dr) {
        while ((en = readdir(dr)) != NULL) 
        {
            if(en->d_name[0] != '.'
            && en->d_name != "..")
            {
                string subFilePath = dir + "/" + en->d_name;
                struct stat sb;
                lstat(subFilePath.c_str(), &sb);
                blocks += sb.st_blocks;
            }
        }
        closedir(dr); //close all directory
    }
    return blocks;
}

vector<string> GetFilesAlphabetically(string dir)   //Gets files in order that I want for different systems
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

string PropertiesOfFile(struct stat &sb, const string &path, int refWidth = 0, int userWidth = 0, int groupWidth = 0, int dataWidth = 0)
{
    bool isSoftLink = false;
    stringstream ss;
    //Field 1, file type
    if((sb.st_mode & S_IFMT) == S_IFLNK) {ss << "l"; isSoftLink = true;}        //Is symbolic link
    else if((sb.st_mode & S_IFMT) == S_IFREG) {ss << "-";}                      //Is regular file
    else if((sb.st_mode & S_IFMT) == S_IFDIR) {ss << "d";}                      //Is directory
    else if((sb.st_mode & S_IFMT) == S_IFIFO) {ss << "p";}                      //Is FIFO
    //Field 2, user permissions
    if((S_IRUSR & sb.st_mode) == S_IRUSR) ss << "r";
    else ss << "-";
    if((S_IWUSR & sb.st_mode) == S_IWUSR) ss << "w";
    else ss << "-";
    if((S_IXUSR & sb.st_mode) == S_IXUSR) ss << "x";
    else ss << "-";
    //Field 3, group permissions
    if((S_IRGRP & sb.st_mode) == S_IRGRP) ss << "r";
    else ss << "-";
    if((S_IWGRP & sb.st_mode) == S_IWGRP) ss << "w";
    else ss << "-";
    if((S_IXGRP & sb.st_mode) == S_IXGRP) ss << "x";
    else ss << "-";
    //Field 4, others permissions
    if((S_IROTH & sb.st_mode) == S_IROTH) ss << "r";
    else ss << "-";
    if((S_IWOTH & sb.st_mode) == S_IWOTH) ss << "w";
    else ss << "-";
    if((S_IXOTH & sb.st_mode) == S_IXOTH) ss << "x";
    else ss << "-";
    //Field 5, number of links to this file
    ss << " " << std::setw(refWidth) << sb.st_nlink << " ";
    //Field 6, user name
    ss << std::setw(userWidth) << getpwuid(sb.st_uid)->pw_name << " ";
    //Field 7, group name
    ss << std::setw(groupWidth) << getpwuid(sb.st_gid)->pw_name << " ";
    //Field 8, file size in Bytes
    ss << std::setw(dataWidth) << sb.st_size << " ";
    //Field 9, time of last modification
    ss << string(asctime(localtime(&sb.st_mtim.tv_sec))).substr(4, 12) << " ";
    //Field 10, file name
    ss << GetName(path);
    //Optional soft link Field
    if(isSoftLink)
    {
        ss << " -> " << string(fs::read_symlink(path));
    }   

    return ss.str();
}

void CommandLS(string path)                     //Processes the command
{
    char workingDirBuf[PATH_BUFFER_SIZE];
    getcwd(workingDirBuf, PATH_BUFFER_SIZE);
    string workingDir(workingDirBuf);
    string systemRoot = GetRoot(workingDir);
    string pathRoot = GetRoot(path);
    string filePath = "";
    
    if(pathRoot == systemRoot)              //Absolute path
    {
        filePath = path;
    }
    else if(path == "..")                   //Directory up
    {

    }
    else if(path == "." || path == "")      //Current directory
    {
        filePath = workingDir;
    }
    else                                    //Relative path
    {
        filePath = workingDir;
        filePath.append("/" + path);
    }


    struct stat sb;
    if(lstat(filePath.c_str(), &sb) == -1)   //Doesn't exist
    {
        cout << "ls: cannot access" << filePath << ": No such file or directory\n";
    }
    else if(S_ISDIR(sb.st_mode))     //Is a directory
    {      
        cout << "total " << NumBlocks(filePath) / 2 << "\n";
        int refWidth;
        int userWidth;
        int groupWidth;
        int dataWidth;
        MaxWidths(filePath, refWidth, userWidth, groupWidth, dataWidth);
        for(string name : GetFilesAlphabetically(filePath))
        {
            //Look at properties and print accordingly
            string subFilePath = filePath + "/" + name;
            struct stat sb;
            lstat(subFilePath.c_str(), &sb);
            cout << PropertiesOfFile(sb, subFilePath, refWidth, userWidth, groupWidth, dataWidth) << "\n";
        }
    }
    else                        //Is not a directory
    {   
        cout << PropertiesOfFile(sb, filePath) << "\n";
    }

    
    


    
}



int main(int argc, char* argv[])
{
    string testPath("/home/osboxes/repos/CustomLS/dopelink");
    struct stat sb;
    lstat(testPath.c_str(), &sb);
    int opt;
    string path = "";
    if(argc > 2 && argv[2] != NULL)
    {
        path = argv[2];
    }
    CommandLS(path);


    return 0;
}