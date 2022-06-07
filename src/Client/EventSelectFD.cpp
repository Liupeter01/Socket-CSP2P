#include"ClientSocketFunction.h"

#ifndef _WIN3264     //Linux平台专属fd_set结构体
_fd_set::_fd_set() :m_sizeCount(0) {
          _FD_ZERO();
}
_fd_set::~_fd_set() {
          m_sizeCount = 0;
          _FD_ZERO();
}
void _fd_set::_FD_ZERO() {
          ::FD_ZERO(&m_fd_set.fds_bits);
          m_sizeCount = 0;
}
void _fd_set::_FD_SET(Socket& socket) {
          ::FD_SET(socket.m_socket, &m_fd_set.fds_bits);
          m_sizeCount++;
}
void _fd_set::_FD_CLR(Socket& socket) {
          ::FD_CLR(socket.m_socket, &m_fd_set.fds_bits);
          m_sizeCount--;
}
int _fd_set::_FD_ISSET(Socket& socket)
{
          return ::FD_ISSET(socket.m_socket, &m_fd_set.fds_bits);
}
int _fd_set::getFdSetSize() {
          return m_sizeCount;
}
bool _fd_set::getFdStatus(Socket& socket) {
          return static_cast<bool>(m_fd_set.fds_bits[static_cast<int>(socket.m_socket) - 1]);
}
#endif // _WIN3264     //Linux平台专属fd_set结构体

EventSelectStruct::EventSelectStruct(const Socket& _socket)
          :m_listenServer(_socket),
          m_timeset(NULL)
{
#ifdef _WIN3264
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
          FD_SET(m_listenServer.m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(m_listenServer.m_socket);
#endif // _WIN3264

}

EventSelectStruct::EventSelectStruct(const Socket& _socket, timeval& _timeval)
          :m_listenServer(_socket),
          m_timeset(new timeval(_timeval))
{
#ifdef _WIN3264
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
          FD_SET(m_listenServer.m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(m_listenServer.m_socket);
#endif // _WIN3264
}

int EventSelectStruct::StartSelect(Socket& _client)
{
          SOCKET temp;
#ifdef _WIN3264                         //WINDOWS计算模式
          int number = 0;
          for (size_t i = 0; i < getReadCount(); ++i) {
                    if (static_cast<int>(m_fdRead.fd_array[i]) > number) {
                              number = static_cast<int>(m_fdRead.fd_array[i]);
                    }
          }
          temp = static_cast<SOCKET>(number) + 1;
#else
          temp = _client.m_socket + 1;  //Linux计算方式
#endif
          return ::select(temp, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
}

int EventSelectStruct::isSelectSocketRead()                                               //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#else
          return m_fdRead._FD_ISSET(m_listenServer);
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#else
          return m_fdWrite._FD_ISSET(m_listenServer);
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#else
          return m_fdException._FD_ISSET(m_listenServer);
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //清除Select模型的写入
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(*s);
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //清除Select模型的写入
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(s);
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //清除Select模型的发送
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdWrite);
#else
          m_fdWrite._FD_CLR(s);
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //清除Select模型的异常
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdException);
#else
          m_fdException._FD_CLR(s);
#endif
}

void EventSelectStruct::updateClientConn(const std::vector<Socket*>& vec)
{
          for (size_t i = 0; i < vec.size(); ++i) {
#ifdef _WIN3264                         //WINDOWS计算模式
                    FD_SET(vec.at(i)->m_socket, &m_fdRead);
#else
                    m_fdWrite._FD_SET(*vec.at(i));
#endif
          }
}
void EventSelectStruct::updateClientConn(const Socket& s)
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_SET(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(s);
#endif
}

void EventSelectStruct::updateClientConn(const Socket*& s)
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_SET(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(*s);
#endif
}

size_t  EventSelectStruct::getReadCount()                                                           //读取的Select数据个数
{
#ifdef _WIN3264
          return m_fdRead.fd_count;
#else
          return m_fdRead.getFdSetSize();
#endif // __WIN3264
}

size_t  EventSelectStruct::getWriteCount()                                                            //写入的Select数据个数
{
#ifdef _WIN3264
          return m_fdWrite.fd_count;
#else
          return m_fdWrite.getFdSetSize();
#endif // __WIN3264
}

size_t  EventSelectStruct::getExceptionCount()                                                            //异常的Select数据个数
{
#ifdef _WIN3264
          return m_fdException.fd_count;
#else
          return m_fdException.getFdSetSize();
#endif // __WIN3264
}

std::vector<Socket*>::iterator EventSelectStruct::getReadSocket(std::vector<Socket*>& vec, int pos)
{
          for (auto ib = vec.begin(); ib != vec.end(); ib++) {
#ifdef _WIN3264
                    if ((*ib)->getSocketConnStatus() && (*ib)->getSocket() == this->m_fdRead.fd_array[pos]) {                 //判断连接状态和匹配状态       
                              return ib;
                    }
#else
                    if ((*ib)->getSocketConnStatus() && m_fdRead.getFdStatus(*(*ib))) {                                     //判断连接状态和匹配状态       
                              return ib;
                    }
#endif // __WIN3264
          }
          return vec.end();             //没有找到
}

EventSelectStruct::~EventSelectStruct()
{
          delete m_timeset;
}