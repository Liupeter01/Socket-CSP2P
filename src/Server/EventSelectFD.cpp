#include"ServerSocketFunction.h"

EventSelectStruct::EventSelectStruct(const Socket& listenserver, timeval& _timeval)
          :m_listenServer(listenserver),
          m_timeset(new timeval (_timeval))    
{
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
          FD_SET(listenserver.m_socket, &this->m_fdRead);
}

int EventSelectStruct::StartSelect()
{
#ifdef WINDOWSPLATFROM
          return ::select(m_listenServer.m_socket + 1, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
#endif
}

int EventSelectStruct::isSelectSocketRead()                                               //判断是否设置读取描述符
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //判断是否设置读取描述符
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //判断是否设置读取描述符
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#endif
}

void  EventSelectStruct::cleanSelectSocketRead(const Socket &s)                      //清除Select模型的写入
{
#ifdef WINDOWSPLATFROM  
          FD_CLR(s.m_socket, &m_fdRead);
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //清除Select模型的发送
{
#ifdef WINDOWSPLATFROM
          FD_CLR(s.m_socket, &m_fdWrite);
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //清除Select模型的异常
{
#ifdef WINDOWSPLATFROM
          FD_CLR(s.m_socket, &m_fdException);
#endif
}


void EventSelectStruct::updateClientConn(const std::vector<Socket>& vec)
{
          for (int i = vec.size() - 1; i >= 0; --i) {
                    FD_SET(vec.at(i).m_socket, &m_fdRead);
          }
}

void EventSelectStruct::updateClientConn(const Socket& s)
{
          FD_SET(s.m_socket, &m_fdRead);
}

size_t  EventSelectStruct::getReadCount()                                                           //读取的Select数据个数
{
          return m_fdRead.fd_count;
}

size_t  EventSelectStruct::getWriteCount()                                                            //写入的Select数据个数
{
          return m_fdWrite.fd_count;
}

size_t  EventSelectStruct::getExceptionCount()                                                            //异常的Select数据个数
{
          return m_fdException.fd_count;
}

std::vector<Socket>::iterator EventSelectStruct::getReadSocket( std::vector<Socket>& vec, int pos)
{
          for (auto ib = vec.begin(); ib != vec.end(); ib++) {
                    
                    if ((*ib).getSocketConnStatus() && (*ib).getSocket() == this->m_fdRead.fd_array[pos]) {                 //判断连接状态和匹配状态       
                              return ib;
                    }
          }
          return vec.end();             //没有找到
}

EventSelectStruct::~EventSelectStruct() 
{
          delete m_timeset;
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
}