////
#ifndef __NGX_SOCKET_H__
#define __NGX_SOCKET_H__

#include <vector>
#include <sys/epoll.h> //epoll
#include <sys/socket.h>

#define NGX_LISTEN_BACKLOG  511    //已完成连接队列，nginx给511
#define NGX_MAX_EVENTS      512    //epoll_wait一次最多接收这么多个事件，nginx中缺省是512

typedef struct ngx_listening_s   ngx_listening_t, *lpngx_listening_t;
typedef struct ngx_connection_s  ngx_connection_t,*lpngx_connection_t;
typedef class  CSocket           CSocket;

typedef void (CSocket::*ngx_event_handler_pt)(lpngx_connection_t c); //定义成员函数指针

struct ngx_listening_s  
{
	int                       port;       
	int                       fd;          
	lpngx_connection_t        pconnection;  
};

struct ngx_connection_s
{
	
	int                       fd;            
	lpngx_listening_t         plistening;      

	unsigned                  instance:1;     //【位域】失效标志位：0：有效，1：失效
	uint64_t                  icur_sequence;  
	struct sockaddr           t_sockaddr;     //保存对方地址信息用的

	//和读有关的标志-----------------------
	uint8_t                   iwrite_ready;        //写准备好标记

	ngx_event_handler_pt      fp_rhandler;       
	ngx_event_handler_pt      fp_whandler;      
	
	lpngx_connection_t        pnext;          
};

//socket相关类
class CSocket
{
public:
	CSocket();                                           
	virtual ~CSocket();                                   

public:
    virtual bool Initialize();                      

public:	
	int  ngx_epoll_init();                                             //epoll功能初始化

	//epoll增加事件
	int  ngx_epoll_add_event(int fd,int readevent,int writeevent,uint32_t otherflag,uint32_t eventtype,lpngx_connection_t pconn);     
	
	//epoll等待接收和处理事件
	int  ngx_epoll_process_events(int timer);                         
private:
	void ReadConf();
	bool ngx_open_listening_sockets();                    
	void ngx_close_listening_sockets();                   
	bool setnonblocking(int sockfd);                      

//一些业务处理函数handler
	void ngx_event_accept(lpngx_connection_t pold_conn);                    //建立新连接
	void ngx_wait_request_handler(lpngx_connection_t c);               //设置数据来时的读处理函数

	void ngx_close_accepted_connection(lpngx_connection_t c);          //用户连入，我们accept4()时，得到的socket在处理中产生失败，则资源用这个函数释放【因为这里涉及到好几个要释放的资源，所以写成函数】

//获取对端信息相关                                              
	////根据参数1给定的信息，获取地址端口字符串，返回这个字符串的长度
	size_t ngx_sock_ntop(struct sockaddr *sa,int port,u_char *text,size_t len);  

//连接池 或 连接 相关
	//从连接池中获取一个空闲连接
	lpngx_connection_t ngx_get_connection(int isock);                  
	//归还参数c所代表的连接到到连接池中	
	void ngx_free_connection(lpngx_connection_t c);                    

private:
	int                            m_ilisten_port_count;    			 //所监听的端口数量
	int 						   m_iworker_connections;
	int                            m_iepoll_handle;                     

	std::vector<lpngx_listening_t> m_listen_socket_list;                

	//和连接池有关的
	lpngx_connection_t             m_pconnections;                     //连接池的首地址
	lpngx_connection_t             m_pfree_connections;                //空闲连接链表头
	                                                                	
	int                            m_iconnection;                     //当前进程中所有连接对象的总数【连接池大小】
	int                            m_ifree_connection;                //连接池中可用连接总数

	struct epoll_event             m_event_arr[NGX_MAX_EVENTS];           //用于在epoll_wait()中承载返回的所发生的事件
};


#endif
