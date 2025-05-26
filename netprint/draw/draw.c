#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <string.h>
#include <input_manager.h>


/*定义一个页描述结构体，包含当前页数，当前LCD首个显示字符在文件中的位置以及下一页首个显示字符在文件中的位置；
 *结构体指针指向前或者下一页的页描述结构体；
 **/
typedef struct PageDesc{
    int iPageNo;
    unsigned char *pucLcdFirstPosAtFile;
    unsigned char *pucLcdNextPageFirstPosAtFile;
    struct PageDesc *ptNextPage;
    struct PageDesc *ptPrePage;
}T_PageDesc, *PT_PageDesc;


static int g_iFdTextFile;
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;
static PT_EncodingOpr g_ptEncodingOprForFile;
static PT_DispOpr g_ptDispOpr;

// 当前页面的起始位置
static unsigned char *g_pucLcdFirstPosAtFile;
// 当前页面下一行的位置
static unsigned char *g_pucLcdNextPosAtFile;
static int g_dwFontSize;


static PT_PageDesc g_ptPages   = NULL;
static PT_PageDesc g_ptCurPage = NULL;

int OpenTextFile(char *pcFileName){

    struct stat tStat;

    g_iFdTextFile = open(pcFileName, O_RDONLY);
    if(g_iFdTextFile < 0){
        DBG_PRINTF("open file %s failed\n", pcFileName);
        return -1;
    }

    // 管家级别的函数
    if(fstat(g_iFdTextFile, &tStat)){
        DBG_PRINTF("fstat failed\n");
        return -1;
    }

    g_pucTextFileMem = mmap(NULL, tStat.st_size, PROT_READ, MAP_SHARED, g_iFdTextFile, 0);
    if (g_pucTextFileMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for text file\n");
		return -1;
	}
    
    g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size;
    // 选择编码格式
    g_ptEncodingOprForFile = SelectEncodingOprForFile(g_pucTextFileMem);
    if(g_ptEncodingOprForFile){
        // 获取真正有效的字体编码值
        g_pucLcdFirstPosAtFile = g_pucTextFileMem + g_ptEncodingOprForFile->iHeadLen;
        return 0;
    }else{
        return -1;
    }
}
int SetFontDetail(char *pcHZKFile, char *pcFileFreetype, unsigned int dwFontSize){
    /*主要就是调用FontInit设置字体文件和字体大小*/
    PT_FontOpr ptFontOpr;
    PT_FontOpr ptTmp;
    int iError = 0;
    int iRet = -1;

    ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
    g_dwFontSize = dwFontSize;

    while(ptFontOpr){
        if(strcmp(ptFontOpr->name, "ascii") == 0){
            iError = ptFontOpr->FontInit(NULL, dwFontSize);
        }else if(strcmp(ptFontOpr->name, "gbk") == 0){
            iError = ptFontOpr->FontInit(pcHZKFile, dwFontSize);
        }else{
            iError = ptFontOpr->FontInit(pcFileFreetype, dwFontSize);
        }

        DBG_PRINTF("%s, %d\n", ptFontOpr->name, iError);

        ptTmp = ptFontOpr->ptNext;
        if(iError == 0){
            iRet = 0;
        }else{
            // 删除该字体
            DelFontOprFrmEncoding(g_ptEncodingOprForFile, ptFontOpr);
        }
        ptFontOpr = ptTmp;
    }
    return iRet;
}

int SelectAndInitDisplay(char *pcName){

    int iError;

    // 通过名称得到显示设备结构体
    g_ptDispOpr = GetDispOpr(pcName);
    if(!g_ptDispOpr){
        return -1;
    }

    // 调用初始化函数
    iError = g_ptDispOpr->DeviceInit();
    return iError;
}

/* 如果超出屏幕高度，则返回0 */
int IncLcdY(int iY)
{
	if (iY + g_dwFontSize < g_ptDispOpr->iYres)
		return (iY + g_dwFontSize);
	else
		return 0;
}


