#include"DataPacketDef.h"

DataPacketHeader::DataPacketHeader() :
          _dataLength(0), _command(DEFAULT) {
}

DataPacketHeader::~DataPacketHeader() 
{

}

enum CMD DataPacketHeader::getPacketCommand() 
{
          return this->_command;
}

int DataPacketHeader::getPacketLength() 
{
          return this->_dataLength;
}

DataPacketHeader::DataPacketHeader(int _dataLength, enum  CMD _cmdCommand) 
{
          this->_dataLength = _dataLength + sizeof(DataPacketHeader);
          this->_command = _cmdCommand;
}

void* DataPacketHeader::getPacketAddr()
{
          return this;
}

ConnectControlPackage::ConnectControlPackage() :DataPacketHeader() 
{

}

ConnectControlPackage::ConnectControlPackage(enum  CMD _cmdCommand) : 
          DataPacketHeader(sizeof(ConnectControlPackage), _cmdCommand){
}

ConnectControlPackage::ConnectControlPackage(std::string&& _userName, std::string&& _userPass) :
          userName(_userName), userPassWord(_userPass){
}

ConnectControlPackage::ConnectControlPackage(enum  CMD _cmdCommand,
          const std::string& _userName,
          const   std::string& _userPass) :
          userName(_userName), userPassWord(_userPass),
          DataPacketHeader(sizeof(ConnectControlPackage), _cmdCommand){
}

ConnectControlPackage::ConnectControlPackage(enum  CMD _cmdCommand,
          std::string&& _userName,
          std::string&& _userPass) :
          userName(_userName), userPassWord(_userPass),
          DataPacketHeader(sizeof(ConnectControlPackage), _cmdCommand){
}

void* ConnectControlPackage::getPacketAddr() 
{
          return this;
}

void    ConnectControlPackage::setUserName(std::string&& str) 
{
          userName.erase(userName.begin(), userName.end());
          userName = str;
}

void    ConnectControlPackage::setUserPass(std::string&& str) 
{
          userPassWord.erase(userPassWord.begin(), userPassWord.end());
          userPassWord = str;
}

ConnectControlPackage::ConnectControlPackage(int _datalength, enum  CMD _cmdCommand) :
          DataPacketHeader(_datalength, _cmdCommand) {
}

void* ClientUpdatePackage::getPacketAddr()
{
          return this;
}

DataTransferState::DataTransferState() :
          DataPacketHeader() {
}

DataTransferState::DataTransferState(enum  CMD _cmdCommand) :
          DataTransferState(sizeof(DataTransferState), _cmdCommand) {
}

DataTransferState::DataTransferState(int _datalength, enum  CMD _cmdCommand) :
          DataPacketHeader(_datalength, _cmdCommand) {
}

void* DataTransferState::getPacketAddr()
{
          return this;
}