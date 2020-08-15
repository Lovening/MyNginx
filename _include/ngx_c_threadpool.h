
#ifndef __NGX_THREADPOOL_H__
#define __NGX_THREADPOOL_H__

#include <vector>
#include <pthread.h>
#include <atomic>   //c++11里的原子操作

//线程池相关类
class CThreadPool
{
public:
    CThreadPool();               
    
    virtual ~CThreadPool();                           

public:
    bool Create(int threadNum);                     
    void StopAll();                                 

    void InMsgRecvQueueAndSignal(char *buf);        //收到一个完整消息后，入消息队列，并触发线程池中线程来处理该消息
    void Call();                                   

private:
    static void* ThreadFunc(void *threadData);     
    void ClearMsgRecvQueue();                      

private:
    struct ThreadItem   
    {
        pthread_t   handle;                        //线程句柄
        CThreadPool *pThis;                        //记录线程池的指针	
        bool        ifrunning;                      //标记是否正式启动起来，启动起来后，才允许调用StopAll()来释放

        ThreadItem(CThreadPool *pthis):pThis(pthis),ifrunning(false){}                             
        ~ThreadItem(){}        
    };

private:
    static pthread_mutex_t     m_pThreadMutex;      //线程同步互斥量/也叫线程同步锁
    static pthread_cond_t      m_pThreadCond;       //线程同步条件变量
    static bool                m_bShutDown;         

    int                        m_iThreadNum;        

    std::atomic<int>           m_iRunningThreadNum; //线程数, 运行中的线程数，原子操作
    time_t                     m_iLastEmgTime;      //上次发生线程不够用【紧急事件】的时间,防止日志报的太频繁

    std::vector<ThreadItem *>  m_threadVector;      

    //接收消息队列相关
    std::list<char *>          m_MsgRecvQueue;      //接收数据消息队列 
	int                        m_iRecvMsgQueueCount;//收消息队列大小
};

#endif
