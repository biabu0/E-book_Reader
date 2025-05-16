/* 构造、设置、注册结构体*/

#include <config.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>


static int FBDeviceInit(void);		//初始化设备
static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor);		//显示字符
static int FBCleanScreen(unsigned int dwBackColor);		//清屏，用于翻页

static int g_iFBFd;			/*g:全局变量，t:结构体*/
static struct fb_var_screeninfo g_tVar;	/* Current var */
static struct fb_fix_screeninfo g_tFix;	/* Current fix */
static int g_iScreenSize;
static unsigned char *g_pucFbMem;		/*p:指针，uc:unsigned char*/
static int g_iLineWidth;
static int g_iPixelWidth;

static T_DispOpr g_tFBDispOpr = {
	.name 		 = "fb",
	.DeviceInit  = FBDeviceInit,
	.ShowPixel   = FBShowPixel,
	.CleanScreen = FBCleanScreen,
};

static int FBDeviceInit(void){
	g_iFBFd = open(FB_DEVICE_NAME, O_RDWR);
	if (g_iFBFd < 0)
	{
		DBG_PRINTF("can't open /dev/fb0\n");
		return -1;
	}

	if (ioctl(g_iFBFd, FBIOGET_VSCREENINFO, &g_tVar))
	{
		DBG_PRINTF("can't get var\n");
		return -1;
	}

	if (ioctl(g_iFBFd, FBIOGET_FSCREENINFO, &g_tFix))
	{
		DBG_PRINTF("can't get fix\n");
		return -1;
	}
	g_tFBDispOpr.iXres = g_tVar.xres;
	g_tFBDispOpr.iYres = g_tVar.yres;
	g_tFBDispOpr.iBpp  = g_tVar.bits_per_pixel;

	g_iLineWidth  = g_tVar.xres * g_tVar.bits_per_pixel / 8;
	g_iPixelWidth = g_tVar.bits_per_pixel / 8;
	g_iScreenSize = g_tVar.xres * g_tVar.yres * g_tVar.bits_per_pixel / 8;
	
	g_pucFbMem = (unsigned char *)mmap(NULL , g_iScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_iFBFd, 0);
	if (g_pucFbMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap\n");
		return -1;
	}
	return 0;
	
}

/*dwColor: 0xRRGGBB*/

static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor){
	unsigned char *pucPen8 = g_pucFbMem + iPenY * g_iLineWidth + iPenX * g_iPixelWidth;
	unsigned short *pwPen16; 
	unsigned int *pdwPen32;	
	unsigned short wColor16bpp; /* 565 */

	unsigned int red, green, blue;	

	pwPen16  = (unsigned short *)pucPen8;
	pdwPen32 = (unsigned int *)pucPen8;

	switch (g_tFBDispOpr.iBpp)
	{
		case 8:
		{
			*pucPen8 = dwColor;
			break;
		}
		case 16:
		{
			/* 565 */
			red   	 = (dwColor >> 16) & 0xff;
			green 	 = (dwColor >> 8) & 0xff;
			blue  	 = (dwColor >> 0) & 0xff;
			wColor16bpp  = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pwPen16 = wColor16bpp;
			break;
		}
		case 32:
		{
			*pdwPen32 = dwColor;
			break;
		}
		default:
		{
			DBG_PRINTF("can't surport %dbpp\n", g_tFBDispOpr.iBpp);
			return -1;
			break;
		}
	}
	return 0;

}
static int FBCleanScreen(unsigned int dwBackColor){
	unsigned int red, green, blue;	
	unsigned char *pucPen8 = g_pucFbMem;
	unsigned short *pwPen16; 
	unsigned int *pdwPen32;	
	unsigned short wColor16bpp; /* 565 */

	int i = 0;

	pwPen16  = (unsigned short *)pucPen8;
	pdwPen32 = (unsigned int *)pucPen8;

	switch (g_tFBDispOpr.iBpp)
	{
		case 8:
		{
			memset(pucPen8, dwBackColor, g_iScreenSize);
			break;
		}
		case 16:
		{
			/* 565 */
			red   	 = (dwBackColor >> 16) & 0xff;
			green 	 = (dwBackColor >> 8) & 0xff;
			blue  	 = (dwBackColor >> 0) & 0xff;
			wColor16bpp  = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);

			// 此处会出现错误，原因在于一次循环中i改变3个字节
			// for(i = 0; i < g_iScreenSize; i++){
			// 	*pwPen16 = wColor16bpp;
			// 	pwPen16++;
			// 	i += 2;
			// }

			while(i < g_iScreenSize){
				*pwPen16 = wColor16bpp;
				pwPen16++;
				i += 2;
			}


			break;
		}
		case 32:
		{
			// for(i = 0; i < g_iScreenSize; i++){
			// 	*pdwPen32 = dwBackColor;
			// 	pdwPen32++;
			// 	i += 4;
			// }
			while(i < g_iScreenSize){
				*pdwPen32 = dwBackColor;
				pdwPen32++;
				i += 4;
			}
			break;
		}
		default:
		{
			DBG_PRINTF("can't surport %dbpp\n", g_tVar.bits_per_pixel);
			return -1;
			break;
		}
	}
	return 0;
}

int FBInit(void){
	return RegisterDispOpr(&g_tFBDispOpr);
}


