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
class EventSelectStruct;

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
          Socket* getMySelf() {
                    return this;
          }

          /*Client Only*/
          int socketConnectServer();                                                                                                //socket���ӷ�����

          /*Server Only*/
          int socketAddrBind();                                                                                                      //Socket��ַ�󶨹��߽��޷�����
private:
          friend MainServer;
          friend EventSelectStruct;

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
          MainServer(timeval& t);
          virtual ~MainServer();
public:
          bool initlizeServer();                                                                              //��ʼ��������
          int setListenQueue(const Socket& _listenserver, int _queueSize);  //�����ACK�����Ӷ��л����С

          Socket* acceptClientCom(const Socket& _listenServer);                     //��������תȫ����,�������ӿͻ���ѹ������
          void eventSelectCom(const Socket& _listenServer);                                                                  //�¼�ѡ������ģ��
          void clientConnectLogger(const Socket& _client);                              //Client���ӽ�����ַ��¼��
          void clientDisconnectLogger(const Socket& _client);                          //Client�Ͽ����ӵ�ַ��¼��
          bool clientService(Socket*& _client)      ;                                     //ҵ������
          std::vector<Socket*>::iterator FindSocket(const SOCKET& s);                            //Ѱ���Ƿ����Socket
          template<typename T>
          static  void cleanArray(T* _array, int size);
private:
          timeval* m_timesetting;                                                                       //��������ʱ�¼�
          std::mutex m_acceptMutex;                                                                 //accept��
          std::mutex m_loggerDisplayMutex;                                                    //�������Ϣ�����
          std::mutex m_dataPacketMutex;                                                          //���ݱ��ļ�¼����
          std::vector< Socket*> m_connClients;                //�ͻ������Ӽ�¼��
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //��������������ֵ
};

class EventSelectStruct{
public:
          EventSelectStruct(const Socket& _socket);
          EventSelectStruct(const Socket& _socket, timeval& _timeval);
          virtual ~EventSelectStruct();
public:
          int StartSelect();
          int isSelectSocketRead();                                                 //�ж��Ƿ����ö�ȡ������
          int isSelectSocketWrite();                                                 //�ж��Ƿ����ö�ȡ������
          int isSelectSocketException();                                                 //�ж��Ƿ����ö�ȡ������
          void cleanSelectSocketRead(const Socket*& s);                                            //���Selectģ�͵�д��
          void cleanSelectSocketRead(const Socket& s);                                            //���Selectģ�͵�д��

          void cleanSelectSocketWrite(const Socket& s);                                                 //���Selectģ�͵ķ���
          void cleanSelectSocketException(const Socket& s);                                          //���Selectģ�͵��쳣
          
          void updateClientConn(const std::vector<Socket*>& vec);        //�������¿ͻ�������
          void updateClientConn(const Socket& s);                               //���¸����ӵĿͻ�
          void updateClientConn(const Socket*& s);                               //���¸����ӵĿͻ�
          size_t getReadCount();                                                            //��ȡ��Select���ݸ���
          size_t getWriteCount();                                                            //д���Select���ݸ���
          size_t getExceptionCount();                                                            //�쳣��Select���ݸ���
          std::vector<Socket*>::iterator getReadSocket(std::vector<Socket*>& vec, int pos);                 //����socket���±�λ�ò�ѯ
public:
          const Socket& m_listenServer;
          fd_set m_fdRead;                                                    //�����ļ��������Ŀɶ�(����)����
          fd_set m_fdWrite;                                                   //�����ļ��������Ŀ�д(����)����
          fd_set m_fdException;                                            //ȱʡ
          timeval* m_timeset;
};