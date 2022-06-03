#include"DataPacketDef.h"

class CreateSocketError {                //SOCKET创建失败
public:
};

class SocketCloseFailed {               //SOCKET关闭失败
public:
};

/*declare zone*/
class MainClient;
class EventSelectStruct;

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
          Socket* getMySelf() {
                    return this;
          }

          /*Client Only*/
          int socketConnectServer();                                                                                                //socket连接服务器

          /*Server Only*/
          int socketAddrBind();                                                                                                      //Socket地址绑定工具仅限服务器
private:
          friend MainClient;
          friend EventSelectStruct;                                                                                                     //事件选择模型
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
          MainClient(timeval& t);;
          virtual ~MainClient();
public:
          bool initlizeClient();                                                                              //初始化服务器
          void sventSelectCom(Socket& _client);                                                //事件选择网络

          void ClientServiceStart(Socket& _client);                                             //启动服务
          bool UserService(Socket& _client);                                                       //服务器接收处理函数
          void UserInput(Socket& _client);                                                          //用户输入处理线程
          template<typename T>
          static void cleanArray(T* _array, int size);      //数组清理工具
      
private:  
          std::vector<std::thread>m_thread;                                                    //客户端的线程
          timeval* m_timesetting;                                                                       //客户端超时事件                   
          std::mutex m_DisplayMutex;                                                               //服务端消息输出锁
          std::mutex m_dataPacketMutex;                                                          //数据报文记录器锁
          WSADATA _wsadata;                                                                         //wsadata
          int _retValue = 0;                                                                                  //服务器函数返回值
};

class EventSelectStruct {
public:
          EventSelectStruct(const Socket& _socket);
          EventSelectStruct(const Socket& _socket, timeval& _timeval);
          virtual ~EventSelectStruct();
public:
          int StartSelect();
          int isSelectSocketRead();                                                 //判断是否设置读取描述符
          int isSelectSocketWrite();                                                 //判断是否设置读取描述符
          int isSelectSocketException();                                                 //判断是否设置读取描述符
          void cleanSelectSocketRead(const Socket*& s);                                            //清除Select模型的写入
          void cleanSelectSocketRead(const Socket& s);                                            //清除Select模型的写入

          void cleanSelectSocketWrite(const Socket& s);                                                 //清除Select模型的发送
          void cleanSelectSocketException(const Socket& s);                                          //清除Select模型的异常

          void updateClientConn(const std::vector<Socket*>& vec);        //批量更新客户的连接
          void updateClientConn(const Socket& s);                               //更新刚连接的客户
          void updateClientConn(const Socket*& s);                               //更新刚连接的客户
          size_t getReadCount();                                                            //读取的Select数据个数
          size_t getWriteCount();                                                            //写入的Select数据个数
          size_t getExceptionCount();                                                            //异常的Select数据个数
          std::vector<Socket*>::iterator getReadSocket(std::vector<Socket*>& vec, int pos);                 //根据socket的下标位置查询
public:
          const Socket& m_listenServer;
          fd_set m_fdRead;                                                    //监视文件描述符的可读(接收)集合
          fd_set m_fdWrite;                                                   //监视文件描述符的可写(发送)集合
          fd_set m_fdException;                                            //缺省
          timeval* m_timeset;
};