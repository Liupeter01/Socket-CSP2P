#include"ClientSocketFunction.h"

int main()
{
          timeval ts{ 0,0 };                      //超时设定
          MainClient client(ts);
          if (!client.initlizeClient()) {
                    Socket dstServer(inet_addr("127.0.0.1"), PORT);                          //创建需要远程连接服务器的地址
                    if (!dstServer.socketConnectServer())          //连接服务器
                    {
                              std::cout << "连接成功!" << std::endl;
                              client.sventSelectCom(dstServer);                           //引入select模型
                    }
          }
          return 0;
}