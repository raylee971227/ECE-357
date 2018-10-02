//Raymond Lee
//Find Command Source Code
//ECE-357: Computer Operating Systems
//Created 9/29/18

//usage: find /starting_directory -ls

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// char *getUserUID(uid_t uid);

// char *getUserGID(uid_t uid);

// char *humanTime(time_t time);

// char printFileSize(long long int size);

// int searchDir(const char *dirName);

//get user's name from UID
char *getUserUID(uid_t uid) {
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}
//get group name from GID
char *getUserGID(gid_t gid) {
    struct group *grp;
    grp = getgrgid(gid);
    return grp->gr_name;
}

//parse mtime
char *humanTime(time_t time) {
    char *ct = ctime(&time);
    char *start = &ct[4];
    char *end = &ct[16];
    char *substr = (char *)calloc(1, end - start + 1);
    memcpy(substr, start, end - start);
    return substr;
}


void printLine(struct stat statBuf,char * path) {
    //Inode block number
    printf("%llu ",statBuf.st_ino);
    //disk usage of file
    printf("%lli ",statBuf.st_blocks);
    //inode type and mode string (permissions)
    printf( (S_ISDIR(statBuf.st_mode)) ? "d" : "-");
    printf( (statBuf.st_mode & S_IRUSR) ? "r" : "-");
    printf( (statBuf.st_mode & S_IWUSR) ? "w" : "-");
    printf( (statBuf.st_mode & S_IXUSR) ? "x" : "-");
    printf( (statBuf.st_mode & S_IRGRP) ? "r" : "-");
    printf( (statBuf.st_mode & S_IWGRP) ? "w" : "-");
    printf( (statBuf.st_mode & S_IXGRP) ? "x" : "-");
    printf( (statBuf.st_mode & S_IROTH) ? "r" : "-");
    printf( (statBuf.st_mode & S_IWOTH) ? "w" : "-");
    printf( (statBuf.st_mode & S_IXOTH) ? "x" : "-");
    printf(" "); //space
    //link count
    printf("%hu ", statBuf.st_nlink);
    //uid name
    printf("%s  ", getUserUID(statBuf.st_uid));
    //gid name
    printf("%s  ", getUserGID(statBuf.st_gid));
    //file size
    printf("%lld ", statBuf.st_size);
    //mtime
    printf("%s ", humanTime(statBuf.st_mtime));
    //path
    printf("%s ", path);
}

int searchDir(const char *dirName) {
    DIR *dir;
    struct dirent *dp;
    struct stat statBuf;
    int flag = 1;
    //open directory
    if((dir = opendir(dirName)) == NULL) {
        fprintf(stderr, "Error: Cannot open directory %s: %s\n", dirName, strerror(errno));
        return -1;
    }
    //read directory
    while((dp = readdir(dir)) != NULL) {
        char path[1024];
        //path -> string
        sprintf(path, "%s/%s", dirName, dp->d_name);

        if(lstat(path, &statBuf) == -1) {
            fprintf(stderr, "Error: Cannot stat file %s: %s\n", dirName, strerror(errno));
            return -1;
        }
        if(flag == 1) { //print current directory
            printLine(statBuf, path);
            printf("\n");
            flag = 0;
        }

        if(dp->d_type == DT_DIR) {
            if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }
            printLine(statBuf, path);
            printf("\n");
            searchDir(path);
        }
        else if(dp->d_type == DT_REG) {
            printLine(statBuf, path);
            printf("\n");
        }
        else if(dp->d_type == DT_LNK) {
            char symlBuf[1024];
            if(readlink(path, symlBuf, sizeof(symlBuf) - 1) == -1) {
                fprintf(stderr, "Error: Could not read contents of symlink %s\n%s\n", path, strerror(errno));
                return -1;
            }
            printLine(statBuf, path);
            printf("-> %s\n", symlBuf);
        }
        else {
            printf("\n");
        }
    }
    if(closedir(dir) < 0) {
        fprintf(stderr, "Error: Could not successfully close directory %s.\n%s\n", dirName, strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    char *startDir;
    switch(argc) {
        case 2:
            startDir = argv[1];
            break;
        case 1:
            startDir = ".";
            break;
        default:
            printf("usage: ./find [starting_directory] -ls\n");
            return -1;
    }
    searchDir(startDir);
    return 0;
}


