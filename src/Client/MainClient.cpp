#include"ClientSocketFunction.h"

int main()
{
          timeval ts{ 0,0 };                      //��ʱ�趨
          MainClient client(ts);
          if (!client.initlizeClient()) {
                    Socket dstServer(inet_addr("127.0.0.1"), PORT);                          //������ҪԶ�����ӷ������ĵ�ַ
                    if (!dstServer.socketConnectServer())          //���ӷ�����
                    {
                              std::cout << "���ӳɹ�!" << std::endl;
                              client.sventSelectCom(dstServer);                           //����selectģ��
                    }
          }
          return 0;
}