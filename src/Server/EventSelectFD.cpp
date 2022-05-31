#include"ServerSocketFunction.h"

EventSelectStruct::EventSelectStruct(const Socket& listenserver, timeval& _timeval)
          :m_listenServer(listenserver),
          m_timeset(new timeval (_timeval))    
{
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
          FD_SET(listenserver.m_socket, &this->m_fdRead);
}

int EventSelectStruct::StartSelect()
{
#ifdef WINDOWSPLATFROM
          return ::select(m_listenServer.m_socket + 1, &m_fdRead, &m_fdWrite, &m_fdException, m_timeset);
#endif
}

int EventSelectStruct::isSelectSocketRead()                                               //�ж��Ƿ����ö�ȡ������
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdRead);
#endif
}

int EventSelectStruct::isSelectSocketWrite()                                             //�ж��Ƿ����ö�ȡ������
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdWrite);
#endif
}

int EventSelectStruct::isSelectSocketException()                                                //�ж��Ƿ����ö�ȡ������
{
#ifdef WINDOWSPLATFROM
          return  ::FD_ISSET(m_listenServer.m_socket, &m_fdException);
#endif
}

void  EventSelectStruct::cleanSelectSocketRead(const Socket &s)                      //���Selectģ�͵�д��
{
#ifdef WINDOWSPLATFROM  
          FD_CLR(s.m_socket, &m_fdRead);
#endif
}

void EventSelectStruct::cleanSelectSocketWrite(const Socket& s)                            //���Selectģ�͵ķ���
{
#ifdef WINDOWSPLATFROM
          FD_CLR(s.m_socket, &m_fdWrite);
#endif
}

void EventSelectStruct::cleanSelectSocketException(const Socket& s)              //���Selectģ�͵��쳣
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

size_t  EventSelectStruct::getReadCount()                                                           //��ȡ��Select���ݸ���
{
          return m_fdRead.fd_count;
}

size_t  EventSelectStruct::getWriteCount()                                                            //д���Select���ݸ���
{
          return m_fdWrite.fd_count;
}

size_t  EventSelectStruct::getExceptionCount()                                                            //�쳣��Select���ݸ���
{
          return m_fdException.fd_count;
}

std::vector<Socket>::iterator EventSelectStruct::getReadSocket( std::vector<Socket>& vec, int pos)
{
          for (auto ib = vec.begin(); ib != vec.end(); ib++) {
                    
                    if ((*ib).getSocketConnStatus() && (*ib).getSocket() == this->m_fdRead.fd_array[pos]) {                 //�ж�����״̬��ƥ��״̬       
                              return ib;
                    }
          }
          return vec.end();             //û���ҵ�
}

EventSelectStruct::~EventSelectStruct() 
{
          delete m_timeset;
          FD_ZERO(&m_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&m_fdException);                //���fd_set�ṹ�е�����
}