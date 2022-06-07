#include"ClientSocketFunction.h"

#ifndef _WIN3264     //Linuxƽ̨ר��fd_set�ṹ��
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
          m_fdRead._FD_SET(m_listenServer.m_socket);
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
          m_fdRead._FD_SET(m_listenServer.m_socket);
#endif // _WIN3264
}

int EventSelectStruct::StartSelect(Socket& _client)
{
          SOCKET temp;
#ifdef _WIN3264                         //WINDOWS����ģʽ
          int number = 0;
          for (size_t i = 0; i < getReadCount(); ++i) {
                    if (static_cast<int>(m_fdRead.fd_array[i]) > number) {
                              number = static_cast<int>(m_fdRead.fd_array[i]);
                    }
          }
          temp = static_cast<SOCKET>(number) + 1;
#else
          temp = _client.m_socket + 1;  //Linux���㷽ʽ
#endif
          return ::select(temp, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
}

int EventSelectStruct::isSelectSocketRead()                                               //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#else
          return m_fdRead._FD_ISSET(m_listenServer);
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#else
          return m_fdWrite._FD_ISSET(m_listenServer);
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //�ж��Ƿ����ö�ȡ������
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#else
          return m_fdException._FD_ISSET(m_listenServer);
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //���Selectģ�͵�д��
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(*s);
#endif
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //���Selectģ�͵�д��
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdRead);
#else
          m_fdRead._FD_CLR(s);
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //���Selectģ�͵ķ���
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdWrite);
#else
          m_fdWrite._FD_CLR(s);
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //���Selectģ�͵��쳣
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_CLR(s.m_socket, &m_fdException);
#else
          m_fdException._FD_CLR(s);
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
          m_fdRead._FD_SET(s);
#endif
}

void EventSelectStruct::updateClientConn(const Socket*& s)
{
#ifdef _WIN3264                         //WINDOWS����ģʽ
          FD_SET(s->m_socket, &m_fdRead);
#else
          m_fdRead._FD_SET(*s);
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