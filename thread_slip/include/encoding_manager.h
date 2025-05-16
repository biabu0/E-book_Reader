
#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

#include <fonts_manager.h>
#include <disp_manager.h>

typedef struct EncodingOpr {
	char *name;
	int iHeadLen;		/* 不同编码格式可能会有不同的头部 UTF-8有三个字节的头部 */
	PT_FontOpr ptFontOprSupportedHead;		/* 支持这个编码格式保存的文件字体有哪些 */
	int (*isSupport)(unsigned char *pucBufHead);	/* 根据文件前面的字节(前导码)判断该文件是否支持该编码格式 */
	int (*GetCodeFrmBuf)(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode); /* 从buffer中获取字体编码值 */
	struct EncodingOpr *ptNext;
}T_EncodingOpr, *PT_EncodingOpr;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr);
void ShowEncodingOpr(void);
PT_DispOpr GetDispOpr(char *pcName);
PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pucFileBufHead);
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr);
int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr);
int EncodingInit(void);
int  Utf8EncodingInit(void);

#endif /* _ENCODING_MANAGER_H */

