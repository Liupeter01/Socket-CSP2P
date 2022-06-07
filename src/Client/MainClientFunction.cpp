#include"ClientSocketFunction.h"

template<typename T>
void MainClient::cleanArray(T* _array, int size) {
          for (int i = 0; i < size; ++i) {
                    _array[i] = 0;
          }
}

MainClient::MainClient(timeval& t):
          m_timesetting(new timeval(t))
{
          this->_retValue = 0;
#ifdef  _WIN3264       
          this->_wsadata = { 0 };
#endif
}

MainClient::~MainClient()
{
#ifdef WINDOWSPLATFROM                                                                              //Windows ƽ̨����
          ::WSACleanup();
#endif
          for (auto ib = m_thread.begin(); ib != m_thread.end(); ++ib) {                    //�̵߳ĵȴ�
                    if ((*ib).joinable()) {
                              (*ib).join();
                    }
                    m_thread.erase(ib);
          }
}

bool MainClient::initlizeClient()
{
#ifdef  _WIN3264                                                                            //Windows ƽ̨����
          return WSAStartup(MAKEWORD(2, 2), &this->_wsadata);
#endif
          return true;
}

void MainClient::ClientServiceStart(Socket &_client)
{
          m_thread.emplace_back(&MainClient::sventSelectCom, this, std::ref(_client));
          m_thread.emplace_back(&MainClient::UserInput, this, std::ref(_client));
}

void MainClient::sventSelectCom(Socket& _client)                //�ͻ������������һ��һ������
{
          while (true)
          {
                    EventSelectStruct eventSelect(_client,*m_timesetting);
                    if (eventSelect.StartSelect(_client) < 0) {              //�ͻ��˼���Ƿ��յ�������������Ϣ
                              std::lock_guard<std::mutex> lock(m_DisplayMutex);
                              std::cout << "SELECT ��������!    ";
#ifdef _WIN3264
                              std::cout << WSAGetLastError() << std::endl;
#endif
                              break;
                    }
                    else
                    {
                              if (eventSelect.isSelectSocketRead()) { //�������Ƿ�仯�������Է��������������ͻ��˵���Ϣ
                                        /*��ʱ�������ӵ����������ӵ�P2P�ͻ���*/
                                        //for(auto ib; ; ;)
                                           /*���Է������Ǳ���Ҫ���д����*/
                                        eventSelect.cleanSelectSocketRead(_client);                 //������������ר������socket
                                        /*�Ƿ������������ÿͻ��˵�����ͨ�ſͻ���*/
                                        //for(auto ib; ; ){}
                                        if (UserService(_client)) {

                                                  std::lock_guard<std::mutex> lock(m_DisplayMutex);
                                                  std::cout << "socket �ر�" << std::endl;
                                                  break;
                                        }
                              }
                    }
          }
}


void MainClient::UserInput(Socket& _client)                           //�û������߳�
{
          while (1)
          {
                    char szSendBuffer[4096]{ 0 };                                                                                   //���ͻ�����
                    ConnectControlPackage* body = reinterpret_cast<ConnectControlPackage*>(reinterpret_cast<char*>(szSendBuffer));
                    /*�˴���Ҫʹ����������*/
                    std::string inputData;
                    std::cout << "�������Ĳ���(login,logout):";
                    std::cin >> inputData;
                    if (inputData.length() != 0 && !strcmp(inputData.c_str(), "login")) {
                              body = new (szSendBuffer)  ConnectControlPackage(CMD::CMD_LOGIN, "ADMIN", "ADMIN");
                    }
                    else if (inputData.length() != 0 && !strcmp(inputData.c_str(), "logout")) {
                              body = new (szSendBuffer)  ConnectControlPackage(CMD::CMD_LOGOUT);
                    }
                    else {
                              body = new (szSendBuffer) ConnectControlPackage(CMD::DEFAULT);
                    }
                    _client.PackageSend(szSendBuffer, 0, body->getPacketLength());                                 //�������ݱ��ķ���״̬
          }
}

bool MainClient::UserService(Socket& _client)                            //����ҵ����
{
          bool _shutdownflag(false);
          char szRecvBuffer[4096]{ 0 };                                                                                   //���ܻ�����
          char szSendBuffer[4096]{ 0 };                                                                                   //���ͻ�����

          /*�����ж��ڷ������Ĵ������ݵļ�����*/
          DataPacketHeader* header(reinterpret_cast<DataPacketHeader*> (szRecvBuffer));
          ClientUpdatePackage* update = reinterpret_cast<ClientUpdatePackage*>(
                    reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader));

          DataTransferState* state = reinterpret_cast<DataTransferState*>(
                    reinterpret_cast<char*>(szRecvBuffer) + header->getPacketLength() - sizeof(DataPacketHeader));

          if ((_retValue = _client.PackageRecv(szRecvBuffer, 0, sizeof(DataPacketHeader))) > 0) {   //����header
                    if (_retValue < sizeof(DataPacketHeader)) {                        /* �����ٰ������*/

                    }
                    else
                    {
                              _retValue = _client.PackageRecv(szRecvBuffer, sizeof(DataPacketHeader), header->getPacketLength() - sizeof(DataPacketHeader));     //ƫ��һ����Ϣͷ�ĳ���
                              if (header->getPacketCommand() == CMD::CMD_LOGIN_RESULT)                             //�û���¼�������ɹ�
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "�û���½�������ɹ�" << std::endl;
                                        m_DisplayMutex.unlock();
                              }
                              else if (header->getPacketCommand() == CMD::CMD_LOGOUT_RESULT)                  //�û��ǳ��������ɹ�
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "�û��ɹ��ǳ�������" << std::endl;
                                        m_DisplayMutex.unlock();
                                        return true;
                              }
                              else if (header->getPacketCommand() == CMD::CMD_NEWMEMBER_JOINED)          //���û��������client�б�
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_NEWMEMBER_JOINED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*�������ӵ�P2P�ͻ��˵��б�*/
                              }
                              else if (header->getPacketCommand() == CMD::CMD_MEMBER_LEAVED)               //�����ӵ��û��뿪
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_MEMBER_LEAVED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*�������ӵ�P2P�ͻ��˵��б�*/
                              }
                              else if (header->getPacketCommand() == CMD::CMD_ESTABLISHED)               //�����ӵ��û��뿪
                              {
                                        m_DisplayMutex.lock();
                                        std::cout << "CMD_ESTABLISHED" << std::endl;
                                        m_DisplayMutex.unlock();
                                        /*�������ӵ�P2P�ͻ��˵��б�*/
                              }
                              else {
                                        m_DisplayMutex.lock();
                                        std::cout << "�û���½����������" << std::endl;
                                        m_DisplayMutex.unlock();
                              }
                              cleanArray<char>(szRecvBuffer, sizeof(szRecvBuffer) / sizeof(char));
                    }
          }
          else {
                    /*������������ʱû�����ݣ����������쳣*/
          }
           /*������û�����ݵĸ��£���˿ͻ���ִ���Լ��Ĳ���*/
       
          return _shutdownflag;
}