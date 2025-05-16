#include <config.h>
#include <fonts_manager.h>
#include <string.h>

static PT_FontOpr g_ptFontOprHead = NULL;

int RegisterFontOpr(PT_FontOpr ptFontOpr){
	PT_FontOpr pt_Tmp;
	
	if(!g_ptFontOprHead){
		g_ptFontOprHead = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}else{
		pt_Tmp = g_ptFontOprHead;
		while(pt_Tmp->ptNext){
			pt_Tmp = pt_Tmp->ptNext;
		}
		pt_Tmp->ptNext = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}
	
	return 0;
}
void ShowFontOpr(void){

	PT_FontOpr pt_Tmp = g_ptFontOprHead;
	int i = 0;
	
	while(pt_Tmp){
		printf("%02d  %s\n", i, pt_Tmp->name);
		i++;
		pt_Tmp = pt_Tmp->ptNext;
	}
}

PT_FontOpr GetFontOpr(char *pcName){
	PT_FontOpr pt_Tmp = g_ptFontOprHead;
	
	while(pt_Tmp){
		if(strcmp(pt_Tmp->name, pcName) == 0){
			return pt_Tmp;
		}
		pt_Tmp = pt_Tmp->ptNext;
	}
	
	return NULL;
}


int FontsInit(void){
	int iError;
	iError = FreeTypeInit();
	if(iError){
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}
	return 0;
}






