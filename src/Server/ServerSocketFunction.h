#include"DataPacketDef.h"

class CreateSocketError {                //SOCKET����ʧ��
public:
};

class SocketCloseFailed {               //SOCKET�ر�ʧ��
public:
};

class ClientConnectFailed {               //���������ܿͻ�������ʧ��
public:
};

/*declare zone*/
class MainServer;

class Socket {
public:
          Socket();
          Socket(unsigned long _ipaddr, unsigned short _port);
          virtual ~Socket();
public:
          void setSocketAddr(unsigned long _ipaddr, unsigned short _port);        //Socket��ַ���ù���
          bool socketClose();
          bool getSocketConnStatus();                 //Socket�Ƿ�������
          SOCKET getSocket(); 
          int PackageRecv(char* lppackage, int offset, int Length);
          int PackageSend(const char* lppackage, int offset, int Length);

          /*Server Only*/
          int socketAddrBind();                                                                              //Socket��ַ�󶨹��߽��޷�����
private:
          friend MainServer;
          static SOCKET createTCPSocket();               //����TCP socket
          SOCKADDR_IN&& createAddrDef(unsigned long _ipaddr, unsigned short _port);     //������ַ�����ṹ
private:
          SOCKET m_socket = INVALID_SOCKET;
          SOCKADDR_IN m_addrInfo = { 0 };
          int m_socketSizeInfo = sizeof(SOCKADDR_IN);
          bool m_socketStatus = false;         //SOCKET����
};

class MainServer{
public:
          MainServer();
          virtual ~MainServer();
public:
          bool initlizeServer();                                                                              //��ʼ��������
          int setListenQueue(const Socket& _listenserver, int _queueSize);  //�����ACK�����Ӷ��л����С

          void acceptClientCom(const Socket& _listenServer);                     //��������תȫ����,�������ӿͻ���ѹ������
          void clientAddrLogger(const Socket& _client);                              //Client��ַ��¼��
          void clientService(Socket& _client)      ;                                     //ҵ������
          std::vector<Socket>::iterator FindSocket(const SOCKET& s);                            //Ѱ���Ƿ����Socket
          template<typename T>
          static  void cleanArray(T* _array, int size);
private:
          fd_set _fdRead;                                                                                      //�����ļ��������Ŀɶ�(����)����
          fd_set _fdWrite;                                                                                    //�����ļ��������Ŀ�д(����)����
          fd_set _fdException;                                                                             //ȱʡ
          std::mutex m_acceptMutex;                                                                 //accept��
          std::mutex m_loggerDisplayMutex;                                                    //�������Ϣ�����
          std::mutex m_dataPacketMutex;                                                          //���ݱ��ļ�¼����
          std::vector< Socket> m_connClients;                //�ͻ������Ӽ�¼��
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //��������������ֵ
};