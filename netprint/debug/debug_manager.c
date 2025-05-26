#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include<stdio.h>
#include <stdarg.h>


#define DEFAULT_DEBUGLEVEL 4

static PT_DebugOpr g_ptDebugOprHead = NULL;
// 开始将其设置为4
static int g_iDebugLevelLimit = DEFAULT_DEBUGLEVEL;
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr){
    PT_DebugOpr ptTmp;

    if(!g_ptDebugOprHead){
        g_ptDebugOprHead = ptDebugOpr;
        ptDebugOpr->ptNext = NULL;
    }else{
        ptTmp = g_ptDebugOprHead;
        while(ptTmp->ptNext){
            ptTmp = ptTmp->ptNext;
        }
        ptTmp->ptNext = ptDebugOpr;
        ptDebugOpr->ptNext = NULL;
    }
    return 0;
}

int ShowDebugOpr(void){

    PT_DebugOpr ptTmp = g_ptDebugOprHead;
    int i = 0;

    while(ptTmp){
        printf("%02d %s\n", i++, ptTmp->name);
        ptTmp = ptTmp->ptNext;
    }
    return 0;
}

PT_DebugOpr GetDebugOpr(char *pcName){

	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}


// dbglevel=0,1,2,3...7
int SetDebugLevel(char *strBuf){
    g_iDebugLevelLimit = strBuf[9] - '0';
    return 0;
}


// * stdout = 0/1           :关闭/打开标准输出打印
// * netprint = 0/1         :关闭/打开网络打印
int SetDebugChannel(char *strBuf){
    char *pStrTmp;
    char strName[100];
    PT_DebugOpr ptDebugOprTmp;
    /* 在strBuf中第一次出现"="的位置 */
    pStrTmp = strchr(strBuf, (int)'=');
    if(pStrTmp == NULL){
        return -1;
    }else{
        strncpy(strName, strBuf, pStrTmp - strBuf);
        strName[pStrTmp - strBuf] = '\0';
        ptDebugOprTmp = GetDebugOpr(strName);
        if(!ptDebugOprTmp){
            return -1;
        }
        if(pStrTmp[1] == '0')
            ptDebugOprTmp->canUsed = 0;
        else
            ptDebugOprTmp->canUsed = 1;
        
        return 0;
    }
}

int DebugPrint(const char *pcFormat, ...){
    char strTmpBuf[1000];
    char *pcTmp = strTmpBuf;
    int dbglevel = DEFAULT_DEBUGLEVEL;
    PT_DebugOpr ptTmp = g_ptDebugOprHead;

    va_list tArgs;
    int iNum;
    

    va_start(tArgs, pcFormat);
    iNum = vsprintf(strTmpBuf, pcFormat, tArgs);
    strTmpBuf[iNum] = '\0';
    va_end(tArgs);

    /* 根据打印级别判断是否打印 */
    if((strTmpBuf[0] == '<') && strTmpBuf[2] == '>'){
        //当前信息的打印级别
        dbglevel = strTmpBuf[1] - '0';
        if(dbglevel >= 0 && dbglevel <= 9){
            /* 当前打印级别是有效的，打印的时候不打印这个级别字符 */
            pcTmp = strTmpBuf + 3;
        }else{
            dbglevel = DEFAULT_DEBUGLEVEL;
        }
    }
    if(dbglevel > g_iDebugLevelLimit){
        return -1;
    }
    while(ptTmp){
        if(ptTmp->canUsed){
            ptTmp->DebugPrint(pcTmp);
        }
        ptTmp = ptTmp->ptNext;
    }

    return 0;
}

int DebugInit(void){
    int iError;
    iError = StdoutInit();
    iError |= NetPrintInit();
    return iError;
}

int InitDebugChannel(void){ 
    PT_DebugOpr ptTmp = g_ptDebugOprHead;
    while(ptTmp){
        if(ptTmp->canUsed && ptTmp->DebugInit){
            ptTmp->DebugInit();
        }
        ptTmp = ptTmp->ptNext;
    }
    return 0;
}