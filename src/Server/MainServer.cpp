#include"ServerSocketFunction.h"

int main()
{
          MainServer _server;
          if (!_server.initlizeServer()) {
                    Socket _listenServer(INADDR_ANY, 8834);                                 //��������˵ļ���socket����ʼ����ַ��Ϣ
                    if (_listenServer.socketAddrBind()!=SOCKET_ERROR) {
                              std::cout << "�����������ɹ�" << std::endl;
                              if (!_server.setListenQueue(_listenServer, 10)) {                      //�����ACK�����Ӷ��л����С
                                        while (1) {
                                                  std::cout << "��ʼ�������ģʽ���ȴ��ͻ��˵�����........" << std::endl;
                                                  Socket _comClient;
                                                  _comClient = _server.acceptClientCom(_comClient,_listenServer);        //���ܿͻ��˵�����
                                                  if (_comClient.getSocketConnStatus()) {                                              //���ӱ�����Ч
                                                            _server.clientAddrLogger(_comClient);                        //�����¼��Ϣ
                                                            _server.clientService(_comClient);
                                                  }
                                        }
                              }
                    }
          }

          return 0;
}