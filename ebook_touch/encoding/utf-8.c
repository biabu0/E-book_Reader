
#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int isUtf8Coding(unsigned char *pucBufHead);	/* 根据文件前面的字节(前导码)判断该文件是否支持该编码格式 */
static int Utf8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode); 

static T_EncodingOpr g_tUtf8EncodingOpr = {
	.name = "utf-8",
	.iHeadLen = 3,
	.isSupport = isUtf8Coding,
	.GetCodeFrmBuf = Utf8GetCodeFrmBuf,
};


static int isUtf8Coding(unsigned char *pucBufHead){

	const char aStrUtf8[] = {0xEF, 0xBB, 0xBF, 0};
	
	if(strncmp(pucBufHead, aStrUtf8, 3) == 0){
		return 1;
	}else{
		return 0;
	}
		
}


static int GetPreOneBits(unsigned char ucVal)
{
	int i;
	int j = 0;
	
	for (i = 7; i >= 0; i--)
	{
		if (!(ucVal & (1<<i)))
			break;
		else
			j++;
	}
	return j;

}

/* 获取Unicode码 */
static int Utf8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode){
	int i;	
	int iNum;
	unsigned char ucVal;
	unsigned int dwSum = 0;

	if (pucBufStart >= pucBufEnd)
	{
		/* 文件结束 */
		return 0;
	}

	ucVal = pucBufStart[0];
	iNum  = GetPreOneBits(pucBufStart[0]);

	if ((pucBufStart + iNum) > pucBufEnd)
	{
		/* 文件结束 */
		return 0;
	}

	if (iNum == 0)
	{
		/* ASCII */
		*pdwCode = pucBufStart[0];
		return 1;
	}
	else
	{
		ucVal = ucVal << iNum;
		ucVal = ucVal >> iNum;
		dwSum += ucVal;
		for (i = 1; i < iNum; i++)
		{
			ucVal = pucBufStart[i] & 0x3f;
			dwSum = dwSum << 6;
			dwSum += ucVal;			
		}
		*pdwCode = dwSum;
		return iNum;
	}
}

int Utf8EncodingInit(void){
	AddFontOprForEncoding(&g_tUtf8EncodingOpr, GetFontOpr("freetype"));
	return RegisterEncodingOpr(&g_tUtf8EncodingOpr);
}






