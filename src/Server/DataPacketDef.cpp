#include"DataPacketDef.h"

void* DataPacketHeader::getPacketAddr()
{
          return this;
}

void* DataPacketBody::getPacketAddr()
{
          return this;
}

void* DataTransferState::getPacketAddr()
{
          return this;
}