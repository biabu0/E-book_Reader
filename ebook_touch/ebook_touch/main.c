#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "include/draw.h"
#include "include/encoding_manager.h"
#include "include/fonts_manager.h"
#include "include/config.h"
#include "include/input_manager.h"
#include "include/disp_manager.h"


/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
#define ArraySize 128

int main(int argc, char ** argv){

	int iError;
	int bList= 0;
	unsigned int dwFontSize = 16;	//默认大小
	char acFreetypeFile[ArraySize];
	char acHzkFile[ArraySize];
	char acDsiplay[ArraySize];
	char acTextFile[ArraySize];

	acFreetypeFile[0] 	= '\0';
	acHzkFile[0] 		= '\0';
	acTextFile[0] 		= '\0';

	T_InputEvent tInputEvent;

	// 默认是framebuffer
	strcpy(acDsiplay, "fb");

	// 解析参数
	while((iError = getopt(argc, argv, "ls:f:h:d")) != -1){
		switch(iError){
			// 列出支持编码格式，字体文件和显示设备
			case 'l':{
				bList = 1;
				break;
			}
			case 's':{
				dwFontSize = strtoul(optarg, NULL, 0);
				break;
			}
			case 'f':{
				// freetype 库
				strncpy(acFreetypeFile, optarg, ArraySize);
				acFreetypeFile[ArraySize-1] = '\0';
				break;
			}
			case 'h':{
				// 汉字库
				strncpy(acHzkFile, optarg, ArraySize);
				acHzkFile[ArraySize-1] = '\0';
				break;
			}
			case 'd':{
				// 显示设备
				strncpy(acDsiplay, optarg, ArraySize);
				acDsiplay[ArraySize-1] = '\0';
				break;
			}
			default:{
				printf("Usage: %s [-s Size] [-f freetype_font_file] [-h HZK] <text_file>\n", argv[0]);
				printf("Usage : %s -l\n", argv[0]);
				//	 结束运行即可
				return -1;
				break;
			}
		}
	}

	//optind 表示已经解析的参数的个数，如果optind < argc，则还有未解析的参数，需要继续解析， 否则代表解析结束
	// 当所有选项解析完毕后，optind应指向第一个非选项参数（即<text_file>）。若optind >= argc，则表明无剩余非选项参数‌，即没有提供<text_file>
	if(!bList && (optind >= argc)){
		printf("Usage: %s [-s Size] [-f freetype_font_file] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage : %s -l\n", argv[0]);
		//	 结束运行即可
		return -1;	
	}

	iError = DrawInit();
	if(iError != 0){
		printf("DisplayInit failed!\n");
		return -1;
	}

	// framebuffer 是 Linux 内核为显示设备提供的‌软件驱动接口‌，属于字符设备类别
	// 显示支持的类型
	if(bList){
		printf("Support display: \n");
		ShowDispOpr();

		printf("Support font: \n");
		ShowFontOpr();

		printf("Support encoding: \n");
		ShowEncodingOpr();

		printf("Support input: \n");
		ShowInputOpr();


		return 0;
	}

	// 解析完成后optind指向最后一个非选项参数，也就是<text file>

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	strncpy(acTextFile, argv[optind], ArraySize);
	acTextFile[ArraySize - 1] = '\0';

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	// 打开文本文件，获取有效字体编码位置
	iError = OpenTextFile(acTextFile);
	if(iError){
		printf("OpenTextFile failed\n");
		return -1;
	}

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	iError = SetFontDetail(acHzkFile, acFreetypeFile, dwFontSize);
	if(iError){
		printf("SetTextDetail failed\n");
		return -1;
	}

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = SelectAndInitDisplay(acDsiplay);
	if (iError)
	{
		printf("SelectAndInitDisplay error!\n");
		return -1;
	}

	iError = AllInputDeviceInit();
	if (iError)
	{
		printf("AllInputDeviceInit error!\n");
		return -1;
	}
	// 显示第一页
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = ShowNextPage();
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (iError)
	{
		printf("Error to show first page\n");
		return -1;
	}
	printf("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");
	printf("Touch the upper of touchscreen to show previous page");
	printf("Touch the lower of touchscreen to show next page");
	printf("Touch the middle of touchscreen to quit");

	while(1){
		
		// 获取有效操作
		if(GetInputEvent(&tInputEvent) == 0){
			if(tInputEvent.iVal == INPUT_VAL_DOWN){
				ShowNextPage();
			}else if(tInputEvent.iVal == INPUT_VAL_UP){
				ShowPrePage();
			}else if(tInputEvent.iVal == INPUT_VAL_EXIT){
				return 0;
			}
		}

		
	}

	return 0;
}