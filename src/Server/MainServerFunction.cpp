#include"ServerSocketFunction.h"

template<typename T>
void MainServer::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

/*�Ż�������accept�ķ�ʽ*/
Socket* MainServer::acceptClientCom(const Socket& _listenServer)
{
          Socket* socketTemp(new Socket);
          try {
                    socketTemp->m_socket = Socket::createTCPSocket();                                        //ͨ�ŵĿͻ���socket�Ĵ��� 
                    socketTemp->m_socketStatus = true;
                    std::lock_guard<std::mutex>_lck(m_acceptMutex);                                        //����������
                    if ((socketTemp->m_socket = accept(_listenServer.m_socket,
                              reinterpret_cast<sockaddr*>(&socketTemp->m_addrInfo),
                              &socketTemp->m_socketSizeInfo)) == INVALID_SOCKET)
                    {
                              throw ClientConnectFailed();
                    }
                    clientConnectLogger(*socketTemp);                                                                     //���з�������ӵĿͻ��˵ĵ�ַ��ʾ
                    socketTemp->m_socketStatus = true;
                    m_connClients.push_back(socketTemp);                                                          //���ͻ���Socketѹ������
          }
          catch (const  ClientConnectFailed&) {
                    socketTemp->m_socketStatus = false;
                    #ifdef  _DEBUG
                    std::cout << "[DEBUG INFO] : CLIENT CONNECT ERROR! \n" << WSAGetLastError() << std::endl;
          #endif 
          }
          return socketTemp;
}

MainServer::MainServer(timeval &t):
          m_timesetting(new timeval(t))
{
          this->_retValue = 0;
#ifdef _WIN3264
          this->_wsadata = { 0 };
#endif // _WIN3264
}

MainServer::~MainServer()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows ƽ̨����
          ::WSACleanup();
#endif
          //�ر����е������ͻ���
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ++ib) {
                    (*ib)->socketClose();
          }
}

bool MainServer::initlizeServer()
{
#ifdef  _WIN3264                                                                         //Windows ƽ̨����
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
}

int  MainServer::setListenQueue(const Socket& _listenserver, int _queueSize) //�����ACK�����Ӷ��л����С
{
          return ::listen(_listenserver.m_socket, _queueSize);
}

void MainServer::clientConnectLogger(const Socket& _client)               //Client��ַ��¼��
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Online Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << " Port = " << _client.m_addrInfo.sin_port << std::endl;
}

void  MainServer::clientDisconnectLogger(const Socket& _client)               //Client�˳���¼��ַ��¼
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Disconnect Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << " Port =  " << _client.m_addrInfo.sin_port << std::endl;
}

std::vector<Socket*>::iterator MainServer::FindSocket(const SOCKET& s)
{
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ib++) {
                    if ((*ib)->getSocketConnStatus() && (*ib)->m_socket == s) {            //�ж�����״̬��ƥ��״̬
                              return ib;
                    }
          }
          return m_connClients.end();             //û���ҵ�
}

