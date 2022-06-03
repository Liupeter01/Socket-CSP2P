#include"ServerSocketFunction.h"

template<typename T>
void MainServer::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

/*优化服务器accept的方式*/
Socket* MainServer::acceptClientCom(const Socket& _listenServer)
{
          Socket* socketTemp(new Socket);
          try {
                    socketTemp->m_socket = Socket::createTCPSocket();                                        //通信的客户端socket的创建 
                    socketTemp->m_socketStatus = true;
                    std::lock_guard<std::mutex>_lck(m_acceptMutex);                                        //构造析构锁
                    if ((socketTemp->m_socket = accept(_listenServer.m_socket,
                              reinterpret_cast<sockaddr*>(&socketTemp->m_addrInfo),
                              &socketTemp->m_socketSizeInfo)) == INVALID_SOCKET)
                    {
                              throw ClientConnectFailed();
                    }
                    clientConnectLogger(*socketTemp);                                                                     //进行服务端连接的客户端的地址显示
                    socketTemp->m_socketStatus = true;
                    m_connClients.push_back(socketTemp);                                                          //将客户端Socket压入容器
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
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          ::WSACleanup();
#endif
          //关闭所有的其他客户端
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ++ib) {
                    (*ib)->socketClose();
          }
}

bool MainServer::initlizeServer()
{
#ifdef  _WIN3264                                                                         //Windows 平台适配
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
}

int  MainServer::setListenQueue(const Socket& _listenserver, int _queueSize) //允许的ACK半连接队列缓冲大小
{
          return ::listen(_listenserver.m_socket, _queueSize);
}

void MainServer::clientConnectLogger(const Socket& _client)               //Client地址记录器
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Online Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << " Port = " << _client.m_addrInfo.sin_port << std::endl;
}

void  MainServer::clientDisconnectLogger(const Socket& _client)               //Client退出登录地址记录
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Disconnect Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << " Port =  " << _client.m_addrInfo.sin_port << std::endl;
}

std::vector<Socket*>::iterator MainServer::FindSocket(const SOCKET& s)
{
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ib++) {
                    if ((*ib)->getSocketConnStatus() && (*ib)->m_socket == s) {            //判断连接状态和匹配状态
                              return ib;
                    }
          }
          return m_connClients.end();             //没有找到
}

void MainServer::eventSelectCom(const Socket& _listenServer)
{
          std::cout << "开始进入监听模式，等待客户端的连接........" << std::endl;
          while (1) {
                    /*在此处加入Select socket网络模型结构*/
                    EventSelectStruct eventSelect(_listenServer, *m_timesetting);
                    eventSelect.updateClientConn(this->m_connClients);                                                    //将之前保存的客户端继续更新到文件描述符fd_set的信息
                    if (eventSelect.StartSelect(const_cast<Socket&>(_listenServer)) < 0) {                                                                                        //出现错误
                              std::cout << "SELECT 工作错误!    " << std::endl;
                    }
                    else
                    {
                              if (eventSelect.isSelectSocketRead()) {               //   谁链接进来就发送ESTABLISHED
                                        char szSendBuffer[1024]{ 0 };
                                        ClientUpdatePackage* packet = reinterpret_cast<ClientUpdatePackage*>(szSendBuffer);
                                        const Socket* ConnectSocket = this->acceptClientCom(_listenServer);           //接收新连接并更新
                                        eventSelect.updateClientConn(ConnectSocket);
                                        const_cast<Socket*>(ConnectSocket)->PackageSend(szSendBuffer, 0, packet->getPacketLength());
                                        for (size_t i = 0; i < this->m_connClients.size(); ++i) {                  //给所有用户发送当前在线的所有用户

                                                  char szSendBuffer[1024]{ 0 };
                                                  ClientUpdatePackage* packet = reinterpret_cast<ClientUpdatePackage*>(szSendBuffer);
                                                  packet = new (szSendBuffer)  ClientUpdatePackage(CMD::CMD_NEWMEMBER_JOINED);
                                                  this->m_connClients.at(i)->PackageSend(szSendBuffer, 0, packet->getPacketLength());
                                        }
                              }
                              eventSelect.cleanSelectSocketRead(_listenServer);      //在_fdRead中进行清除
                              for (size_t i = 0; i < eventSelect.getReadCount(); ++i) {      //遍历fd_set.fd_array[i]
                                        const std::vector<Socket*>::iterator iter = eventSelect.getReadSocket(this->m_connClients, i);
                                        if (iter != m_connClients.end() && (*iter)->m_socket != INVALID_SOCKET) {
                                                  if (clientService(*iter))                   //用户主动关闭logout
                                                  {
                                                            const Socket* socket = (*iter)->getMySelf();
                                                            clientDisconnectLogger(*socket);                                             //退出时的记录工具
                                                             eventSelect.cleanSelectSocketRead(socket);
                                                             delete socket;               //手动关闭并释放socket
                                                            m_connClients.erase(iter);    //完成后删除
                                                            for (size_t i = 0; i < this->m_connClients.size(); ++i) {            //在客户端推出后再次更新
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


bool MainServer::clientService(Socket*& _client)                            //核心业务函数
{
          bool _shutdownflag(false);
          char szRecvBuffer[4096]{ 0 };                                                                                   //接受缓冲区
          char szSendBuffer[4096]{ 0 };                                                                                   //发送缓冲区
          if ((_retValue = _client->PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0)   //先读取消息头无偏移
          {
                    /* 出现少包的情况*/
                    if (_retValue < sizeof(DataPacketHeader)) {

                    }
                    else {
                              DataPacketHeader* header(reinterpret_cast<DataPacketHeader*>(szRecvBuffer));
                           /*   ConnectControlPackage* body(reinterpret_cast<ConnectControlPackage*>(
                                        reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader)));*/
                              ConnectControlPackage* body(reinterpret_cast<ConnectControlPackage*>(reinterpret_cast<char*>(szRecvBuffer)));
                              DataTransferState* state(reinterpret_cast<DataTransferState*>(szSendBuffer));
                              _retValue = _client->PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //偏移一个消息头的长度
                              if (header->getPacketCommand() == CMD::CMD_LOGIN) {              //登入状态确定
                                        std::cout << std::endl << "收到命令信息 CMD_LOGIN:" << std::endl;
                                        std::cout << "登录ID：" << body->getUserName() << std::endl;
                                        std::cout << "登录Pass：" << body->getUserPassword() << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_LOGIN_RESULT);

                              }
                              else if (header->getPacketCommand() == CMD::CMD_LOGOUT) {         //登出状态确定
                                        std::cout << std::endl << "收到命令信息 CMD_LOGOUT:" << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_LOGOUT_RESULT);
                                        _shutdownflag = true;                                                              //退出当前socket处理
                              }
                              else {                                                                                              //无效指令
                                        std::cout << "收到命令信息错误" << std::endl;
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                        state = new (szSendBuffer)  DataTransferState(CMD::CMD_ERROR);
                              }
                              _retValue = _client->PackageSend(szSendBuffer, 0, state->getPacketLength());
                              cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                              cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                    }
          }
          else {
                    /*没有接受到数据，因此判定客户端已经退出*/
                    std::cout << "客户端已经退出程序!，结束服务!" << std::endl;
                    _shutdownflag = true;                                                              //退出当前socket处理
          }
          return _shutdownflag;                   //是否关闭socket
}