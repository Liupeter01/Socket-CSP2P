#include"ServerSocketFunction.h"

class EventSelectStruct 
{
public:
          EventSelectStruct();
          EventSelectStruct(const Socket& s) {
                    
          }
          virtual ~EventSelectStruct();
public:
          fd_set _fdRead;                                                    //�����ļ��������Ŀɶ�(����)����
          fd_set _fdWrite;                                                   //�����ļ��������Ŀ�д(����)����
          fd_set _fdException;                                            //ȱʡ
};