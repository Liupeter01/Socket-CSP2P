#include"DataPacketDef.h"

class CreateSocketError {                //SOCKET创建失败
public:
};

class SocketCloseFailed {               //SOCKET关闭失败
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
          void setSocketAddr(unsigned long _ipaddr, unsigned short _port);                                    //Socket地址设置工具
          bool socketClose();
          bool getSocketConnStatus();                                                                                                //Socket是否建立连接
          SOCKET getSocket(); 
          int PackageRecv(char* lppackage, int offset, int Length);
          int PackageSend(const char* lppackage, int offset, int Length);
          /*Client Only*/
          int socketConnectServer();                                                                                                //socket连接服务器

          /*Server Only*/
          int socketAddrBind();                                                                                                      //Socket地址绑定工具仅限服务器
private:
          friend MainClient;
          static SOCKET createTCPSocket();                                                                                    //创建TCP socket
          SOCKADDR_IN&& createAddrDef(unsigned long _ipaddr, unsigned short _port);     //创建地址描述结构
private:
          SOCKET m_socket = INVALID_SOCKET;
          SOCKADDR_IN m_addrInfo = { 0 };
          int m_socketSizeInfo = sizeof(SOCKADDR_IN);
          bool m_socketStatus = false;                                                                                            //SOCKET状态连接
};

class MainClient {
public:
          MainClient();
          virtual ~MainClient();
public:
          bool initlizeClient();                                                                              //初始化服务器
          void UserService(Socket& _client);                                                       //业务处理函数
          template<typename T>static void cleanArray(T* _array, int size);      //数组清理工具
private:
          std::mutex m_DisplayMutex;                                                               //服务端消息输出锁
          std::mutex m_dataPacketMutex;                                                          //数据报文记录器锁
          std::list<  DataPacketHeader*> m_recivedDataPacket;               //数据报接收记录器
          std::list<  DataPacketHeader*> m_sentDataPacket;                   //数据报发送记录器
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //服务器函数返回值
};