#include<input_manager.h>
#include <draw.h>
#include<sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <tslib.h>
#include <config.h>

static int g_iXres;
static int g_iYres;
static int TouchScreenDeviceExit();
static int TouchScreenDeviceInit();
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent);


static struct tsdev *g_tTSDEV;

static T_InputOpr g_tTouchScreenOpr = {
    .name = "TouchScreen",
    .DeviceExit = TouchScreenDeviceExit,
    .DeviceInit = TouchScreenDeviceInit,
    .GetInputEnvent =TouchScreenGetInputEvent,
};


static int TouchScreenDeviceExit(){
    return 0;
}

/* 由于调用了LCD的分辨率，要在初始化显示屏之后调用 */
static int TouchScreenDeviceInit(){
    
    char *pcTSName = NULL;

    if((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL){
        // 设置为0，以阻塞的方式打开
        g_tTSDEV = ts_open(pcTSName, 0);
    }else{
        g_tTSDEV = ts_open("/dev/input/event1", 0);
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
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent){
    // 记录压下的值
    struct ts_sample tSampPressed;
    // 记录松开时候的值
    struct ts_sample tSampReleased;
    struct ts_sample tSamp;
    int iRet;
    int iStart = 0;
    int iDelta;
    
    while(1){
        iRet = ts_read(g_tTSDEV, &tSamp, 1);/* 如果没有数据则休眠 */
        if(iRet == 1){
            // 压下
            if((tSamp.pressure > 0) && (iStart == 0)){
                //第一次按下，记录按下值 
                tSampPressed = tSamp;
                iStart = 1;
            }
            // 松开的时候，记录松开的位置
            if(tSamp.pressure <= 0){
                tSampReleased = tSamp;
                if(!iStart){
                    return -1;
                }else{
                    iDelta = tSampReleased.x - tSampPressed.x;
                    /* 输入事件赋值 */ 
                    ptInputEvent->tTime = tSampReleased.tv;
                    ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
                    if(iDelta > g_iXres / 5){
                        /* 上一页的事件 */
                        ptInputEvent->iVal = INPUT_VAL_UP;

                    }
                    else if(iDelta < 0 - g_iXres / 5){
                        /* 下一页的事件 */
                        ptInputEvent->iVal = INPUT_VAL_DOWN;
                    }
                    else{
                        /* 不是有效的输入事件 */
                        ptInputEvent->iVal = INPUT_VAL_UNKNOWN;
                    }
                    return 0;
                }   
            }
        }
        else{
            return -1;
        }
    }
    return 0;
}

int TouchScreenInit(void){
    return RegisterInputOpr(&g_tTouchScreenOpr);
}