int RelocateFontPos(PT_FontBitMap ptFontBitMap)
{
	int iLcdY;
	int iDeltaX;
	int iDeltaY;

	if (ptFontBitMap->iYMax > g_ptDispOpr->iYres)
	{
		/* 满页了 */
		return -1;
	}

	/* 超过LCD最右边 */
	if (ptFontBitMap->iXMax > g_ptDispOpr->iXres)
	{
		/* 换行 */		
		iLcdY = IncLcdY(ptFontBitMap->iCurOriginY);
		if (0 == iLcdY)
		{
			/* 满页了 */
			return -1;
		}
		else
		{
			/* 没满页 */
			iDeltaX = 0 - ptFontBitMap->iCurOriginX;
			iDeltaY = iLcdY - ptFontBitMap->iCurOriginY;

			ptFontBitMap->iCurOriginX  += iDeltaX;
			ptFontBitMap->iCurOriginY  += iDeltaY;

			ptFontBitMap->iNextOriginX += iDeltaX;
			ptFontBitMap->iNextOriginY += iDeltaY;

			ptFontBitMap->iXLeft += iDeltaX;
			ptFontBitMap->iXMax  += iDeltaX;

			ptFontBitMap->iYTop  += iDeltaY;
			ptFontBitMap->iYMax  += iDeltaY;;
			
			return 0;	
		}
	}
	
	return 0;
}

