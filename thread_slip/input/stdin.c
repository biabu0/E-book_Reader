
#include <input_manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include<sys/time.h>

#define	STDIN_FILENO	0

static int StdinDeviceExit(void);
static int StdinDeviceInit(void);
static int StdinGetInputEncent(PT_InputEvent ptInputEvent);


static T_InputOpr g_tStdinOpr = {
    .name = "stdin",
    .DeviceExit = StdinDeviceExit,
    .DeviceInit = StdinDeviceInit,
    .GetInputEnvent =StdinGetInputEncent,
};

// 对标准输入进行初始化
// 轮询方式读取数据，要求只有终端有数据就立刻返回
static int StdinDeviceInit(void){
    /*
        功能：配置终端的输入模式，使其进入非规范模式（即无需按回车即可读取输入），并设置读取字符的最小数量
        return 0：成功
    **/

    struct termios tTYYState;

    //获取终端状态
    tcgetattr(STDIN_FILENO, &tTYYState);
    //设置终端状态
    tTYYState.c_lflag &= ~ICANON;
    //最小读取输入字符数
    tTYYState.c_cc[VMIN] = 1;
    // 设置终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &tTYYState);

    return 0;

}

static int StdinDeviceExit(void){
    return RegisterInputOpr(&g_tStdinOpr);
}

static int StdinGetInputEncent(PT_InputEvent ptInputEvent){
    /* 
     * 如果存在数据则读取、处理并返回；
     * 如果没有数据，立刻返回，不等待
     **/
    char c;

    // 监听的读取描述符的列表
    //  判断该描述符是否准备好进行IO
    ptInputEvent->iType = INPUT_TYPE_STDIN;
    /* 处理数据 会休眠直到有输入 */
    c = fgetc(stdin);
    gettimeofday(&ptInputEvent->tTime, NULL);
    switch(c){
        case 'u':
            ptInputEvent->iVal = INPUT_VAL_UP;
            break;
        case 'n':
            ptInputEvent->iVal = INPUT_VAL_DOWN;
            break;
        case 'q':
            ptInputEvent->iVal = INPUT_VAL_EXIT;
            break;
        default:
            ptInputEvent->iVal = INPUT_VAL_UNKNOWN;
            break; 
    }
    return 0;
}
int StdinInit(void){
    return RegisterInputOpr(&g_tStdinOpr);
}

