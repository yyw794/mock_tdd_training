#include "Flash.h"
#include "IO.h"
#include "m28w160ect.h"

int Flash_Write(ioAddress address, ioData data)
{
	IO_Write(0x40, 0);
	IO_Write(address, data);
	IO_Read(0);
	IO_Read(address);
	return SUCCESS;
}