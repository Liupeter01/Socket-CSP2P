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
          bool socketClose();
          bool getSocketConnStatus();                 //Socket是否建立连接
          SOCKET getSocket(); 
          int PackageRecv(char* lppackage, int offset, int Length);
          int PackageSend(const char* lppackage, int offset, int Length);

          /*Server Only*/
          int socketAddrBind();                                                                              //Socket地址绑定工具仅限服务器
private:
          friend MainServer;
          static SOCKET createTCPSocket();               //创建TCP socket
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

          void acceptClientCom(const Socket& _listenServer);                     //将半连接转全连接,并将连接客户端压入容器
          void clientAddrLogger(const Socket& _client);                              //Client地址记录器
          void clientService(Socket& _client)      ;                                     //业务处理函数
          std::vector<Socket>::iterator FindSocket(const SOCKET& s);                            //寻找是否存在Socket
          template<typename T>
          static  void cleanArray(T* _array, int size);
private:
          fd_set _fdRead;                                                                                      //监视文件描述符的可读(接收)集合
          fd_set _fdWrite;                                                                                    //监视文件描述符的可写(发送)集合
          fd_set _fdException;                                                                             //缺省
          std::mutex m_acceptMutex;                                                                 //accept锁
          std::mutex m_loggerDisplayMutex;                                                    //服务端消息输出锁
          std::mutex m_dataPacketMutex;                                                          //数据报文记录器锁
          std::vector< Socket> m_connClients;                //客户端连接记录器
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //服务器函数返回值
};