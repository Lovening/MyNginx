
//和 线程池 有关的函数放这里
/*
王健伟老师 《Linux C++通讯架构实战》
商业级质量的代码，完整的项目，帮你提薪至少10K
*/

#include <stdarg.h>
#include <unistd.h>  //usleep

#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_threadpool.h"
#include "ngx_c_memory.h"
#include "ngx_macro.h"
#include "ngx_c_slogic.h"

//静态成员初始化
pthread_mutex_t CThreadPool::m_pThreadMutex = PTHREAD_MUTEX_INITIALIZER;  //#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) -1)
pthread_cond_t CThreadPool::m_pThreadCond = PTHREAD_COND_INITIALIZER;     //#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) -1)
bool CThreadPool::m_bShutDown = false;    

//构造函数
CThreadPool::CThreadPool()
{
    m_iRunningThreadNum = 0;  
    m_iLastEmgTime = 0;      
    m_iRecvMsgQueueCount = 0; 
    m_iThreadNum = 1;
}

//析构函数
CThreadPool::~CThreadPool()
{    
    //资源释放在StopAll()里统一进行，就不在这里进行了

    //接收消息队列中内容释放
    ClearMsgRecvQueue();
}

//各种清理函数-------------------------
//清理接收消息队列，注意这个函数的写法。
void CThreadPool::ClearMsgRecvQueue()
{
	char * sTmpMempoint;
	CMemory *p_memory = CMemory::GetInstance();

	//尾声阶段，需要互斥？该退的都退出了，该停止的都停止了，应该不需要退出了
	while(!m_MsgRecvQueue.empty())
	{
		sTmpMempoint = m_MsgRecvQueue.front();		
		m_MsgRecvQueue.pop_front(); 
		p_memory->FreeMemory(sTmpMempoint);
	}	
}

//创建线程池中的线程
bool CThreadPool::Create(int threadNum)
{    
   m_iThreadNum = threadNum;

   for (int i = 0; i < m_iThreadNum; ++i)
   {
       ThreadItem *pNewItem = new ThreadItem(this);
       m_threadVector.push_back(pNewItem);

       int err = pthread_create(&pNewItem->handle, NULL, ThreadFunc, pNewItem);

       if (err != 0)
       {
            ngx_log_stderr(err,"CThreadPool::Create()创建线程%d失败，返回的错误码为%d!",i,err);
            return false;
       }
   }

    std::vector<ThreadItem*>::iterator iter;

lblfor:
    for (iter = m_threadVector.begin(); iter != m_threadVector.end(); ++iter)
    {
        if (!(*iter)->ifrunning)
        {
            usleep(100 * 1000);
            
            goto lblfor;
        }
    }

    return true;
}

//线程入口函数
void* CThreadPool::ThreadFunc(void* threadData)
{
    ThreadItem *pThreadItem = (ThreadItem*)threadData;
    CThreadPool *pThreadPool = pThreadItem->pThis;

    CMemory *pmemory = CMemory::GetInstance();	  
    pthread_t tid = pthread_self();
    int err = 0;

    //两层while
    while(true)
    {
        err = pthread_mutex_lock(&m_pThreadMutex);
        if (err != 0)
        {
            ngx_log_stderr(err, "CThreadPool::ThreadFunc()中pthread_mutex_lock()失败，错误码为%d", err);
        }

        while(pThreadPool->m_MsgRecvQueue.empty() && !m_bShutDown)
        {
            if (!pThreadItem->ifrunning)
            {
                pThreadItem->ifrunning = true;
            }

            pthread_cond_wait(&m_pThreadCond, &m_pThreadMutex);
        }

        if  (m_bShutDown)
        {
            pthread_mutex_unlock(&m_pThreadMutex);
            break;
        }

        //取消息
        char *pjobbuf = pThreadPool->m_MsgRecvQueue.front();
        pThreadPool->m_MsgRecvQueue.pop_front();
        --pThreadPool->m_iRecvMsgQueueCount;

        err = pthread_mutex_unlock(&m_pThreadMutex);
        if(err != 0)  
        {
            ngx_log_stderr(err,"CThreadPool::ThreadFunc()pthread_cond_wait()失败，返回的错误码为%d!",err);
        }

        //线程处理消息
        ++pThreadPool->m_iRunningThreadNum;
        g_socket.threadRecvProcFunc(pjobbuf);
        pmemory->FreeMemory(pjobbuf);
        --pThreadPool->m_iRunningThreadNum;

    }

    return (void*)0;
}

//停止所有线程
void CThreadPool::StopAll() 
{
    if (m_bShutDown)
        return;

    m_bShutDown = true;

    //唤醒条件
    int err = pthread_cond_broadcast(&m_pThreadCond);
    if (err != 0)
    {
        ngx_log_stderr(err,"CThreadPool::StopAll()中pthread_cond_broadcast()失败，返回的错误码为%d!",err);
        return;
    }

    //等待完成
    for(int i = 0; i < m_threadVector.size(); ++i)
    {
        ThreadItem *pItem = m_threadVector.at(i);
        pthread_join(pItem->handle, NULL);

        delete pItem;
    }

    m_threadVector.clear();

    pthread_mutex_destroy(&m_pThreadMutex);
    pthread_cond_destroy(&m_pThreadCond);

    ngx_log_stderr(0,"CThreadPool::StopAll()成功返回，线程池中线程全部正常结束!");

}

//收到一个完整消息后，入消息队列，并触发线程池中线程来处理该消息
void CThreadPool::InMsgRecvQueueAndSignal(char *buf)
{
    int err = pthread_mutex_lock(&m_pThreadMutex);
    if (err != 0)
    {
        ngx_log_stderr(err, "CThreadPoll::InMsgRecvQueueAndSign()中pthread_lock()调用失败，错误码为%s",  err);
    }

    m_MsgRecvQueue.push_back(buf);
    ++m_iRecvMsgQueueCount;

    err = pthread_mutex_unlock(&m_pThreadMutex);   
    if(err != 0)
    {
        ngx_log_stderr(err,"CThreadPool::inMsgRecvQueueAndSignal()pthread_mutex_unlock()失败，返回的错误码为%d!",err);
    }

    Call();
}

//激发
void CThreadPool::Call()
{
    int err = pthread_cond_signal(&m_pThreadCond);
    if (err != 0)
    {
            ngx_log_stderr(err,"CThreadPool::Call()中pthread_cond_signal()失败，返回的错误码为%d!",err);
    }

    //告警
    if (m_iThreadNum == m_iRunningThreadNum)
    {
        time_t curtime = time(NULL);
        if (curtime - m_iLastEmgTime > 10)
        {
            ngx_log_stderr(0,"CThreadPool::Call()中发现线程池中当前空闲线程数量为0，要考虑扩容线程池了!");
            m_iLastEmgTime = curtime;
        }
        
    }
}

