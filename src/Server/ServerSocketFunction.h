#include"DataPacketDef.h"

class CreateSocketError {                //SOCKET创建失败
public:
};

class SocketCloseFailed {               //SOCKET关闭失败
public:
};

class ClientConnectFailed {               //服务器接受客户端连接失败
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
          void setSocketAddr(unsigned long _ipaddr, unsigned short _port);        //Socket地址设置工具
          bool socketClose(SOCKET* socket);
          bool getSocketConnStatus();                 //Socket是否建立连接
          int PackageRecv(char* lppackage, int offset, int Length);
          int PackageSend(const char* lppackage, int offset, int Length);

          /*Server Only*/
          int socketAddrBind();                                                                              //Socket地址绑定工具仅限服务器
private:
          friend MainServer;
          SOCKET createTCPSocket();               //创建TCP socket
          SOCKADDR_IN&& createAddrDef(unsigned long _ipaddr, unsigned short _port);     //创建地址描述结构
private:
          SOCKET m_socket = INVALID_SOCKET;
          SOCKADDR_IN m_addrInfo = { 0 };
          int m_socketSizeInfo = sizeof(SOCKADDR_IN);
          bool m_socketStatus = false;         //SOCKET连接
};

class MainServer{
public:
          MainServer();
          virtual ~MainServer();
public:
          bool initlizeServer();                                                                              //初始化服务器
          int setListenQueue(const Socket& _listenserver, int _queueSize);  //允许的ACK半连接队列缓冲大小
          Socket &acceptClientCom(Socket& _comClient, const Socket& _listenServer);           //将半连接转换为全连接ESTABLISHED
          void clientAddrLogger(const Socket& _client);                              //Client地址记录器
          void clientService(Socket& _client)      ;                                     //业务处理函数
          template<typename T>
          static  void cleanArray(T* _array, int size);
private:
          std::mutex m_acceptMutex;                                                                 //accept锁
          std::mutex m_loggerDisplayMutex;                                                    //服务端消息输出锁
          std::mutex m_dataPacketMutex;                                                          //数据报文记录器锁
          std::vector<  DataPacketHeader*> m_recivedDataPacket;               //数据报接收记录器
          std::vector<  DataPacketHeader*> m_sentDataPacket;                   //数据报发送记录器
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //服务器函数返回值
};