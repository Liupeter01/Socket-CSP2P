#include"ServerSocketFunction.h"

Socket::Socket()
{
          this->m_socket = INVALID_SOCKET;
          this->m_addrInfo = { 0 };
}

Socket::Socket(unsigned long _ipaddr, unsigned short _port)
{
          if ((this->m_socket = createTCPSocket()) != INVALID_SOCKET) {
                    this->m_addrInfo = createAddrDef(_ipaddr, _port);
                    return;
          }
}

Socket::~Socket()
{
          if (!socketClose()) {
                    std::cout << "SOCKET关闭失败" << std::endl;
                    return;
          }
          this->m_socketStatus = false;           //SOCKET直接被关闭
}

void Socket::setSocketAddr(unsigned long _ipaddr, unsigned short _port)
{
          this->m_addrInfo = createAddrDef(_ipaddr, _port);
}

int  Socket::socketAddrBind() {                 //Socket地址绑定工具仅限服务器
#ifdef  _WIN3264                                                                             //Windows 平台适配
          return ::bind(m_socket, reinterpret_cast<const sockaddr*>(&m_addrInfo), sizeof(SOCKADDR_IN));
#endif
}

bool  Socket::socketClose() {
          try {
                    if (this->m_socket == INVALID_SOCKET) {
                              throw SocketCloseFailed();
                    }
                    closesocket(this->m_socket);
                    this->m_socket = INVALID_SOCKET;
          }
          catch (const SocketCloseFailed&) {
#ifdef _DEBUG
                    std::cout << "[DEBUG INFO] : SOCKET Close ERROR! \n" << WSAGetLastError() << std::endl;
#endif 
                    return false;
          }
          return true;
}

bool Socket::getSocketConnStatus()
{
          return m_socketStatus;
}

SOCKET Socket::getSocket() 
{
          return m_socket;
}

SOCKET Socket::createTCPSocket()
{
          SOCKET _socket;
          try {
                    if ((_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
                              throw CreateSocketError();
                    }
          }
          catch (const CreateSocketError&) {
#ifdef _DEBUG
                    std::cout << "[DEBUG INFO] : SOCKET CREATE ERROR! \n" << WSAGetLastError() << std::endl;
#endif
          }
          return _socket;
}

SOCKADDR_IN&& Socket::createAddrDef(unsigned long _ipaddr, unsigned short _port)      //创建地址描述结构
{
          SOCKADDR_IN _temp{ 0 };
          _temp.sin_addr.S_un.S_addr = _ipaddr;     //ip任意
          _temp.sin_family = AF_INET;
          _temp.sin_port = ::htons(_port);
          return std::move(_temp);
}

int Socket::PackageRecv(char* lppackage, int offset, int Length)
{
#ifdef  _WIN3264
          return ::recv(m_socket, lppackage + offset, Length, 0);
#endif
}

int Socket::PackageSend(const char* lppackage, int offset, int Length)
{
#ifdef  _WIN3264      
          return ::send(m_socket, lppackage + offset, Length, 0);
#endif
}

