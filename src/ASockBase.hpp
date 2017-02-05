
///////////////////////////////////////////////////////////////////////////////

#ifndef __A_SOCKET_BASE_HPP__
#define __A_SOCKET_BASE_HPP__

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <string>
#include <mutex> 


#include <sys/epoll.h>
#include "CumBuffer.h"

using namespace std;

typedef struct  sockaddr_in SOCKADDR_IN ;
typedef struct  sockaddr_un SOCKADDR_UN ;
typedef struct  sockaddr    SOCKADDR ;
typedef         socklen_t   SOCKLEN_T ;

namespace asocklib
{
    const int       DEFAULT_PACKET_SIZE =2048;
    const int       DEFAULT_CAPACITY    =2048;
    const size_t    MORE_TO_COME        = -1;
    //const int DEFAULT_CAPACITY    =10;

    typedef enum
    {
        SOCKET_OK    = 1, 
        SELECT_TIME_OUT ,
        RECONNECT       ,
        RETRY           ,
        BUFFERED        ,
        MAX_SOCKET_RETURN_TYPE
    } ENUM_SOCKET_RETURN_TYPE ;
};

///////////////////////////////////////////////////////////////////////////////
class Context 
{
    public :

        CumBuffer       recvBuffer_;
        CumBuffer       sendBuffer_; 
        int             socket_{-1};
        std::mutex      clientSendLock_; 
};

///////////////////////////////////////////////////////////////////////////////
class ASockBase
{
    public :

        ASockBase(){};
        ASockBase(int nMaxMsgLen);
        virtual ~ASockBase() =default ;
        bool            SetBufferCapacity(int nMaxMsgLen);
        std::string     GetLastErrMsg(){return strErr_; }
        bool            SetNonBlocking(int nSockFd);

    protected :
        std::string     strErr_ ;
        char            szRecvBuff_     [asocklib::DEFAULT_PACKET_SIZE];
        char            szOnePacketData_[asocklib::DEFAULT_PACKET_SIZE];
        int             nBufferCapcity_ {-1};

        //epoll
        struct          epoll_event* pEpEvents_{nullptr};
        int             nEpfd_          {-1};

    protected :
        bool            Recv(Context* pContext);
        bool            Send(Context* pContext, const char* pData, int nLen); 
        void            EpollCtlModify(Context* pClientContext , uint32_t events);

    private:
        virtual size_t  GetOnePacketLength(Context* pContext)=0; 
        virtual bool    OnRecvOnePacketData(Context* pContext, char* pOnePacket, int nPacketLen)=0; 
};

#endif 


