#include "Flash.h"
#include "IO.h"
#include "m28w160ect.h"

int Flash_Write(ioAddress address, ioData data)
{
	IO_Write(CommandRegister, ProgramCommand);
	IO_Write(address, data);
	IO_Read(StatusRegister);
	IO_Read(address);
	return SUCCESS;
}