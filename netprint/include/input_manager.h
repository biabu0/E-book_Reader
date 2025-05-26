#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include <sys/time.h>
#include <pthread.h>

#define INPUT_TYPE_STDIN 0  
#define INPUT_TYPE_TOUCHSCREEN 1

#define INPUT_VAL_UP 0
#define INPUT_VAL_DOWN 1
#define INPUT_VAL_EXIT 2
#define INPUT_VAL_UNKNOWN -1

// 保存输入事件的结构体
typedef struct InputEvent{
    struct timeval tTime;
    // 类别：按键类还是触摸屏类等       0或者1
    int iType;
    // val 值表示动作
    int iVal;  

} T_InputEvent, *PT_InputEvent;

typedef struct InputOpr{
    char * name;
    pthread_t pid;
    // 初始化与退出
    int (*DeviceInit)(void);
    int (*DeviceExit)(void);
    // 获取输入事件
    int (*GetInputEnvent)(PT_InputEvent ptInputEvent);
    // 链表
    struct InputOpr *ptNext;
}T_InputOpr, *PT_InputOpr;

int AllInputDeviceInit(void);
int InputInit(void);
int RegisterInputOpr(PT_InputOpr ptInputOpr);
int ShowInputOpr(void);
int GetInputEvent(PT_InputEvent ptInputEvent);
int TouchScreenInit(void);
int StdinInit(void);

#endif