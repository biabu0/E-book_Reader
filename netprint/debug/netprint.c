#include <config.h>
#include <debug_manager.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// 定义一个端口值
#define SERVER_PORT 8888
#define PRINT_BUF_SIZE  (16*1024)

// 全局变量，需要在多个函数中使用
static int g_iSocketServer;
static struct sockaddr_in g_tSocketServerAddr;
static struct sockaddr_in g_tSocketClientAddr;

static int g_iHaveConnected = 0;

// 不一定使用网络打印，故不适用数组的形式；只有当使用网络打印的时候再为其分配空间
static char *g_pcNetPrintBuf;
// 环形缓冲区读写位置
static int g_iReadPos = 0;
static int g_iWritePos = 0;
// 线程ID
static pthread_t g_tSendThreadID;
static pthread_t g_tRecvThreadID;

// 多线程访问共享资源需要互斥
static pthread_mutex_t g_tSendMutex = PTHREAD_MUTEX_INITIALIZER;
// 条件变量，一种同步机制，与互斥量一起使用允许线程以无竞争的方式等待特定事件的发生
static pthread_cond_t g_tSendConVar = PTHREAD_COND_INITIALIZER;


// 写位置加一模长度等于读位置即为满
static int isFull(void){
    return (g_iWritePos + 1) % PRINT_BUF_SIZE == g_iReadPos;
}
// 读位置等于写位置即为空
static int isEmpty(void){ 
    return g_iReadPos == g_iWritePos;
}
static int PutData(char cVal){
    if(isFull()){
        // 缓冲区已满，则不再写入
        return -1;
    }else{
        g_pcNetPrintBuf[g_iWritePos] = cVal;
        g_iWritePos = (g_iWritePos + 1) % PRINT_BUF_SIZE;
        return 0;
    }
}


static int GetData(char *pcVal){ 
    if(isEmpty()){
        return -1;
    }else{
        *pcVal = g_pcNetPrintBuf[g_iReadPos];
        g_iReadPos = (g_iReadPos + 1) % PRINT_BUF_SIZE;
        return 0;
    }
}

static void *NetDebugSendThread(void *pVoid){

    char strTmpBuf[512];
    int i;
    char cVal;

    while(1){
        /* 休眠状态 ：当客户端连接成功并且环形缓冲区有数据时，唤醒线程 */
        pthread_mutex_lock(&g_tSendMutex);
        pthread_cond_wait(&g_tSendConVar, &g_tSendMutex);
        pthread_mutex_unlock(&g_tSendMutex);
        /* 有客户端连接并且缓冲区中有数据 */
        while(g_iHaveConnected && !isEmpty()){
            i = 0;
            /* 将数据从环形缓冲区中取出数据，放入到要发送的缓冲区中 */
            while((i < 512) && (0 == GetData(&cVal))){
                strTmpBuf[i] = cVal;
                i++;
            }
            /* 使用sendto函数发送打印信息给客户端 */
            sendto(g_iSocketServer, strTmpBuf, i, 0, (const struct sockaddr *)&g_tSocketClientAddr, sizeof(struct sockaddr));
        }
        
    }
    return NULL;
}

static void *NetDebugRecvThread(void *pVoid){
    int iAddrLen;
    int iRecvLen;
    char cRecvBuf[1000];
    /* 客户端地址 */
    struct sockaddr_in tSocketClientAddr;
    while(1){
        iAddrLen = sizeof(struct sockaddr_in);
        iRecvLen = recvfrom(g_iSocketServer, cRecvBuf, 999, 0, (struct sockaddr *)&tSocketClientAddr, (socklen_t *)&iAddrLen);
        if(iRecvLen > 0){
            /* 加入结束符 */
            cRecvBuf[iRecvLen] = '\0';
            /* 解析数据：
            * dbglevel=0,1,2,3...    :修改打印级别
            * stdout = 0/1           :关闭/打开标准输出打印
            * netprint = 0/1         :关闭/打开网络打印
            * setclient              :设置客户端地址
            **/
           
           if(strcmp(cRecvBuf, "setclient") == 0){
            /* 设置客户端地址 ：全局变量给到发送线程，用于发送线程向该地址发送数据 */
                g_tSocketClientAddr = tSocketClientAddr;
                g_iHaveConnected = 1;
           }else if(strncmp(cRecvBuf, "dbglevel=", 9)  == 0){
            /* 设置调试等级 ：错误信息、警告信息、提示信息等*/
                SetDebugLevel(cRecvBuf);
           }else{
            /* 设置调试通道 ：*/
                SetDebugChannel(cRecvBuf);
           }

        }

    }

    return NULL;
}


static int NetDebugInit(void){
    /* 网络编程初始化：socket初始化 选择端口，ip*/
    /* 使用udp协议 */
    int iRet;
    g_iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
    if(g_iSocketServer == -1){
        printf("server: socket error!\n");
        return -1;
    }
    g_tSocketServerAddr.sin_port = htons(SERVER_PORT);
    g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
    g_tSocketServerAddr.sin_family = AF_INET;
    memset(g_tSocketServerAddr.sin_zero, 0, 8);
    iRet = bind(g_iSocketServer, (struct sockaddr *)&g_tSocketServerAddr, sizeof(g_tSocketServerAddr));
    if(iRet == -1){
        printf("server: bind error!\n");
        return -1;
    }

    // 初始化的时候表示要使用网络打印，这个时候可以分配缓冲区内存
    g_pcNetPrintBuf = malloc(PRINT_BUF_SIZE);
    // 缓冲区内存分配错误，关闭socket
    if(g_pcNetPrintBuf == NULL){
        close(g_iSocketServer);
        return -1;
    }

    /* 创建发送线程：用于发送打印信息给客户端 */
    pthread_create(&g_tSendThreadID, NULL, NetDebugSendThread, NULL);
    /* 创建接收线程：用于接收控制信息（修改打印级别，打开/关闭打印）*/
    pthread_create(&g_tRecvThreadID, NULL, NetDebugRecvThread, NULL);

    return 0;

}

static int NetDebugExit(void){
    /* 关闭socket,清理缓冲区 ... */
    close(g_iSocketServer);
    free(g_pcNetPrintBuf);
    return 0;
}



static int NetDebugPrint(char *strData){
    /* 将数据放入到环形缓冲区中*/
    int i;
    /* 将临时缓冲区中的数据放入到网络打印缓冲区中 */
    for(i = 0; i < strlen(strData); i++){
        if(0 != PutData(strData[i])){
            break;
        }
    }

    /* 如果有客户端连接，将数据通过网络发送给客户端 : 可以使用sendto或者唤醒线程 */
    /* 唤醒netprint的发送线程 */
    pthread_mutex_lock(&g_tSendMutex);
    pthread_cond_signal(&g_tSendConVar);
    pthread_mutex_unlock(&g_tSendMutex);

    return i;
}

static T_DebugOpr g_tNetDebugOpr = {
    .canUsed = 1,
    .name = "netprint",
    .DebugPrint = NetDebugPrint,
    .DebugExit = NetDebugExit,
    .DebugInit = NetDebugInit,
};


int NetPrintInit(void)
{
    return RegisterDebugOpr(&g_tNetDebugOpr);
}