int ShowOneFont(PT_FontBitMap ptFontBitMap){

    int x;
    int y;
    int i = 0;
    int bit;
    unsigned char ucByte = 0;

    //位图中一位表示一个像素；目前只支持一位表示一个像素
    if(ptFontBitMap->iBpp == 1){
        for(y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++){
            i = (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
            for (x = ptFontBitMap->iXLeft, bit = 7; x < ptFontBitMap->iXMax; x++){
                if (bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}
                if (ucByte & (1<<bit))
				{
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
				}
                bit--;
				if (bit == -1)
				{
					bit = 7;
				}

            }

        }

    }else{
        DBG_PRINTF("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
    }

    return 0;
}

int ShowOnePage(unsigned char *pucTextFileMemCurPos){

    int iLen;
    int iError;
    int bHasGetCode = 0;
    int bHasNotClrSceen = 1;
    
    PT_FontOpr ptFontOpr;
	T_FontBitMap tFontBitMap;

    unsigned char *pucBufStart;
    // 存放字体编码
    unsigned int dwCode;

    tFontBitMap.iCurOriginX = 0;
    tFontBitMap.iCurOriginY = g_dwFontSize;
    pucBufStart = pucTextFileMemCurPos;


    while(1){
        /* 获取Unicode编码到dwCode中*/
        iLen = g_ptEncodingOprForFile->GetCodeFrmBuf(pucBufStart, g_pucTextFileMemEnd, &dwCode);
        if(iLen == 0){
            /* 文件结束 */
            if(!bHasGetCode){
                return -1;
            }else{
                return 0;
            }
        }

        bHasGetCode = 1;

        pucBufStart += iLen;
        /* 有些文本, \n\r两个一起才表示回车换行
		 * 碰到这种连续的\n\r, 只处理一次
		 */
        if(dwCode == '\n'){
            // 下一字体点阵的位置
            g_pucLcdNextPosAtFile = pucBufStart;

            tFontBitMap.iCurOriginX = 0;
            tFontBitMap.iCurOriginY = IncLcdY(tFontBitMap.iCurOriginY);
            if(tFontBitMap.iCurOriginY == 0){
                /* 满屏 当前页面显示结束 */
                return 0;
            }else{
                continue;
            }
        }else if(dwCode == '\r'){
            continue;
        }else if(dwCode == '\t'){
            dwCode = ' ';
        }
        //DBG_PRINTF("dwCode = 0x%x\n", dwCode);

        ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
        while(ptFontOpr){
            //DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            /* 根据unicode码获取点阵信息*/
            iError = ptFontOpr->GetFontBitmap(dwCode, &tFontBitMap);
            //DBG_PRINTF("%s %s %d, ptFontOpr->name = %s, %d\n", __FILE__, __FUNCTION__, __LINE__, ptFontOpr->name, iError);
            if(iError == 0){
                /* 需要判断当前字体点阵是否超过屏幕限制 */
                if(RelocateFontPos(&tFontBitMap)){
                    return 0;
                }
                /* 清屏 */
                if(bHasNotClrSceen){
                    g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
                    bHasNotClrSceen = 0;

                }
                /* 根据点阵信息显示一个字体 */
                if(ShowOneFont(&tFontBitMap)){
                    return -1;
                }
                
                /* 当前的值应该等于下一个值 */
                tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
                tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
                g_pucLcdNextPosAtFile = pucBufStart;
                break;
            }
            ptFontOpr = ptFontOpr->ptNext;
        }
    }

    return 0;
}


// 记录已经打开的页
static void RecordPage(PT_PageDesc ptPageNew){
    PT_PageDesc ptPage;

    if(!g_ptPages){
        g_ptPages = ptPageNew;
    }else{
        ptPage = g_ptPages;
        while(ptPage->ptNextPage){
            ptPage = ptPage->ptNextPage;
        }
        ptPage->ptNextPage = ptPageNew;
        ptPageNew->ptPrePage = ptPage;
    }
}

int ShowNextPage(void){

    unsigned char *pucTextFileMemCurPos;
    PT_PageDesc ptPage;
    int iError;

    // 当前页面为空，即刚开始运行程序
    if(g_ptCurPage){
        pucTextFileMemCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;
    }else{
        pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;
    }
    // 得到一页的起始位置，可以显示一页了
    iError = ShowOnePage(pucTextFileMemCurPos);
    DBG_PRINTF("%s %d, %d\n", __FUNCTION__, __LINE__, iError);
    // 显示成功之后，设置下一页
    if(iError == 0){
       if (g_ptCurPage && g_ptCurPage->ptNextPage)
		{
			g_ptCurPage = g_ptCurPage->ptNextPage;
			return 0;
		} 

        ptPage = malloc(sizeof(T_PageDesc));
        if(ptPage){
            ptPage->pucLcdFirstPosAtFile            = pucTextFileMemCurPos;
            ptPage->pucLcdNextPageFirstPosAtFile    = g_pucLcdNextPosAtFile;
            ptPage->ptPrePage                        = NULL;
            ptPage->ptNextPage                        = NULL;
            g_ptCurPage = ptPage;

            DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)ptPage->pucLcdFirstPosAtFile);
            // 将该页面记录到链表
            RecordPage(ptPage);
            return 0;
        }else{
            return -1;
        }
    }
    return iError;
}
int ShowPrePage(void){
    int iError;

    if(!g_ptCurPage || !g_ptCurPage->ptPrePage){
        return -1;
    }
    DBG_PRINTF("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
    iError = ShowOnePage(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
    if(iError == 0){
        DBG_PRINTF("%s %d\n", __FUNCTION__, __LINE__);
        g_ptCurPage = g_ptCurPage->ptPrePage;
    }

    return iError;
    
}



int DrawInit(void){
    int iError;

    // 初始化各层
	iError = DisplayInit();
	if(iError){
		printf("DisplayInit failed\n");
		return -1;
	}

	iError = FontsInit();
	if(iError){
		printf("FontsInit failed\n");
		return -1;
	}

	iError = EncodingInit();
	if(iError){
		printf("EncodingInit failed\n");
		return -1;
	}

    iError = InputInit();
	if(iError){
		printf("InputInit failed\n");
		return -1;
	}

    return 0;
}

// 获取显示分辨率，用于触摸屏中判断触摸点位于上中下哪一部分
int GetDispResolution(int *piXres, int *piYres){
    if(g_ptDispOpr){
        *piXres = g_ptDispOpr->iXres;
        *piYres = g_ptDispOpr->iYres;
        return 0;
    }else{
        return -1;
    }
}

