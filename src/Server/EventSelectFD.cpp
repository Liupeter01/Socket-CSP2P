#include"ServerSocketFunction.h"

EventSelectStruct::EventSelectStruct(const Socket& _socket) 
          :m_listenServer(_socket),
          m_timeset(NULL)
{
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
          FD_SET(m_listenServer.m_socket, &m_fdRead);
}

EventSelectStruct::EventSelectStruct(const Socket& _socket, timeval& _timeval)
          :m_listenServer(_socket),
          m_timeset(new timeval (_timeval))    
{
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
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

int EventSelectStruct::isSelectSocketRead()                                               //�ж��Ƿ����ö�ȡ������
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
}

int EventSelectStruct::isSelectSocketWrite()                                             //�ж��Ƿ����ö�ȡ������
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
}

int EventSelectStruct::isSelectSocketException()                                                //�ж��Ƿ����ö�ȡ������
{
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket*& s)                      //���Selectģ�͵�д��
{
          FD_CLR(s->m_socket, &m_fdRead);
}
void  EventSelectStruct::cleanSelectSocketRead(const Socket& s)                      //���Selectģ�͵�д��
{
          FD_CLR(s.m_socket, &m_fdRead);
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //���Selectģ�͵ķ���
{
          FD_CLR(s.m_socket, &m_fdWrite);
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //���Selectģ�͵��쳣
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

size_t  EventSelectStruct::getReadCount()                                                           //��ȡ��Select���ݸ���
{
#ifdef _WIN3264
          return m_fdRead.fd_count;
#else
          int _readCount(0);
          for (auto i : m_fdRead.fds_bits) {
                    if (!(*i)) {                  //��ǰλ��Ϊ0
                              _readCount++;
                    }
          }
          return _readCount;
#endif // __WIN3264
}

size_t  EventSelectStruct::getWriteCount()                                                            //д���Select���ݸ���
{
#ifdef _WIN3264
          return m_fdWrite.fd_count;
#else
          int _writeCount(0);
          for (auto i : m_fdWrite.fds_bits) {
                    if (!(*i)) {                  //��ǰλ��Ϊ0
                              _writeCount++;
                    }
          }
          return _writeCount;
#endif // __WIN3264
}

size_t  EventSelectStruct::getExceptionCount()                                                            //�쳣��Select���ݸ���
{
#ifdef _WIN3264
          return m_fdException.fd_count;
#else
          int _exceptionCount(0);
          for (auto i : m_fdException.fds_bits) {
                    if (!(*i)) {                  //��ǰλ��Ϊ0
                              _exceptionCount++;
                    }
          }
          return _exceptionCount;
#endif // __WIN3264
}

std::vector<Socket*>::iterator EventSelectStruct::getReadSocket( std::vector<Socket*>& vec, int pos)
{
          for (auto ib = vec.begin(); ib != vec.end(); ib++) {
                    
                    if ((*ib)->getSocketConnStatus() && (*ib)->getSocket() == this->m_fdRead.fd_array[pos]) {                 //�ж�����״̬��ƥ��״̬       
                              return ib;
                    }
          }
          return vec.end();             //û���ҵ�
}

EventSelectStruct::~EventSelectStruct() 
{
          delete m_timeset;
          //FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          //FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          //FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
}