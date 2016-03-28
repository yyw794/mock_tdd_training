
extern "C"
{
#include "Flash.h"
#include "MockIO.h"
#include "m28w160ect.h"
#include "FakeMicroTime.h"
}

#include "CppUTest/TestHarness.h"

//START: TEST_GROUP_with_MicroTime
TEST_GROUP(Flash)
{
    ioAddress address;
    ioData data;
    int result;
    int timeoutInMicroseconds;
    int fakeMicroTimeIncrement;

    void setup()
    {
        address = 0x1000;
        data = 0xBEEF;
        result = -1;
        MockIO_Create(20);
        FakeMicroTime_Init(0, 1);
        Flash_Create();
    }

    void teardown()
    {
        Flash_Destroy();
        MockIO_Verify_Complete();
        MockIO_Destroy();
    }
};

//END: TEST_GROUP_with_MicroTime

//START: ProgramSucceedsReadyImmediately_Refactored_Test
TEST(Flash, WriteSucceeds_ReadyImmediately)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit);
    MockIO_Expect_ReadThenReturn(address, data);
    result = Flash_Write(address, data);
    LONGS_EQUAL(FLASH_SUCCESS, result);
}
//END: ProgramSucceedsReadyImmediately_Refactored_Test

//START: ProgramSucceedsNotImmediatelyReady
TEST(Flash, WriteSucceeds_NotImmediatelyReady)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, 0);
    MockIO_Expect_ReadThenReturn(StatusRegister, 0);
    MockIO_Expect_ReadThenReturn(StatusRegister, 0);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit);
    MockIO_Expect_ReadThenReturn(address, data);

    result = Flash_Write(address, data);
    LONGS_EQUAL(FLASH_SUCCESS, result);
}
//END: ProgramSucceedsNotImmediatelyReady

//START: WriteFails_VppError
TEST(Flash, WriteFails_VppError)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit | VppErrorBit);
    MockIO_Expect_Write(CommandRegister, Reset);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_VPP_ERROR, result);
}
//END: WriteFails_VppError

//START: WriteFails_ProgramError
TEST(Flash, WriteFails_ProgramError)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit | ProgramErrorBit);
    MockIO_Expect_Write(CommandRegister, Reset);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_PROGRAM_ERROR, result);
}
//END: WriteFails_ProgramError

//START: WriteFails_ProtectedBlockError
TEST(Flash, WriteFails_ProtectedBlockError)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit | BlockProtectionErrorBit);
    MockIO_Expect_Write(CommandRegister, Reset);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_PROTECTED_BLOCK_ERROR, result);
}
//END: WriteFails_ProtectedBlockError

//START: WriteFails_FlashUnknownProgramError
TEST(Flash, WriteFails_FlashUnknownProgramError)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit |  EraseSuspendBit | EraseErrorBit | ProgramSuspendBit | ReservedBit);
    MockIO_Expect_Write(CommandRegister, Reset);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_UNKNOWN_PROGRAM_ERROR, result);
}
//END: WriteFails_FlashUnknownProgramError

//START: WriteFails_FlashReadBackError
TEST(Flash, WriteFails_FlashReadBackError)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit);
    MockIO_Expect_ReadThenReturn(address, data-1);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_READ_BACK_ERROR, result);
}
//END: WriteFails_FlashReadBackError

//START: WriteSucceeds_IgnoresOtherBitsUntilReady
TEST(Flash, WriteSucceeds_IgnoresOtherBitsUntilReady)
{
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    MockIO_Expect_ReadThenReturn(StatusRegister, ~ReadyBit);
    MockIO_Expect_ReadThenReturn(StatusRegister, ReadyBit);
    MockIO_Expect_ReadThenReturn(address, data);

    result = Flash_Write(address, data);

    LONGS_EQUAL(FLASH_SUCCESS, result);
}
//END: WriteSucceeds_IgnoresOtherBitsUntilReady

//START: WriteFails_Timeout_1
TEST(Flash, WriteFails_Timeout)
{
    FakeMicroTime_Init(0, 500);
    Flash_Create();
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    for (int i = 0; i < 10; i++)
        MockIO_Expect_ReadThenReturn(StatusRegister, ~ReadyBit);
    result = Flash_Write(address, data);
    LONGS_EQUAL(FLASH_TIMEOUT_ERROR, result);
}
//END: WriteFails_Timeout_1

//START: WriteFails_TimeoutAtEndOfTime
TEST(Flash, WriteFails_TimeoutAtEndOfTime)
{
    FakeMicroTime_Init(0xffffffff, 500);
    Flash_Create();
    MockIO_Expect_Write(CommandRegister, ProgramCommand);
    MockIO_Expect_Write(address, data);
    for (int i = 0; i < 10; i++)
        MockIO_Expect_ReadThenReturn(StatusRegister, ~ReadyBit);
    result = Flash_Write(address, data);
    LONGS_EQUAL(FLASH_TIMEOUT_ERROR, result);
}
//END: WriteFails_TimeoutAtEndOfTime
