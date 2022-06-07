#include"ClientSocketFunction.h"

template<typename T>
void MainClient::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

MainClient::MainClient(timeval& t):
          m_timesetting(new timeval(t))
{
          this->_retValue = 0;
#ifdef  _WIN3264       
          this->_wsadata = { 0 };
#endif
}

MainClient::~MainClient()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          ::WSACleanup();
#endif
          for (auto ib = m_thread.begin(); ib != m_thread.end(); ++ib) {                    //线程的等待
                    if ((*ib).joinable()) {
                              (*ib).join();
                    }
                    m_thread.erase(ib);
          }
}

bool MainClient::initlizeClient()
{
#ifdef  _WIN3264                                                                            //Windows 平台适配
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
          return true;
}

void MainClient::ClientServiceStart(Socket &_client)
{
          m_thread.emplace_back(&MainClient::sventSelectCom, this, std::ref(_client));
          m_thread.emplace_back(&MainClient::UserInput, this, std::ref(_client));
}

void MainClient::sventSelectCom(Socket& _client)                //客户端与服务器是一对一的连接
{
          while (true)
          {
                    EventSelectStruct eventSelect(_client,*m_timesetting);
                    if (eventSelect.StartSelect(_client) < 0) {              //客户端检测是否收到服务器的新消息
                              std::lock_guard<std::mutex> lock(m_DisplayMutex);
                              std::cout << "SELECT 工作错误!    ";
#ifdef _WIN3264
                              std::cout << WSAGetLastError() << std::endl;
#endif
                              break;
                    }
                    else
                    {
                              if (eventSelect.isSelectSocketRead()) { //描述符是否变化处理来自服务器或者其他客户端的消息
                                        /*此时建立连接的是其他连接的P2P客户端*/
                                        //for(auto ib; ; ;)
                                           /*来自服务器是必须要进行处理的*/
                                        eventSelect.cleanSelectSocketRead(_client);                 //清除处理服务器专属连接socket
                                        /*是否存在其他连入该客户端的其他通信客户端*/
                                        //for(auto ib; ; ){}
                                        if (UserService(_client)) {

                                                  std::lock_guard<std::mutex> lock(m_DisplayMutex);
                                                  std::cout << "socket 关闭" << std::endl;
                                                  break;
                                        }
                              }
                    }
          }
}


void MainClient::UserInput(Socket& _client)                           //用户输入线程
{
          while (1)
          {
                    char szSendBuffer[4096]{ 0 };                                                                                   //发送缓冲区
                    ConnectControlPackage* body = reinterpret_cast<ConnectControlPackage*>(reinterpret_cast<char*>(szSendBuffer));
                    /*此处需要使用条件变量*/
                    std::string inputData;
                    std::cout << "输入您的操作(login,logout):";
                    std::cin >> inputData;
                    if (inputData.length() != 0 && !strcmp(inputData.c_str(), "login")) {
                              body = new (szSendBuffer)  ConnectControlPackage(CMD::CMD_LOGIN, "ADMIN", "ADMIN");
                    }
                    else if (inputData.length() != 0 && !strcmp(inputData.c_str(), "logout")) {
                              body = new (szSendBuffer)  ConnectControlPackage(CMD::CMD_LOGOUT);
                    }
                    else {
                              body = new (szSendBuffer) ConnectControlPackage(CMD::DEFAULT);
                    }
                    _client.PackageSend(szSendBuffer, 0, body->getPacketLength());                                 //发送数据报文返回状态
          }
}

bool MainClient::UserService(Socket& _client)                            //核心业务函数
{
          bool _shutdownflag(false);
          char szRecvBuffer[4096]{ 0 };                                                                                   //接受缓冲区
          char szSendBuffer[4096]{ 0 };                                                                                   //发送缓冲区

          /*先运行对于服务器的传输数据的检测程序*/
          DataPacketHeader* header(reinterpret_cast<DataPacketHeader*> (szRecvBuffer));
          ClientUpdatePackage* update = reinterpret_cast<ClientUpdatePackage*>(
                    reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader));

          DataTransferState* state = reinterpret_cast<DataTransferState*>(
                    reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader));

          if ((_retValue = _client.PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0) {   //接收header
                    if (_retValue < sizeof(DataPacketHeader)) {                        /* 出现少包的情况*/

                    }
                    else
                    {
                              _retValue = _client.PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //偏移一个消息头的长度
                              if (header->getPacketCommand() == CMD::CMD_LOGIN_RESULT)                             //用户登录服务器成功
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "用户登陆服务器成功" << std::endl;
                                        m_DisplayMutex.unlock();
                              }
                              else if (header->getPacketCommand() == CMD::CMD_LOGOUT_RESULT)                  //用户登出服务器成功
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "用户成功登出服务器" << std::endl;
                                        m_DisplayMutex.unlock();
                                        return true;
                              }
                              else if (header->getPacketCommand() == CMD::CMD_NEWMEMBER_JOINED)          //新用户加入更新client列表
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_NEWMEMBER_JOINED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*更新连接的P2P客户端的列表*/
                              }
                              else if (header->getPacketCommand() == CMD::CMD_MEMBER_LEAVED)               //已连接的用户离开
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_MEMBER_LEAVED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*更新连接的P2P客户端的列表*/
                              }
                              else if (header->getPacketCommand() == CMD::CMD_ESTABLISHED)               //已连接的用户离开
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_ESTABLISHED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*更新连接的P2P客户端的列表*/
                              }
                              else {
                                        m_DisplayMutex.lock();
                                        std::cout << "用户登陆服务器错误" << std::endl;
                                        m_DisplayMutex.unlock();
                              }
                              cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                    }
          }
          else {
                    /*服务器可能暂时没有数据，或者连接异常*/
          }
           /*服务器没有数据的更新，因此客户端执行自己的操作*/
       
          return _shutdownflag;
}