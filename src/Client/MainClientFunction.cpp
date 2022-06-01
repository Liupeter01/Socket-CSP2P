#include"ClientSocketFunction.h"

template<typename T>
void MainClient::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

MainClient::MainClient()
{
          this->_retValue = 0;
          this->_wsadata = { 0 };
}

MainClient::~MainClient()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          ::WSACleanup();
#endif
          for (auto ib = m_recivedDataPacket.begin(); ib != m_recivedDataPacket.end(); ib++) {
                    if (*ib == nullptr) {
                              delete* ib;
                    }
          }
          for (auto ib = m_sentDataPacket.begin(); ib != m_sentDataPacket.end(); ib++) {
                    if (*ib == nullptr) {
                              delete* ib;
                    }
          }
}

bool MainClient::initlizeClient()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows 平台适配
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
}

void MainClient::sventSelectCom(Socket& _client)                //客户端与服务器是一对一的连接
{
          while (true)
          {
                    EventSelectStruct eventSelect(_client);
                    if (eventSelect.StartSelect() < 0) {              //客户端检测是否收到服务器的新消息
                              std::cout << "SELECT 工作错误!    " << WSAGetLastError() << std::endl;
                              break;
                    }
                    else
                    {
                              if (eventSelect.isSelectSocketRead()) { //描述符是否变化处理来自服务器或者其他客户端的消息
                                        /*此时建立连接的是其他连接的P2P客户端*/
                                        //for(auto ib; ; ;)
                                      
                              }
                              /*来自服务器是必须要进行处理的*/
                              eventSelect.cleanSelectSocketRead(_client);                 //清除处理服务器专属连接socket
                              /*是否存在其他连入该客户端的其他通信客户端*/
                              //for(auto ib; ; ){}
                              UserService(_client);
                    }
          }
}

bool MainClient::UserService(Socket& _client)                            //核心业务函数
{
          bool _shutdownflag(false);
          char szRecvBuffer[4096]{ 0 };                                                                                   //接受缓冲区
          char szSendBuffer[4096]{ 0 };                                                                                   //发送缓冲区
          ConnectControlPackage* body(reinterpret_cast<ConnectControlPackage*>(reinterpret_cast<char*>(szSendBuffer)));
          std::string inputData;
          /*多线程输入处理!!!*/
          std::cout << "输入您的操作(login,logout):";
          std::cin >> inputData;
          if (inputData.length() != 0 && !strcmp(inputData.c_str(), "login")) {
                    body = new (szSendBuffer)  ConnectControlPackage(CMD_LOGIN, "ADMIN", "ADMIN");
          }
          else if (inputData.length() != 0 && !strcmp(inputData.c_str(), "logout")) {
                    body = new (szSendBuffer)  ConnectControlPackage(CMD_LOGOUT);
          }
          else {
                    body = new (szSendBuffer) ConnectControlPackage(DEFAULT);
          }
          _client.PackageSend(szSendBuffer, 0, body->getPacketLength());                                 //发送数据报文返回状态
          cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
          cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
          DataPacketHeader* header(reinterpret_cast<DataPacketHeader*>(szRecvBuffer));
          DataTransferState* state(reinterpret_cast<DataTransferState*>(
                    reinterpret_cast<char*>(szRecvBuffer)) + header->getPacketLength() - sizeof(DataPacketHeader));
          if ((_retValue = _client.PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0)   //先读取消息头无偏移
          {
                    /* 出现少包的情况*/
                    if (_retValue < sizeof(DataPacketHeader)) {

                    }
                    else {
                              _retValue = _client.PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));
                              if (header->getPacketCommand() == CMD_LOGIN_RESULT) {
                                        std::cout << "用户登陆服务器成功" << std::endl;
                              }
                              else if (header->getPacketCommand() == CMD_LOGOUT_RESULT) {
                                        std::cout << "用户登陆服务器失败" << std::endl;
                                        _shutdownflag = false;
                              }
                              else {
                                        std::cout << "用户登陆服务器错误" << std::endl;
                              }
                    }
                    cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                    cleanArray<char>(szSendBuffer, sizeof(szSendBuffer) / sizeof(char));
          }
          else
          {

          }
          return _shutdownflag;
}