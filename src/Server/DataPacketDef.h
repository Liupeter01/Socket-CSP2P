#include"Common.h"

enum   class CMD {
          CMD_ESTABLISHED,     //���ӽ����ɹ�����û�е�¼
          CMD_LOGIN,                    //login
          CMD_LOGIN_RESULT,   //login result
          CMD_LOGOUT,                //logout
          CMD_LOGOUT_RESULT,//logout result
          CMD_ERROR,                     //error
          CMD_NEWMEMBER_JOINED,          //�µĿͻ��˼���
          CMD_MEMBER_LEAVED,          //�����ӵĿͻ����뿪
          DEFAULT                            //no action!
};

/*
*class DataPacketHeader
*name�����ݱ���ͷ
*function description:�ײ���ʽ��Ϣ
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
          int _dataLength;                 //���ݱ���Body���ݳ���
          enum  CMD _command;     //���ݱ���������Ϣ
};

/*
*class ConnectControlPackage
*name�����ӿ��Ʊ�����
*function description:��¼��¼��Ϣ�Ͳ�����Ϣ
*/
class ConnectControlPackage :public DataPacketHeader {       //���ݱ���Body����
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
          std::string userName{ 0 };                                  //�û�����
          std::string userPassWord{ 0 };                          //�û�����
};

/*
*class :   ClientUpdatePackage
*name���ͻ��˱䶯���±�����
*function description:��ͻ��˸��������ͻ��˵���Ϣ
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
*name�����ݱ��ĵĴ���״̬
*function description:��¼���������صĲ���״̬
*/
class DataTransferState :public DataPacketHeader {
public:
          DataTransferState();
          DataTransferState(enum  CMD _cmdCommand);
          virtual void* getPacketAddr();
private:
          DataTransferState(int _datalength, enum  CMD _cmdCommand);
};