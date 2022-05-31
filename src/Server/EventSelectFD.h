#include"ServerSocketFunction.h"

class EventSelectStruct 
{
public:
          EventSelectStruct();
          EventSelectStruct(const Socket& s) {
                    
          }
          virtual ~EventSelectStruct();
public:
          fd_set _fdRead;                                                    //监视文件描述符的可读(接收)集合
          fd_set _fdWrite;                                                   //监视文件描述符的可写(发送)集合
          fd_set _fdException;                                            //缺省
};