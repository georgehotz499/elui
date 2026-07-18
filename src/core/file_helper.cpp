#include "file_helper.h"
#include "resources.h"
#include "screen.h"
#include "string_helper.h"
#include "log.h"

#include <fstream>
#include <sys/stat.h>

#ifdef _WIN64
#include <windows.h>
#elif __linux__
#include <dirent.h>
#include <unistd.h>
#endif


std::string FileHelper::ReadFile(const std::string& path) {
    std::string content;

    std::ifstream is(path.c_str(), std::ifstream::binary);
    if (is) {
        // get length of file:
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);

        if (0 >= length) {
            LOGD("length:%d", length);
            return "";
        }

        char* buffer = new char[length];
        // read data as a block:
        is.read(buffer, length);
        // 保存数据
        content = std::string(buffer, length);
        // 释放申请空间
        delete[] buffer;
    }
    else {
        LOGI("Failed to open file:%s", path.c_str());
    }

    return content;
}

void FileHelper::ReadFile(const std::string& path, std::string& out_file) {
    std::ifstream is(path.c_str(), std::ifstream::binary);
    if (is) {
        // get length of file:
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);

        if (0 >= length) {
            LOGD("length:%d", length);
            return;
        }

        char* buffer = new char[length];
        // read data as a block:
        is.read(buffer, length);
        // 保存数据
        out_file = std::string(buffer, length);
        // 释放申请空间
        delete[] buffer;
    }
    else {
        LOGI("Failed to open file:%s", path.c_str());
    }
}

void FileHelper::WriteFile(const std::string& content, const std::string& path) {
    std::ofstream outfile(path.c_str(), std::ofstream::binary);
    outfile.write(content.c_str(), content.size());
    outfile.flush();
    outfile.close();
    system("sync");
}

void FileHelper::CreatDir(std::string dir) {
    std::string cmd;
#ifdef _WIN64
    dir = StringHelper::Replace(dir, "/", "\\");
    cmd.append("cd data && mkdir ").append(dir);
    LOGI("cmd:%s", cmd.c_str());
    system(cmd.c_str());
#elif __linux__
    cmd.append("mkdir -p /data/").append(dir);
    LOGI("cmd:%s", cmd.c_str());
    system(cmd.c_str());
#endif
}

void FileHelper::DeleteDir(std::string dir) {
    std::string cmd;
#ifdef _WIN64
    dir = StringHelper::Replace(dir, "/", "\\");
    cmd.append("cd data && rd /s /q ").append(dir);
#elif __linux__
    cmd.append("rm -rf /data/").append(dir);
#endif
    LOGI("cmd:%s", cmd.c_str());
    system(cmd.c_str());
    system("sync");
}

bool FileHelper::CheckFileExist(const std::string& file_path) {
    FILE* fp = fopen(file_path.c_str(), "r");
    if (fp) {
        fclose(fp);
        // 文件存在
        return true;
    }
    // 文件不存在
    return false;
}

bool FileHelper::CheckDirecroyExist(const std::string& path) {
#ifdef _WIN64
    // Windows 平台使用 _stat
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) {
        return false;  // 路径不存在
    }
    return (info.st_mode & _S_IFDIR) != 0;  // 判断是否为目录
#elif __linux__
    // Linux 平台使用 stat
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;  // 路径不存在
    }
    return (info.st_mode & S_IFDIR) != 0;  // 判断是否为目录
#endif
    return false;
}

std::unordered_set<std::string> FileHelper::GetFilesInDir(const std::string& dirPath) {
    std::unordered_set<std::string> files;

#ifdef _WIN64
    // Windows 实现：使用 FindFirstFile/FindNextFile
    std::string searchPath = dirPath + "/*.*";  // 匹配所有文件
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        LOGI("Failed to open dir:%s", searchPath.c_str());
        return files;  // 目录不存在或打开失败
    }

    do {
        std::string fileName = findData.cFileName;
        // 跳过 "." 和 ".."（当前目录和父目录）
        if (fileName == "." || fileName == "..") {
            continue;
        }
        // 判断是否为文件（可选：如需包含目录可去掉此判断）
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;  // 跳过子目录
        }
        files.insert(fileName);
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);  // 释放资源
#elif __linux__
    // Linux 实现：使用 dirent.h
    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        return files;  // 目录不存在或打开失败
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName = entry->d_name;
        // 跳过 "." 和 ".."
        if (fileName == "." || fileName == "..") {
            continue;
        }
        // 判断是否为文件（可选：如需包含目录可去掉此判断）
        std::string fullPath = dirPath + "/" + fileName;
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            files.insert(fileName);
        }
    }

    closedir(dir);  // 释放资源
#endif

    return files;
}

bool FileHelper::DelFile(const std::string& file_path) {
#ifdef _WIN64
    // Windows：使用 DeleteFileA（ANSI 版本）
    // 注意：路径中的反斜杠需转义（如 "C:\\test.txt"）或用斜杠 "C:/test.txt"
    return DeleteFileA(file_path.c_str()) != 0;
#elif __linux__
    // Linux：使用 unlink 函数（删除文件）
    // 若删除符号链接，用 unlink；删除目录用 rmdir（但此处只删文件）
    return unlink(file_path.c_str()) == 0;
#endif
    return false;
}
