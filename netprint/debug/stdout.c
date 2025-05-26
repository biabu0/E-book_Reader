#include <config.h>
#include <debug_manager.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


static int StdoutDebugPrint(char *strData) {
    /* 标准输出： 直接将输出信息用printf打印即可 */
    printf("%s", strData);

    return strlen(strData);// 返回已经成功打印的字符数
}


// 分配注册一个结构体
static T_DebugOpr g_tStdoutDebugOpr = {
    .canUsed = 1,
    .name = "stdout",
    .DebugPrint = StdoutDebugPrint,
    // 对于标准输出，不需要做什么初始化和退出操作
    // 因此这里直接设置为空即可，当后续需要调用这两个函数的时候需要判断是否存在这两个函数
    // .DebugExit = StdoutDebugExit,
    // .DebugInit = StdoutDebugInit,
};

// 注册结构体
int StdoutInit(void){
    return RegisterDebugOpr(&g_tStdoutDebugOpr);
}


