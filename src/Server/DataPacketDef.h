#include"Common.h"

enum  CMD {
          CMD_LOGIN,                    //login
          CMD_LOGIN_RESULT,   //login result
          CMD_LOGOUT,                //logout
          CMD_LOGOUT_RESULT,//logout result
          CMD_ERROR,                     //error
          DEFAULT                            //no action!
};

/*
*class DataPacketHeader
*name：数据报文头
*function description:首部格式信息
*/
class DataPacketHeader {
public:
          DataPacketHeader() :
                    _dataLength(0), _command(DEFAULT) {
          }
          virtual ~DataPacketHeader() {

          }
          virtual void* getPacketAddr();
          int getPacketLength() {
                    return this->_dataLength;
          }
          enum CMD getPacketCommand() {
                    return this->_command;
          }
protected:
          DataPacketHeader(int _dataLength, enum  CMD _cmdCommand) {
                    this->_dataLength = _dataLength + sizeof(DataPacketHeader);
                    this->_command = _cmdCommand;
          }
private:
          int _dataLength;                 //数据报文Body数据长度
          enum  CMD _command;     //数据报文命令信息
};

/*
*class DataPacketBody
*name：数据报文体
*function description:记录登录信息和操作信息
*/
class DataPacketBody :public DataPacketHeader {       //数据报文Body本体
public:
          DataPacketBody() :DataPacketHeader() {

          }
          DataPacketBody(enum  CMD _cmdCommand) :DataPacketHeader(sizeof(DataPacketBody), _cmdCommand)
          {
          }
          DataPacketBody(std::string&& _userName, std::string&& _userPass) :
                    userName(_userName), userPassWord(_userPass)
          {
          }
          DataPacketBody(enum  CMD _cmdCommand,
                    const std::string& _userName,
                    const   std::string& _userPass) :
                    userName(_userName), userPassWord(_userPass),
                    DataPacketHeader(sizeof(DataPacketBody), _cmdCommand)
          {
          }
          DataPacketBody(enum  CMD _cmdCommand,
                    std::string&& _userName,
                    std::string&& _userPass) :
                    userName(_userName), userPassWord(_userPass),
                    DataPacketHeader(sizeof(DataPacketBody), _cmdCommand)
          {
          }
          virtual void* getPacketAddr();
private:
          void setUserName(std::string&& str) {
                    userName.erase(userName.begin(), userName.end());
                    userName = str;
          }
          void setUserPass(std::string&& str) {
                    userPassWord.erase(userPassWord.begin(), userPassWord.end());
                    userPassWord = str;
          }
          DataPacketBody(int _datalength, enum  CMD _cmdCommand) :
                    DataPacketHeader(_datalength, _cmdCommand) {
          }
          std::string userName{ 0 };                                  //用户名称
          std::string userPassWord{ 0 };                          //用户密码
};

/*
*class TransferStatus
*function:数据报文的传输状态
*/
/*
*class DataTransferState
*name：
*function description:记录服务器返回的操作状态
*/
class DataTransferState :public DataPacketHeader {
public:
          DataTransferState() :DataPacketHeader() {}
          DataTransferState(enum  CMD _cmdCommand) :
                    DataTransferState(sizeof(DataTransferState), _cmdCommand) {

          }
          virtual void* getPacketAddr();
private:
          DataTransferState(int _datalength, enum  CMD _cmdCommand) :
                    DataPacketHeader(_datalength, _cmdCommand) {
          }
};