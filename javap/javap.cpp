// SensorTest.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#include <Windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#define _tmain      main
#define _ftprintf   fprintf
#endif

#include <locale.h>
#include <stdio.h>
#include "classdump.h"

#define MAX_FILE_SIZE 100L*1024L*1024L

#ifdef WIN32

void* GetFileData(const TCHAR* filename, size_t *len)
{
    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);

    if (INVALID_FILE_SIZE == dwFileSize || dwFileSize > MAX_FILE_SIZE)
    {
        CloseHandle(hFile);
        return NULL;
    }

    void *p = malloc(dwFileSize);

    if (!ReadFile(hFile, p, dwFileSize, &dwFileSize, NULL))
    {
        free(p);
        CloseHandle(hFile);
        return NULL;
    }

    CloseHandle(hFile);
    *len = dwFileSize;
    return p;
}
#else
void* GetFileData(const TCHAR* filename, size_t *len)
{
    FILE *fp = _tfopen(filename, _T("rb"));
    if (!fp || fseek(fp, 0, SEEK_END))
    {
        fclose(fp);
        return NULL;
    }
    int tlen = ftell(fp);
    if(tlen <= 0 || tlen > MAX_FILE_SIZE)
    {
        fclose(fp);
        return NULL;
    }
    void *p = malloc(tlen);
    if (0 != fseek(fp, 0, SEEK_SET))
    {
        free(p);
        fclose(fp);
        return NULL;
    }
    if (fread(p, 1, tlen, fp) < tlen)
    {
        free(p);
        fclose(fp);
        return NULL;
    }
    *len = tlen;
    fclose(fp);
    return p;
}
#endif

int DumpClass(unsigned char* buf, size_t len, FILE *fp)
{
    size_t offset = 0;


    return 0;
}

int ScanFile(const TCHAR *fileName)
{
    size_t len = 0;
    unsigned char *p = (unsigned char*)GetFileData(fileName, &len);

    if (0 == len)
    {
        _ftprintf(stdout, _T("GetFileData is failed: %s"), fileName);
        free(p);
        return 0;
    }
    if (CF_IsJavaClassFile(p, len) ==  CF_OK)
    {
        ClassFile cf;
        size_t offset = 0;
        if (CF_ReadClassFile(p, len, &offset, &cf) == CF_OK)
        {
            CF_DumpClassFile(&cf, stdout);
        }
        else
        {
            fprintf(stderr, "not scan: %s\n", fileName);
        }
        CF_FreeClassFile(&cf);
    }
    else
    {
        fprintf(stderr, "The stream is not a valid class or contains errors\n");
    }
    free(p);
    return 0;
}

#ifdef WIN32


int IsDir(const TCHAR *path)
{
    return (GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY);
}

void ScanDir(const TCHAR *dir, bool recursive)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    TCHAR fullPath[MAX_PATH];
    TCHAR pathMask[MAX_PATH];

    _sntprintf(pathMask, MAX_PATH-1, _T("%s\\*"), dir);

    hFind = FindFirstFile(pathMask, &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            _sntprintf(fullPath, _countof(fullPath)-1, _T("%s\\%s"), dir, FindFileData.cFileName);

            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                ScanFile(fullPath);
                //++g_total;
            }
            else if (FindFileData.cFileName[0] != '.' )
            {
                ScanDir(fullPath, recursive);
            }

        } while (FindNextFile(hFind, &FindFileData));

        FindClose(hFind);
    }
}
#else


int IsDir(const TCHAR *path)
{
    struct stat st;

    if (stat(path,&st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }

    return 0;
}
void ScanDir(const TCHAR *dir, bool recursive)
{
    DIR *dp;
    struct dirent *de;
    struct stat st;
    char full_path[MAX_PATH];

    dp = opendir(dir);

    if (dp)
    {
        de = readdir(dp);
        while (de)
        {
            sprintf(full_path, "%s/%s", dir, de->d_name);
            int err = stat(full_path,&st);
            if (err == 0)
            {
                if(S_ISREG(st.st_mode))
                {
                    ScanFile(full_path);
                }
                else if(recursive && S_ISDIR(st.st_mode) && de->d_name[0] != '.')
                {
                    ScanDir(full_path, recursive);
                }
            }
            de = readdir(dp);
        }
        closedir(dp);
    }
}
#endif


void helpusage(TCHAR* fn)
{
    _tprintf(_T("usage: %s <classname>\n"), fn);
}


int _tmain(int argc, TCHAR* argv[])
{
    if (argc < 2)
    {
        helpusage(argv[0]);
        return 0;
    }

    setlocale(LC_ALL, "");
    TCHAR path[MAX_PATH] = {0};
    _tcsncpy(path, argv[1], MAX_PATH-1);

    if (IsDir(path))
    {
        printf("not support directory.^_^\n");
        return 0;
    }
    else
    {
        ScanFile(path);
    }
    return 0;
}

