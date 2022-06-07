#include"ServerSocketFunction.h"

#ifndef _WIN3264     //Linux平台专属fd_set结构体
_fd_set::_fd_set() :m_sizeCount(0) {
          _FD_ZERO();
}

_fd_set::~_fd_set() {
          m_sizeCount = 0;
          _FD_ZERO();
}

void _fd_set::_FD_ZERO() {
          FD_ZERO(this->getFdStructPtr());
          m_sizeCount = 0;
}

void _fd_set::_FD_SET(Socket& socket) {
          FD_SET(socket.m_socket, this->getFdStructPtr());
          m_sizeCount++;
}

void _fd_set::_FD_CLR(Socket& socket) {
          FD_CLR(socket.m_socket, this->getFdStructPtr());
          m_sizeCount--;
}

int _fd_set::_FD_ISSET(Socket& socket)
{
          return FD_ISSET(socket.m_socket, this->getFdStructPtr());
}

int _fd_set::getFdSetSize() {
          return m_sizeCount;
}

bool _fd_set::getFdStatus(Socket& socket) {
          return static_cast<bool>(this->getFdArray()[static_cast<size_t>(socket.m_socket) - 1]);
}

fd_mask* _fd_set::getFdArray()
{
          return this->m_fd_set.fds_bits;
}

fd_set* _fd_set::getFdStructPtr()
{
          return &this->m_fd_set;
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
          m_fdRead._FD_SET(const_cast<Socket&>(m_listenServer), m_fdRead.getFdStructPtr());
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
          m_fdRead._FD_SET(const_cast<Socket&>(m_listenServer), m_fdRead.getFdStructPtr());
#endif // _WIN3264
}

int EventSelectStruct::StartSelect(Socket& _client)
{
          SOCKET temp = INVALID_SOCKET;                                      //最大临时Socket FD
          size_t _socketNumber(0), _socketFdArraySize(0);                      //

#ifdef _WIN3264                         //WINDOWS计算模式
          _socketFdArraySize = this->getReadCount();                              //得到读取select种的FD数量
          for (size_t i = 0; i < _socketFdArraySize; ++i) {
                    if (static_cast<int>(m_fdRead.fd_array[i]) > _socketNumber) {
                              _socketNumber = static_cast<int>(m_fdRead.fd_array[i]);
                    }
          }
          temp = static_cast<SOCKET>(_socketNumber) + 1;
#else
          _socketFdArraySize = FD_SETSIZE;                                       //FD数组最大大小(全平台通用)
          for (size_t i = 0; i < _socketFdArraySize; ++i) {
                    if (!static_cast<int>(m_fdRead.getFdArray()[i])) {                         //如果非0则代表当前单元i存在FD句柄i+1
                              _socketNumber = i;
                    }
          }
          temp = static_cast<SOCKET>(_socketNumber) + 2;                                  //FD句柄对应关系本身需要+1，其次保存句柄最大值再次+1
#endif

#ifdef _WIN3264                         //WINDOWS计算模式
          return ::select(temp, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
#else
          return ::select(temp, m_fdRead.getFdStructPtr(), m_fdWrite.getFdStructPtr(), m_fdException.getFdStructPtr(), m_timeset);
#endif
}

int EventSelectStruct::isSelectSocketRead()                                               //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#else
          return m_fdRead._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#else
          return m_fdWrite._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //判断是否设置读取描述符
{
#ifdef _WIN3264                         //WINDOWS计算模式
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#else
          return m_fdException._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //清除Select模型的写入
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(const_cast<Socket&>(*s));
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //清除Select模型的写入
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //清除Select模型的发送
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdWrite);
#else
          m_fdWrite._FD_CLR(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //清除Select模型的异常
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_CLR(s.m_socket, &m_fdException);
#else
          m_fdException._FD_CLR(const_cast<Socket&>(s));
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
          m_fdRead._FD_SET(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::updateClientConn(const Socket*& s)
{
#ifdef _WIN3264                         //WINDOWS计算模式
          FD_SET(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(const_cast<Socket&>(*s));
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