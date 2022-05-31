#include"DataPacketDef.h"

class CreateSocketError {                //SOCKET����ʧ��
public:
};

class SocketCloseFailed {               //SOCKET�ر�ʧ��
public:
};

/*declare zone*/
class MainClient;

class Socket {
public:
          Socket();
          Socket(unsigned long _ipaddr, unsigned short _port);
          virtual ~Socket();
public:
          void setSocketAddr(unsigned long _ipaddr, unsigned short _port);                                    //Socket��ַ���ù���
          bool socketClose();
          bool getSocketConnStatus();                                                                                                //Socket�Ƿ�������
          SOCKET getSocket(); 
          int PackageRecv(char* lppackage, int offset, int Length);
          int PackageSend(const char* lppackage, int offset, int Length);
          /*Client Only*/
          int socketConnectServer();                                                                                                //socket���ӷ�����

          /*Server Only*/
          int socketAddrBind();                                                                                                      //Socket��ַ�󶨹��߽��޷�����
private:
          friend MainClient;
          static SOCKET createTCPSocket();                                                                                    //����TCP socket
          SOCKADDR_IN&& createAddrDef(unsigned long _ipaddr, unsigned short _port);     //������ַ�����ṹ
private:
          SOCKET m_socket = INVALID_SOCKET;
          SOCKADDR_IN m_addrInfo = { 0 };
          int m_socketSizeInfo = sizeof(SOCKADDR_IN);
          bool m_socketStatus = false;                                                                                            //SOCKET״̬����
};

class MainClient {
public:
          MainClient();
          virtual ~MainClient();
public:
          bool initlizeClient();                                                                              //��ʼ��������
          void UserService(Socket& _client);                                                       //ҵ������
          template<typename T>static void cleanArray(T* _array, int size);      //����������
private:
          std::mutex m_DisplayMutex;                                                               //�������Ϣ�����
          std::mutex m_dataPacketMutex;                                                          //���ݱ��ļ�¼����
          std::list<  DataPacketHeader*> m_recivedDataPacket;               //���ݱ����ռ�¼��
          std::list<  DataPacketHeader*> m_sentDataPacket;                   //���ݱ����ͼ�¼��
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //��������������ֵ
};