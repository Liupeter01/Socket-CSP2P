#include"ClientSocketFunction.h"

int main()
{
          MainClient client;
          if (!client.initlizeClient()) {
                    Socket dstServer(inet_addr("127.0.0.1"), PORT);                          //������ҪԶ�����ӷ������ĵ�ַ
                    while (1) {
                              if (dstServer.socketConnectServer())          //���ӷ�����
                              {
                                        std::cout << "���ӳɹ�!" << std::endl;
                                        client.UserService(dstServer);
                              }
                    }
          }
          return 0;
}