#include"ServerSocketFunction.h"

#ifndef _WIN3264     //Linuxƽ̨ר��fd_set�ṹ��
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

#endif // _WIN3264     //Linuxƽ̨ר��fd_set�ṹ��

EventSelectStruct::EventSelectStruct(const Socket& _socket)
          :m_listenServer(_socket),
          m_timeset(NULL)
{
#ifdef _WIN3264
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
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
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
          FD_SET(m_listenServer.m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(const_cast<Socket&>(m_listenServer), m_fdRead.getFdStructPtr());
#endif // _WIN3264
}

int EventSelectStruct::StartSelect(Socket& _client)
{
          SOCKET temp = INVALID_SOCKET;                                      //�����ʱSocket FD
          size_t _socketNumber(0), _socketFdArraySize(0);                      //

#ifdef _WIN3264                         //WINDOWS����ģʽ
          _socketFdArraySize = this->getReadCount();                              //�õ���ȡselect�ֵ�FD����
          for (size_t i = 0; i < _socketFdArraySize; ++i) {
                    if (static_cast<int>(m_fdRead.fd_array[i]) > _socketNumber) {
                              _socketNumber = static_cast<int>(m_fdRead.fd_array[i]);
                    }
          }
          temp = static_cast<SOCKET>(_socketNumber) + 1;
#else
          _socketFdArraySize = FD_SETSIZE;                                       //FD��������С(ȫƽ̨ͨ��)
          for (size_t i = 0; i < _socketFdArraySize; ++i) {
                    if (!static_cast<int>(m_fdRead.getFdArray()[i])) {                         //�����0�����ǰ��Ԫi����FD���i+1
                              _socketNumber = i;
                    }
          }
          temp = static_cast<SOCKET>(_socketNumber) + 2;                                  //FD�����Ӧ��ϵ������Ҫ+1����α��������ֵ�ٴ�+1
#endif

#ifdef _WIN3264                         //WINDOWS����ģʽ
          return ::select(temp, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
#else
          return ::select(temp, m_fdRead.getFdStructPtr(), m_fdWrite.getFdStructPtr(), m_fdException.getFdStructPtr(), m_timeset);
#endif
}

int EventSelectStruct::isSelectSocketRead()                                               //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#else
          return m_fdRead._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#else
          return m_fdWrite._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#else
          return m_fdException._FD_ISSET(const_cast<Socket&>(m_listenServer));
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //���Selectģ�͵�д��
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(const_cast<Socket&>(*s));
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //���Selectģ�͵�д��
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //���Selectģ�͵ķ���
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdWrite);
#else
          m_fdWrite._FD_CLR(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //���Selectģ�͵��쳣
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdException);
#else
          m_fdException._FD_CLR(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::updateClientConn(const std::vector<Socket*>& vec)
{
          for (size_t i = 0; i < vec.size(); ++i) {
#ifdef _WIN3264                         //WINDOWS����ģʽ
                    FD_SET(vec.at(i)->m_socket, &m_fdRead);
#else
                    m_fdWrite._FD_SET(*vec.at(i));
#endif
          }
}
void EventSelectStruct::updateClientConn(const Socket& s)
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_SET(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(const_cast<Socket&>(s));
#endif
}

void EventSelectStruct::updateClientConn(const Socket*& s)
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_SET(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(const_cast<Socket&>(*s));
#endif
}

size_t  EventSelectStruct::getReadCount()                                                           //��ȡ��Select���ݸ���
{
#ifdef _WIN3264
          return m_fdRead.fd_count;
#else
          return m_fdRead.getFdSetSize();
#endif // __WIN3264
}

size_t  EventSelectStruct::getWriteCount()                                                            //д���Select���ݸ���
{
#ifdef _WIN3264
          return m_fdWrite.fd_count;
#else
          return m_fdWrite.getFdSetSize();
#endif // __WIN3264
}

size_t  EventSelectStruct::getExceptionCount()                                                            //�쳣��Select���ݸ���
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
                    if ((*ib)->getSocketConnStatus() && (*ib)->getSocket() == this->m_fdRead.fd_array[pos]) {                 //�ж�����״̬��ƥ��״̬       
                              return ib;
                    }
#else
                    if ((*ib)->getSocketConnStatus() && m_fdRead.getFdStatus(*(*ib))) {                                     //�ж�����״̬��ƥ��״̬       
                              return ib;
                    }
#endif // __WIN3264
          }
          return vec.end();             //û���ҵ�
}

EventSelectStruct::~EventSelectStruct()
{
          delete m_timeset;
}