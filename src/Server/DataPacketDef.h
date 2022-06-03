#include"Common.h"

enum   class CMD {
          CMD_ESTABLISHED,     //连接建立成功但是没有登录
          CMD_LOGIN,                    //login
          CMD_LOGIN_RESULT,   //login result
          CMD_LOGOUT,                //logout
          CMD_LOGOUT_RESULT,//logout result
          CMD_ERROR,                     //error
          CMD_NEWMEMBER_JOINED,          //新的客户端加入
          CMD_MEMBER_LEAVED,          //已连接的客户端离开
          DEFAULT                            //no action!
};

/*
*class DataPacketHeader
*name：数据报文头
*function description:首部格式信息
*/
class DataPacketHeader {
public:
          DataPacketHeader();
          virtual ~DataPacketHeader();
public:
          virtual void* getPacketAddr();
          int getPacketLength();
          enum CMD getPacketCommand();
protected:
          DataPacketHeader(int _dataLength, enum  CMD _cmdCommand);
private:
          int _dataLength;                 //数据报文Body数据长度
          enum  CMD _command;     //数据报文命令信息
};

/*
*class ConnectControlPackage
*name：连接控制报文体
*function description:记录登录信息和操作信息
*/
class ConnectControlPackage :public DataPacketHeader {       //数据报文Body本体
public:
          ConnectControlPackage();
          ConnectControlPackage(enum  CMD _cmdCommand);
          ConnectControlPackage(
                    std::string&& _userName,
                    std::string&& _userPass
          );
          ConnectControlPackage(
                    enum  CMD _cmdCommand,
                    const std::string& _userName,
                    const   std::string& _userPass
          );
          ConnectControlPackage(
                    enum  CMD _cmdCommand,
                    std::string&& _userName,
                    std::string&& _userPass
          );

          virtual void* getPacketAddr();
          const std::string& getUserName();
          const std::string& getUserPassword();
private:
          void setUserName(std::string&& str);
          void setUserPass(std::string&& str);
          ConnectControlPackage(
                    int _datalength,
                    enum  CMD _cmdCommand
          );
          std::string userName{ 0 };                                  //用户名称
          std::string userPassWord{ 0 };                          //用户密码
};

/*
*class :   ClientUpdatePackage
*name：客户端变动更新报文体
*function description:向客户端更新其它客户端的信息
*/
class ClientUpdatePackage :public DataPacketHeader {
public:
          ClientUpdatePackage();
          ClientUpdatePackage(enum  CMD _cmdCommand);
public:
          virtual void* getPacketAddr();
};

/*
*class DataTransferState
*name：数据报文的传输状态
*function description:记录服务器返回的操作状态
*/
class DataTransferState :public DataPacketHeader {
public:
          DataTransferState();
          DataTransferState(enum  CMD _cmdCommand);
          virtual void* getPacketAddr();
private:
          DataTransferState(int _datalength, enum  CMD _cmdCommand);
};