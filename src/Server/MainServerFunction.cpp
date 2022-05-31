#include"ServerSocketFunction.h"

template<typename T>
void MainServer::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

/*�Ż�������accept�ķ�ʽ*/
void MainServer::acceptClientCom(const Socket& _listenServer)
{
          Socket _comClient;
          _comClient.m_socket = Socket::createTCPSocket();
          /*�ڴ˴�����Select socket����ģ�ͽṹ*/
          FD_ZERO(&_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&_fdException);                //���fd_set�ṹ�е�����

          FD_SET(_listenServer.m_socket, &_fdRead);
          FD_SET(_listenServer.m_socket, &_fdWrite);
          FD_SET(_listenServer.m_socket, &_fdException);

          for (int i = m_connClients.size() - 1; i >= 0; --i) {
                    FD_SET(m_connClients.at(i).m_socket, &_fdRead);      //�������ӵĿͻ������¸���fd_set�ĵ���Ϣ
          }

          if (::select(_listenServer.m_socket + 1, &_fdRead, &_fdWrite, &_fdException, NULL) < 0) {    //���ִ���
                    std::cout << "SELECT ��������!" << std::endl;
          }
          else
          {
                    if (FD_ISSET(_listenServer.m_socket, &_fdRead)) {                //�Ƿ��Ѿ����ö�ȡ������
                              FD_CLR(_listenServer.m_socket ,&_fdRead);                 //��_fdRead�н������
                              try {
                                        _comClient.m_socketStatus = true;
                                        std::lock_guard<std::mutex>_lck(m_acceptMutex);             //����������
                                        if ((_comClient.m_socket = accept(_listenServer.m_socket,
                                                  reinterpret_cast<sockaddr*>(&_comClient.m_addrInfo),
                                                  &_comClient.m_socketSizeInfo)) == INVALID_SOCKET)
                                        {
                                                  throw ClientConnectFailed();
                                        }
                                        clientAddrLogger(_comClient);           //���з�������ӵĿͻ��˵ĵ�ַ��ʾ
                                        m_connClients.push_back(_comClient);    //���ͻ���Socketѹ������
                                        FD_SET(_comClient.m_socket, &_fdRead);  //д��Select
                              }
                              catch (const  ClientConnectFailed&) {
                                        _comClient.m_socketStatus = false;
                    #ifdef _DEBUG
                                        std::cout << "[DEBUG INFO] : CLIENT CONNECT ERROR! \n" << WSAGetLastError() << std::endl;
                    #endif 
                              }
                    }
                    for (size_t i = 0; i < _fdRead.fd_count; ++i) {      //����fd_set.fd_array[i]
                              std::vector<Socket>::iterator iter = FindSocket(_fdRead.fd_array[i]);
                              if (iter!=m_connClients.end() && (*iter).m_socket!= INVALID_SOCKET) {
                                        clientService(*iter);                   //ִ���û��߼�
                                        m_connClients.erase(iter);    //��ɺ�ɾ��
                              }
                    }
          }
}

MainServer::MainServer()
{
          this->_retValue = 0;
          this->_wsadata = { 0 };
}

MainServer::~MainServer()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows ƽ̨����
          ::WSACleanup();
#endif
          //�ر����е������ͻ���
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ++ib) {
                    (*ib).socketClose();
          }
          FD_ZERO(&_fdRead);                        //���fd_set�ṹ�е�����
          FD_ZERO(&_fdWrite);                       //���fd_set�ṹ�е�����
          FD_ZERO(&_fdException);                //���fd_set�ṹ�е�����
}

bool MainServer::initlizeServer()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows ƽ̨����
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
}

int  MainServer::setListenQueue(const Socket& _listenserver, int _queueSize) //�����ACK�����Ӷ��л����С
{
          return ::listen(_listenserver.m_socket, _queueSize);
}

void MainServer::clientAddrLogger(const Socket& _client)               //Client��ַ��¼��
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Online Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << "Port = " << _client.m_addrInfo.sin_port << std::endl;
}

std::vector<Socket>::iterator MainServer::FindSocket(const SOCKET& s)
{
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ib++) {
                    if ((*ib).getSocketConnStatus() && (*ib).m_socket == s) {            //�ж�����״̬��ƥ��״̬
                              return ib;
                    }
          }
          return m_connClients.end();             //û���ҵ�
}

void MainServer::clientService(Socket& _client)                            //����ҵ����
{
          bool _shutdownflag(false);
          while (1) {
                    char szRecvBuffer[4096]{ 0 };                                                                                   //���ܻ�����
                    char szSendBuffer[4096]{ 0 };                                                                                   //���ͻ�����
                    if ((_retValue = _client.PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0)   //�ȶ�ȡ��Ϣͷ��ƫ��
                    {
                              /* �����ٰ������*/
                              if (_retValue < sizeof(DataPacketHeader)) {

                              }
                              else {
                                        DataPacketHeader* header(reinterpret_cast<DataPacketHeader*>(szRecvBuffer));
                                        DataPacketBody* body(reinterpret_cast<DataPacketBody*>(
                                                  reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader)));
                                        DataTransferState* state(reinterpret_cast<DataTransferState*>(szSendBuffer));
                                        _retValue = _client.PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //ƫ��һ����Ϣͷ�ĳ���
                                        if (header->getPacketCommand() == CMD_LOGIN) {              //����״̬ȷ��
                                                  std::cout << "�յ�������Ϣ CMD_LOGIN:" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_LOGIN_RESULT);
                                                  
                                        }
                                        else if (header->getPacketCommand() == CMD_LOGOUT) {         //�ǳ�״̬ȷ��
                                                  std::cout << "�յ�������Ϣ CMD_LOGOUT:" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_LOGOUT_RESULT);
                                                  _shutdownflag = true;                                                              //�˳���ǰsocket����
                                        }
                                        else {                                                                                              //��Чָ��
                                                  std::cout << "�յ�������Ϣ����" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_ERROR);
                                        }
                                        _retValue = _client.PackageSend(szSendBuffer, 0, state->getPacketLength());
                                        cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                       if (_shutdownflag) {                               //�˳�����
                                                  break;
                                      }
                              }
                    }
                    else {
                              /*û�н��ܵ����ݣ�����ж��ͻ����Ѿ��˳�*/
                              std::cout << "�ͻ����Ѿ��˳�����!����������!" << std::endl;
                              break;
                    }
          }
}