#ifndef _DEBUG_MANAGER_H
#define _DEBUG_MANAGER_H 


// 支持两种调试通道：网络和stdout
typedef struct DebugOpr{
    char *name;
    int canUsed;
    int (*DebugInit)(void);
    int (*DebugExit)(void);
    int (*DebugPrint)(char *strData);
    struct DebugOpr *ptNext;
}T_DebugOpr, *PT_DebugOpr;

int RegisterDebugOpr(PT_DebugOpr ptDebugOpr);
int ShowDebugOpr(void);
PT_DebugOpr GetDebugOpr(char *pcName);
int SetDebugLevel(char *strBuf);
int SetDebugChannel(char *strBuf);
int DebugInit(void);
int DebugPrint(const char *pcFormat, ...);
int InitDebugChannel(void);

int StdoutInit(void);
int NetPrintInit(void);
#endif