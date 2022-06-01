#include"ServerSocketFunction.h"

int main()
{
          timeval timeout{ 0,5000 };
          MainServer _server(timeout);
          if (!_server.initlizeServer()) {
                    Socket _listenServer(INADDR_ANY, 8834);                                 //创建服务端的监听socket并初始化地址信息
                    if (_listenServer.socketAddrBind()!=SOCKET_ERROR) {
                              std::cout << "服务器启动成功" << std::endl;
                              if (!_server.setListenQueue(_listenServer, 10)) {                      //允许的ACK半连接队列缓冲大小
                                        _server.eventSelectCom(_listenServer);                     //在监听socket接受客户端的连接
                              }
                    }
          }
          return 0;
}