#include"ClientSocketFunction.h"

int main()
{
          MainClient client;
          if (!client.initlizeClient()) {
                    Socket dstServer(inet_addr("127.0.0.1"), PORT);                          //创建需要远程连接服务器的地址
                    while (1) {
                              if (dstServer.socketConnectServer())          //连接服务器
                              {
                                        std::cout << "连接成功!" << std::endl;
                                        client.UserService(dstServer);
                              }
                    }
          }
          return 0;
}