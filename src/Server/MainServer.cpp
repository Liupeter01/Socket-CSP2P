#include"ServerSocketFunction.h"

int main()
{
          MainServer _server;
          if (!_server.initlizeServer()) {
                    Socket _listenServer(INADDR_ANY, 8834);                                 //创建服务端的监听socket并初始化地址信息
                    if (_listenServer.socketAddrBind()!=SOCKET_ERROR) {
                              std::cout << "服务器启动成功" << std::endl;
                              if (!_server.setListenQueue(_listenServer, 10)) {                      //允许的ACK半连接队列缓冲大小
                                        while (1) {
                                                  std::cout << "开始进入监听模式，等待客户端的连接........" << std::endl;
                                                  Socket _comClient;
                                                  _comClient = _server.acceptClientCom(_comClient,_listenServer);        //接受客户端的连接
                                                  if (_comClient.getSocketConnStatus()) {                                              //连接必须有效
                                                            _server.clientAddrLogger(_comClient);                        //输出登录信息
                                                            _server.clientService(_comClient);
                                                  }
                                        }
                              }
                    }
          }

          return 0;
}