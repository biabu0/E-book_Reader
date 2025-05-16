
#include <config.h>
#include <disp_manager.h>
#include <string.h>


static PT_DispOpr g_ptDispOprHead;	//结构体链表头部

/*		注册结构体			将结构体加入到链表中取*/
int RegisterDispOpr(PT_DispOpr ptDispOpr){

	PT_DispOpr ptTmp;
	
	if(!g_ptDispOprHead){
		g_ptDispOprHead = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}else{
		ptTmp = g_ptDispOprHead;
		while(ptTmp->ptNext){
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}
	
	return 0;
}

void ShowDispOpr(void){

	PT_DispOpr ptTmp = g_ptDispOprHead;
	int i = 0;
	
	while(ptTmp){
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_DispOpr GetDispOpr(char *pcName){

	PT_DispOpr ptTmp = g_ptDispOprHead;

	while(ptTmp){
		if(strcmp(ptTmp->name, pcName) == 0){
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}


int DisplayInit(void){
	int iError;

	iError = FBInit();

	return iError;
}



