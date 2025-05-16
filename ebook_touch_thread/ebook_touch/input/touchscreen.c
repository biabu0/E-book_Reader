#include<input_manager.h>
#include <draw.h>
#include<sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <tslib.h>
#include <config.h>

#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include <linux/input.h>

#include <sys/ioctl.h>



static int g_iXres;
static int g_iYres;
static int TouchScreenDeviceExit();
static int TouchScreenDeviceInit();
static int TouchScreenGetInputEncent(PT_InputEvent ptInputEvent);


static struct tsdev *g_tTSDEV;

static T_InputOpr g_tTouchScreenOpr = {
    .name = "TouchScreen",
    .DeviceExit = TouchScreenDeviceExit,
    .DeviceInit = TouchScreenDeviceInit,
    .GetInputEnvent =TouchScreenGetInputEncent,
};


static int TouchScreenDeviceExit(){
    return 0;
}

/* 由于调用了LCD的分辨率，要在初始化显示屏之后调用 */
static int TouchScreenDeviceInit(){
    
    char *pcTSName = NULL;

    if((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL){
        // 设置为1，以非阻塞的方式打开
        g_tTSDEV = ts_open(pcTSName, 1);
    }else{
        g_tTSDEV = ts_open("dev/event0", 1);
    }

    if(!g_tTSDEV){
        DBG_PRINTF("ts_open error!\n");
        return -1;
    }

    if(ts_config(g_tTSDEV)){
        DBG_PRINTF("ts_config error!\n");
        return -1;
    }
    if(GetDispResolution(&g_iXres, &g_iYres)){
        DBG_PRINTF("GetDispResolution error!\n");
        return -1;
    }
    return 0;
}

static int isOutOf500ms(struct timeval *ptPreTime, struct timeval *ptCurTime){
    int iPreMs;
    int iCurMs;

    iPreMs = ptPreTime->tv_sec * 1000 + ptPreTime->tv_usec / 1000;
    iCurMs = ptCurTime->tv_sec * 1000 + ptCurTime->tv_usec / 1000;
    return (iCurMs > (iPreMs + 500));
}
static int TouchScreenGetInputEncent(PT_InputEvent ptInputEvent){
    static struct timeval tPreTime;
    struct ts_sample tSamp;
    int iRet;

    iRet = ts_read(g_tTSDEV, &tSamp, 1);
    if(iRet < 0){
        DBG_PRINTF("ts_read error!\n");
        return -1;
    }
    /* 处理数据 */  
    if(isOutOf500ms(&tPreTime, &tSamp.tv)){
        /* 一次触摸是会有多次的触摸事件的，如果此次触摸事件发生的时间，距离上次事件超过了500ms */
        tPreTime = tSamp.tv;
        ptInputEvent->tTime = tSamp.tv;
        ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
        if(tSamp.y < g_iYres / 3){
            ptInputEvent->iVal = INPUT_VAL_UP;
        }else if(tSamp.y > (g_iYres * 2 / 3)){
            ptInputEvent->iVal = INPUT_VAL_DOWN;
        }else{
            ptInputEvent->iVal = INPUT_VAL_UNKNOWN;
        }
        
    }else{
        return -1;
    }

    return 0;
}

int TouchScreenInit(void){
    return RegisterInputOpr(&g_tTouchScreenOpr);
}





