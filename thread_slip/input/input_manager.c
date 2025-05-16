#include <config.h>
#include <input_manager.h>
#include <string.h>
#include <pthread.h>

static T_InputEvent g_tInputEvent;
static PT_InputOpr g_ptInputOprHead = NULL;
// 多线程访问共享资源需要互斥
static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER;
// 条件变量，一种同步机制，与互斥量一起使用允许线程以无竞争的方式等待特定事件的发生
static pthread_cond_t g_tConVar = PTHREAD_COND_INITIALIZER;
int RegisterInputOpr(PT_InputOpr ptInputOpr){
    PT_InputOpr ptTmp;

    if(!g_ptInputOprHead){
        g_ptInputOprHead = ptInputOpr;
        ptInputOpr->ptNext = NULL;
    }else{
        ptTmp = g_ptInputOprHead;
        while(ptTmp->ptNext){
            ptTmp = ptTmp->ptNext;
        }
        ptTmp->ptNext = ptInputOpr;
        ptInputOpr->ptNext = NULL;
    }
    return 0;
}

int ShowInputOpr(void){

    PT_InputOpr ptTmp = g_ptInputOprHead;
    int i = 0;

    while(ptTmp){
        printf("%02d %s\n", i++, ptTmp->name);
        ptTmp = ptTmp->ptNext;
    }
    return 0;
}

static void *InputEventTreadFunc(void *pVoid){
    //线程函数可以获得并调用输入设备的GetInputEnvent函数
    /* 定义函数指针 */ 
    T_InputEvent tInputEvent;

    int (*GetInputEnvent)(PT_InputEvent ptInputEvent);
    GetInputEnvent = (int (*) (PT_InputEvent))pVoid;
    while(1){
        if(0 == GetInputEnvent(&tInputEvent)){
            /* 唤醒主线程，把tInputEvent值赋值给一个全局变量，访问临界资源则需要进行加锁 */
            pthread_mutex_lock(&g_tMutex);
            // 将子线程获取的输入事件赋值给全局变量，主线程从这个全局变量中读取输入事件
            g_tInputEvent = tInputEvent;
            // 唤醒主线程
            pthread_cond_signal(&g_tConVar);
            pthread_mutex_unlock(&g_tMutex);
        }
    }

    return NULL;
}

int AllInputDeviceInit(){
    PT_InputOpr ptTmp = g_ptInputOprHead;
    int iError = -1;

    while(ptTmp){
        if(0 == ptTmp->DeviceInit()){
            /* 创建子线程 :将ptTmp->GetInputEnvent得到的输入事件传递给子线程*/
            pthread_create(&ptTmp->pid, NULL, InputEventTreadFunc, ptTmp->GetInputEnvent);
            iError = 0;
        }
        ptTmp = ptTmp->ptNext;
    }
    return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent){ 
    /* main函数调用 */
    /* 休眠 */
    pthread_mutex_lock(&g_tMutex);
    pthread_cond_wait(&g_tConVar, &g_tMutex);
    /* 被唤醒后，返回数据 */
    *ptInputEvent = g_tInputEvent;
    pthread_mutex_unlock(&g_tMutex);
    return 0;

}

int InputInit(void){
    int iError;
    iError = StdinInit();
    iError = TouchScreenInit();
    return iError;
}