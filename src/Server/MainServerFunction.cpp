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
          try {
                    m_connSocket.m_socket = Socket::createTCPSocket();                                        //ͨ�ŵĿͻ���socket�Ĵ��� 
                    m_connSocket.m_socketStatus = true;
                    std::lock_guard<std::mutex>_lck(m_acceptMutex);                                        //����������
                    if ((m_connSocket.m_socket = accept(_listenServer.m_socket,
                              reinterpret_cast<sockaddr*>(&m_connSocket.m_addrInfo),
                              &m_connSocket.m_socketSizeInfo)) == INVALID_SOCKET)
                    {
                              throw ClientConnectFailed();
                    }
                    clientAddrLogger(m_connSocket);                                                                     //���з�������ӵĿͻ��˵ĵ�ַ��ʾ
                    m_connClients.push_back(m_connSocket);                                                          //���ͻ���Socketѹ������
                   
          }
          catch (const  ClientConnectFailed&) {
                    m_connSocket.m_socketStatus = false;
                    #ifdef  _DEBUG
                    std::cout << "[DEBUG INFO] : CLIENT CONNECT ERROR! \n" << WSAGetLastError() << std::endl;
          #endif 
          }
         
}

void MainServer::eventSelectCom(const Socket& _listenServer)
{
          std::cout << "��ʼ�������ģʽ���ȴ��ͻ��˵�����........" << std::endl;
          while (1) {
                    /*�ڴ˴�����Select socket����ģ�ͽṹ*/
                    EventSelectStruct eventSelect(_listenServer, *m_timesetting);
                    eventSelect.updateClientConn(this->m_connClients);                                                    //����ͻ���û���˳���������µ��ļ�������fd_set����Ϣ
                    if (eventSelect.StartSelect() < 0) {                                                                                        //���ִ���
                              std::cout << "SELECT ��������!    " << WSAGetLastError() << std::endl;
                              continue;
                    }
                    else
                    {
                              if (!eventSelect.isSelectSocketRead()) {               //�Ƿ��ȡ�������仯�������Ѿ�����������
                                        acceptClientCom(_listenServer);
                                        eventSelect.updateClientConn(m_connSocket);                            //���¸�����Ŀͻ��˵���Ϣ
                              }
                              eventSelect.cleanSelectSocketRead(_listenServer);      //��_fdRead�н������
                              for (size_t i = 0; i < eventSelect.getReadCount(); ++i) {      //����fd_set.fd_array[i]
                                        auto iter = eventSelect.getReadSocket(this->m_connClients, i);
                                        if (iter != m_connClients.end() && (*iter).m_socket != INVALID_SOCKET) {
                                                  if (clientService(*iter))                   //�û������ر�logout
                                                  {
                                                            eventSelect.cleanSelectSocketRead((*iter));
                                                            m_connClients.erase(iter);    //��ɺ�ɾ��
                                                  }
                                        }
                              }
                    }
                    //DO STH ELSE
          }
}


MainServer::MainServer(timeval &t):
          m_timesetting(new timeval(t))
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
                    << " Port = " << _client.m_addrInfo.sin_port << std::endl;
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

bool MainServer::clientService(Socket& _client)                            //����ҵ����
{
          bool _shutdownflag(false);
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
                    }
          }
          else {
                    /*û�н��ܵ����ݣ�����ж��ͻ����Ѿ��˳�*/
                    std::cout << "�ͻ����Ѿ��˳�����!����������!" << std::endl;
                    _shutdownflag = true;                                                              //�˳���ǰsocket����
          }
          return _shutdownflag;                   //�Ƿ�ر�socket
}