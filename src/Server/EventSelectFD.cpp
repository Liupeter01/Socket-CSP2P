#include"ServerSocketFunction.h"

EventSelectStruct::EventSelectStruct(const Socket& _socket) 
          :m_listenServer(_socket),
          m_timeset(NULL)
{
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
          FD_SET(m_listenServer.m_socket, &m_fdRead);
}

EventSelectStruct::EventSelectStruct(const Socket& _socket, timeval& _timeval)
          :m_listenServer(_socket),
          m_timeset(new timeval (_timeval))    
{
          FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
          FD_SET(m_listenServer.m_socket, &m_fdRead);
}

int EventSelectStruct::StartSelect(Socket& _client)
{
          SOCKET temp;
#ifdef _WIN3264
          int number = 0;
          for (size_t i = 0; i < getReadCount(); ++i) {
                    if (static_cast<int>(m_fdRead.fd_array[i]) > number) {
                              number = static_cast<int>(m_fdRead.fd_array[i]);
                    }
          }
          temp = static_cast<SOCKET>(number)+1;
#else
          temp = _client.m_socket + 1;
#endif
          return ::select(temp, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
}

int EventSelectStruct::isSelectSocketRead()                                               //判断是否设置读取描述符
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
}

int EventSelectStruct::isSelectSocketWrite()                                             //判断是否设置读取描述符
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
}

int EventSelectStruct::isSelectSocketException()                                                //判断是否设置读取描述符
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //清除Select模型的写入
{
          FD_CLR(s->m_socket, &m_fdRead);
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //清除Select模型的写入
{
          FD_CLR(s.m_socket, &m_fdRead);
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //清除Select模型的发送
{
          FD_CLR(s.m_socket, &m_fdWrite);
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //清除Select模型的异常
{
          FD_CLR(s.m_socket, &m_fdException);
}


void EventSelectStruct::updateClientConn(const std::vector<Socket*>& vec)
{
          for (size_t i = 0; i < vec.size(); ++i) {
                    FD_SET(vec.at(i)->m_socket, &m_fdRead);
          }
}
void EventSelectStruct::updateClientConn(const Socket& s)
{
          FD_SET(s.m_socket, &m_fdRead);
}

void EventSelectStruct::updateClientConn(const Socket*& s)
{
          FD_SET(s->m_socket, &m_fdRead);
}

size_t  EventSelectStruct::getReadCount()                                                           //读取的Select数据个数
{
#ifdef _WIN3264
          return m_fdRead.fd_count;
#else
          int _readCount(0);
          for (auto i : m_fdRead.fds_bits) {
                    if (!(*i)) {                  //当前位不为0
                              _readCount++;
                    }
          }
          return _readCount;
#endif // __WIN3264
}

size_t  EventSelectStruct::getWriteCount()                                                            //写入的Select数据个数
{
#ifdef _WIN3264
          return m_fdWrite.fd_count;
#else
          int _writeCount(0);
          for (auto i : m_fdWrite.fds_bits) {
                    if (!(*i)) {                  //当前位不为0
                              _writeCount++;
                    }
          }
          return _writeCount;
#endif // __WIN3264
}

size_t  EventSelectStruct::getExceptionCount()                                                            //异常的Select数据个数
{
#ifdef _WIN3264
          return m_fdException.fd_count;
#else
          int _exceptionCount(0);
          for (auto i : m_fdException.fds_bits) {
                    if (!(*i)) {                  //当前位不为0
                              _exceptionCount++;
                    }
          }
          return _exceptionCount;
#endif // __WIN3264
}

std::vector<Socket*>::iterator EventSelectStruct::getReadSocket( std::vector<Socket*>& vec, int pos)
{
          for (auto ib = vec.begin(); ib != vec.end(); ib++) {
                    
                    if ((*ib)->getSocketConnStatus() && (*ib)->getSocket() == this->m_fdRead.fd_array[pos]) {                 //判断连接状态和匹配状态       
                              return ib;
                    }
          }
          return vec.end();             //没有找到
}

EventSelectStruct::~EventSelectStruct() 
{
          delete m_timeset;
          //FD_ZERO(&m_fdRead);                        //清空fd_set结构中的数量
          //FD_ZERO(&m_fdWrite);                       //清空fd_set结构中的数量
          //FD_ZERO(&m_fdException);                //清空fd_set结构中的数量
}