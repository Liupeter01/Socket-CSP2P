#include"ClientSocketFunction.h"

int main()
{
          MainClient client;
          if (!client.initlizeClient()) {
                    while (1) {
                              Socket dstServer(inet_addr("127.0.0.1"), PORT);                          //������ҪԶ�����ӷ������ĵ�ַ
                              if (dstServer.socketConnectServer())          //���ӷ�����
                              {
                                        std::cout << "���ӳɹ�!" << std::endl;
                                        client.UserService(dstServer);
                              }
                    }
          }
          return 0;
}