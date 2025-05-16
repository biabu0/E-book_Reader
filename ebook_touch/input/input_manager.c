#include <config.h>
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead = NULL;

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

int AllInputDeviceInit(){
    PT_InputOpr ptTmp = g_ptInputOprHead;
    int iError = -1;

    while(ptTmp){
        if(0 == ptTmp->DeviceInit()){
            iError = 0;
        }
        ptTmp = ptTmp->ptNext;
    }
    return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent){ 
    /* 把链表中的InputOpr的GetInputEvent函数调用起来,有数据则返回*/
    PT_InputOpr ptTmp = g_ptInputOprHead;

    while(ptTmp){
        if(0 == ptTmp->GetInputEnvent(ptInputEvent)){
            return 0;
        }
        ptTmp = ptTmp->ptNext;
    }
    return -1;
}

int InputInit(void){
    int iError;
    iError = StdinInit();
    iError = TouchScreenInit();
    return iError;
}