#include"ServerSocketFunction.h"

template<typename T>
void MainServer::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

/*优化服务器accept的方式*/
void MainServer::acceptClientCom(const Socket& _listenServer)
{
          Socket _comClient;
          _comClient.m_socket = Socket::createTCPSocket();
          /*在此处加入Select socket网络模型结构*/
          FD_ZERO(&_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&_fdException);                //清空fd_set结构中的数量

          FD_SET(_listenServer.m_socket, &_fdRead);
          FD_SET(_listenServer.m_socket, &_fdWrite);
          FD_SET(_listenServer.m_socket, &_fdException);

          for (int i = m_connClients.size() - 1; i >= 0; --i) {
                    FD_SET(m_connClients.at(i).m_socket, &_fdRead);      //根据连接的客户端重新更新fd_set的的信息
          }

          if (::select(_listenServer.m_socket + 1, &_fdRead, &_fdWrite, &_fdException, NULL) < 0) {    //出现错误
                    std::cout << "SELECT 工作错误!" << std::endl;
          }
          else
          {
                    if (FD_ISSET(_listenServer.m_socket, &_fdRead)) {                //是否已经设置读取描述符
                              FD_CLR(_listenServer.m_socket ,&_fdRead);                 //在_fdRead中进行清除
                              try {
                                        _comClient.m_socketStatus = true;
                                        std::lock_guard<std::mutex>_lck(m_acceptMutex);             //构造析构锁
                                        if ((_comClient.m_socket = accept(_listenServer.m_socket,
                                                  reinterpret_cast<sockaddr*>(&_comClient.m_addrInfo),
                                                  &_comClient.m_socketSizeInfo)) == INVALID_SOCKET)
                                        {
                                                  throw ClientConnectFailed();
                                        }
                                        clientAddrLogger(_comClient);           //进行服务端连接的客户端的地址显示
                                        m_connClients.push_back(_comClient);    //将客户端Socket压入容器
                                        FD_SET(_comClient.m_socket, &_fdRead);  //写入Select
                              }
                              catch (const  ClientConnectFailed&) {
                                        _comClient.m_socketStatus = false;
                    #ifdef _DEBUG
                                        std::cout << "[DEBUG INFO] : CLIENT CONNECT ERROR! \n" << WSAGetLastError() << std::endl;
                    #endif 
                              }
                    }
                    for (size_t i = 0; i < _fdRead.fd_count; ++i) {      //遍历fd_set.fd_array[i]
                              std::vector<Socket>::iterator iter = FindSocket(_fdRead.fd_array[i]);
                              if (iter!=m_connClients.end() && (*iter).m_socket!= INVALID_SOCKET) {
                                        clientService(*iter);                   //执行用户逻辑
                                        m_connClients.erase(iter);    //完成后删除
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
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          ::WSACleanup();
#endif
          //关闭所有的其他客户端
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ++ib) {
                    (*ib).socketClose();
          }
          FD_ZERO(&_fdRead);                        //清空fd_set结构中的数量
          FD_ZERO(&_fdWrite);                       //清空fd_set结构中的数量
          FD_ZERO(&_fdException);                //清空fd_set结构中的数量
}

bool MainServer::initlizeServer()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
}

int  MainServer::setListenQueue(const Socket& _listenserver, int _queueSize) //允许的ACK半连接队列缓冲大小
{
          return ::listen(_listenserver.m_socket, _queueSize);
}

void MainServer::clientAddrLogger(const Socket& _client)               //Client地址记录器
{
          std::lock_guard<std::mutex> lock(m_loggerDisplayMutex);
          std::cout << "[Client Online Alert!] : IP=" << ::inet_ntoa(_client.m_addrInfo.sin_addr)
                    << "Port = " << _client.m_addrInfo.sin_port << std::endl;
}

std::vector<Socket>::iterator MainServer::FindSocket(const SOCKET& s)
{
          for (auto ib = m_connClients.begin(); ib != m_connClients.end(); ib++) {
                    if ((*ib).getSocketConnStatus() && (*ib).m_socket == s) {            //判断连接状态和匹配状态
                              return ib;
                    }
          }
          return m_connClients.end();             //没有找到
}

void MainServer::clientService(Socket& _client)                            //核心业务函数
{
          bool _shutdownflag(false);
          while (1) {
                    char szRecvBuffer[4096]{ 0 };                                                                                   //接受缓冲区
                    char szSendBuffer[4096]{ 0 };                                                                                   //发送缓冲区
                    if ((_retValue = _client.PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0)   //先读取消息头无偏移
                    {
                              /* 出现少包的情况*/
                              if (_retValue < sizeof(DataPacketHeader)) {

                              }
                              else {
                                        DataPacketHeader* header(reinterpret_cast<DataPacketHeader*>(szRecvBuffer));
                                        DataPacketBody* body(reinterpret_cast<DataPacketBody*>(
                                                  reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader)));
                                        DataTransferState* state(reinterpret_cast<DataTransferState*>(szSendBuffer));
                                        _retValue = _client.PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //偏移一个消息头的长度
                                        if (header->getPacketCommand() == CMD_LOGIN) {              //登入状态确定
                                                  std::cout << "收到命令信息 CMD_LOGIN:" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_LOGIN_RESULT);
                                                  
                                        }
                                        else if (header->getPacketCommand() == CMD_LOGOUT) {         //登出状态确定
                                                  std::cout << "收到命令信息 CMD_LOGOUT:" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_LOGOUT_RESULT);
                                                  _shutdownflag = true;                                                              //退出当前socket处理
                                        }
                                        else {                                                                                              //无效指令
                                                  std::cout << "收到命令信息错误" << std::endl;
                                                  cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                                  state = new (szSendBuffer)  DataTransferState(CMD_ERROR);
                                        }
                                        _retValue = _client.PackageSend(szSendBuffer, 0, state->getPacketLength());
                                        cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                                        cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
                                       if (_shutdownflag) {                               //退出程序
                                                  break;
                                      }
                              }
                    }
                    else {
                              /*没有接受到数据，因此判定客户端已经退出*/
                              std::cout << "客户端已经退出程序!，结束服务!" << std::endl;
                              break;
                    }
          }
}