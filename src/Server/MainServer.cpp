#include"ServerSocketFunction.h"

int main()
{
          timeval timeout{ 0,5000 };
          MainServer _server(timeout);
          if (!_server.initlizeServer()) {
                    Socket _listenServer(INADDR_ANY, 8834);                                 //��������˵ļ���socket����ʼ����ַ��Ϣ
                    if (_listenServer.socketAddrBind()!=SOCKET_ERROR) {
                              std::cout << "�����������ɹ�" << std::endl;
                              if (!_server.setListenQueue(_listenServer, 10)) {                      //�����ACK�����Ӷ��л����С
                                        _server.eventSelectCom(_listenServer);                     //�ڼ���socket���ܿͻ��˵�����
                              }
                    }
          }
          return 0;
}