void MainServer::eventSelectCom(const Socket& _listenServer)
{
          std::cout << "��ʼ�������ģʽ���ȴ��ͻ��˵�����........" << std::endl;
          while (1) {
                    /*�ڴ˴�����Select socket����ģ�ͽṹ*/
                    EventSelectStruct eventSelect(_listenServer, *m_timesetting);
                    eventSelect.updateClientConn(this->m_connClients);                                                    //��֮ǰ����Ŀͻ��˼������µ��ļ�������fd_set����Ϣ
                    if (eventSelect.StartSelect(const_cast<Socket&>(_listenServer)) < 0) {                                                                                        //���ִ���
                              std::cout << "SELECT ��������!    " << std::endl;
                    }
                    else
                    {
                              if (eventSelect.isSelectSocketRead()) {               //   ˭���ӽ����ͷ���ESTABLISHED
                                        char szSendBuffer[1024]{ 0 };
                                        ClientUpdatePackage* packet = reinterpret_cast<ClientUpdatePackage*>(szSendBuffer);
                                        const Socket* ConnectSocket = this->acceptClientCom(_listenServer);           //���������Ӳ�����
                                        eventSelect.updateClientConn(ConnectSocket);
                                        const_cast<Socket*>(ConnectSocket)->PackageSend(szSendBuffer, 0, packet->getPacketLength());
                                        for (size_t i = 0; i < this->m_connClients.size(); ++i) {                  //�������û����͵�ǰ���ߵ������û�

                                                  char szSendBuffer[1024]{ 0 };
                                                  ClientUpdatePackage* packet = reinterpret_cast<ClientUpdatePackage*>(szSendBuffer);
                                                  packet = new (szSendBuffer)  ClientUpdatePackage(CMD::CMD_NEWMEMBER_JOINED);
                                                  this->m_connClients.at(i)->PackageSend(szSendBuffer, 0, packet->getPacketLength());
                                        }
                              }
                              eventSelect.cleanSelectSocketRead(_listenServer);      //��_fdRead�н������
                              for (size_t i = 0; i < eventSelect.getReadCount(); ++i) {      //����fd_set.fd_array[i]
                                        const std::vector<Socket*>::iterator iter = eventSelect.getReadSocket(this->m_connClients, i);
                                        if (iter != m_connClients.end() && (*iter)->m_socket != INVALID_SOCKET) {
                                                  if (clientService(*iter))                   //�û������ر�logout
                                                  {
                                                            const Socket* socket = (*iter)->getMySelf();
                                                            clientDisconnectLogger(*socket);                                             //�˳�ʱ�ļ�¼����
                                                             eventSelect.cleanSelectSocketRead(socket);
                                                             delete socket;               //�ֶ��رղ��ͷ�socket
                                                            m_connClients.erase(iter);    //��ɺ�ɾ��
                                                            for (size_t i = 0; i < this->m_connClients.size(); ++i) {            //�ڿͻ����Ƴ����ٴθ���
                                                                      char szSendBuffer[1024]{ 0 };
                                                                      ClientUpdatePackage* packet = new (szSendBuffer)  ClientUpdatePackage(CMD::CMD_MEMBER_LEAVED);
                                                                      this->m_connClients.at(i)->PackageSend(szSendBuffer, 0, packet->getPacketLength());
                                                            }
                                                  }
                                        }
                              }
                    }
                    //DO STH ELSE
                    //std::this_thread::sleep_for(std::chrono::seconds(1));
          }
}


bool MainServer::clientService(Socket*& _client)                            //����ҵ����
{
          bool _shutdownflag(false);
          char szRecvBuffer[4096]{ 0 };                                                                                   //���ܻ�����
          char szSendBuffer[4096]{ 0 };                                                                                   //���ͻ�����
          if ((_retValue = _client->PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0)   //�ȶ�ȡ��Ϣͷ��ƫ��
          {
                    /* �����ٰ������*/
                    if (_retValue < sizeof(DataPacketHeader)) {

                    }
                    else {
                              DataPacketHeader* header(reinterpret_cast<DataPacketHeader*>(szRecvBuffer));
                           /*   ConnectControlPackage* body(reinterpret_cast<ConnectControlPackage*>(
                                        reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader)));*/
                              ConnectControlPackage* body(reinterpret_cast<ConnectControlPackage*>(reinterpret_cast<char*>(szRecvBuffer)));
                              DataTransferState* state(reinterpret_cast<DataTransferState*>(szSendBuffer));
                              _retValue = _client->PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //ƫ��һ����Ϣͷ�ĳ���
                              if (header->getPacketCommand() == CMD::CMD_LOGIN) {              //����״̬ȷ��
                                        std::cout << std::endl << "�յ�������Ϣ CMD_LOGIN:" << std::endl;
                                        std::cout << "��¼ID��" << body->getUserName() << std::endl;
                                        std::cout << "��¼Pass��" << body->getUserPassword() << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_LOGIN_RESULT);

                              }
                              else if (header->getPacketCommand() == CMD::CMD_LOGOUT) {         //�ǳ�״̬ȷ��
                                        std::cout << std::endl << "�յ�������Ϣ CMD_LOGOUT:" << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_LOGOUT_RESULT);
                                        _shutdownflag = true;                                                              //�˳���ǰsocket����
                              }
                              else {                                                                                              //��Чָ��
                                        std::cout << "�յ�������Ϣ����" << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_ERROR);
                              }
                              _retValue = _client->PackageSend(szSendBuffer, 0, state->getPacketLength());
                              cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                              cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                    }
          }
          else {
                    /*û�н��ܵ����ݣ�����ж��ͻ����Ѿ��˳�*/
                    std::cout << "�ͻ����Ѿ��˳�����!����������!" << std::endl;
                    _shutdownflag = true;                                                              //�˳���ǰsocket����
          }
          return _shutdownflag;                   //�Ƿ�ر�socket
}