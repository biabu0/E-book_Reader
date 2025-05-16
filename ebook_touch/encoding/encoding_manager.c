#include <config.h>
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr){
	PT_EncodingOpr pt_Tmp = g_ptEncodingOprHead;

	if(!g_ptEncodingOprHead){
		g_ptEncodingOprHead = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}else{
		while(pt_Tmp->ptNext){
			pt_Tmp = pt_Tmp->ptNext;
		}
		pt_Tmp->ptNext = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

	return 0;
	
}
void ShowEncodingOpr(void){
	int i = 0;
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;
	
	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}

}

PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead){
	PT_EncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{	
		if (ptTmp->isSupport(pucFileBufHead))
			return ptTmp;
		else
			ptTmp = ptTmp->ptNext;
	}
	return NULL;

}

int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr){
	PT_FontOpr ptFontOprCpy;

	/* 传入参数为空 */
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if (!ptFontOprCpy)
		{
			return -1;
		}
		else
		{	
			/* 在头部插入新支持的文件字体 */
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(T_FontOpr));
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;
			return 0;
		}		
	}


}
int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr){

	PT_FontOpr ptTmp;
	PT_FontOpr ptPre;
	
	/* 传入参数为空 */
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}else{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead;
		if(strcmp(ptTmp, ptFontOpr) == 0){
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}
		
		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;

		while(ptTmp){
			if(strcmp(ptTmp, ptFontOpr) == 0){
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}else{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext;
			}
		}
		return -1;

	}
}
int EncodingInit(void){

	int iError;

	iError = Utf8EncodingInit();
	if (iError)
	{
		DBG_PRINTF("Utf8EncodingInit error!\n");
		return -1;
	}

	return 0;
}





