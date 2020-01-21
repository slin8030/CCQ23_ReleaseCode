/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
 1.01      11/28/14  Champion Liu Fix the system hang during only EC Index IO support.
           12/05/14  Champion Liu Save/Restore the IO index value before 0xFD60 IO index read/write.
           02/03/15  Champion Liu Modify the return value of "get ec version" by EC cmd 0x51.
*/

#include <CompalECLib.h>

#define WORD_SHIFT_GET_HByte	0x08
#define EC_INDEXIO_BASE_OFFSET_HIGH_BYTE	0x01
#define EC_INDEXIO_BASE_OFFSET_LOW_BYTE		0x02
#define EC_INDEXIO_BASE_OFFSET_DATA_BYTE	0x03

//[COM-START] For Auto detect EC interface
//Offset of implement bytes for memory mapping
#define  EC_MEMMAPPING_IMPLEMENT_OFFSET_1  0xF980
#define  EC_MEMMAPPING_IMPLEMENT_OFFSET_2  0xF981

//Specific mark for check memory mapping implemented or not
#define  EC_MEMMAPPING_IMPLEMENT_DATA_1  0x5A
#define  EC_MEMMAPPING_IMPLEMENT_DATA_2  0xA5
//[COM-END]

//+[COM-START] VF20110425
void
CompalECMemoryMappingCmdDataTranslation(
    IN  UINT8    ECCmd,
    IN  UINT8    ECDataLen,
    IN  UINT8    *ECDataBufferPtr
)
{
    UINT8 Temp = 0;
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_BUFFER, ECCmd);
    while ( ECDataLen )
    {
        CompalECMemoryMappingWrite(EC_INDEXIO_DATA_OF_CMD_BUFFER+Temp, *(ECDataBufferPtr+Temp));
        ECDataLen--;
        Temp++;
    }
}

EFI_STATUS
CompalECMemoryMappingCmdWrite (
    IN  UINT8  ECCmd,
    IN  UINT8  ECDataLen,
    IN  UINT8  *ECDataBufferPtr
)
{
    UINT8 Temp = 0xFF;

    //Step(1): Wait for CMD Control bit0 = 0
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK )
    {
        CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, 0x00);
        Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(2): Set CMD control bit0(Process) = 1
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, EC_INDEXIO_CMD_CNTL_BIT_PROCESS);

    //Step (3): Move Command & Data Buffer into EC Memory area
    CompalECMemoryMappingCmdDataTranslation(ECCmd,ECDataLen,ECDataBufferPtr);

    //Step(4): Set CMD control bit1(Start) = 1
    Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    Temp |= EC_INDEXIO_CMD_CNTL_BIT_START;
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, Temp);

    //Step(5): wait for EC to excute this command
    Temp = 0xff;
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_START_MASK  )
    {
        Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    }

    //Step (6): Clear CMD control bit0(Process) = 0, finish all Command process
    Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    Temp = Temp & ~(EC_INDEXIO_CMD_CNTL_BIT_PROCESS);
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, Temp);

    return EFI_SUCCESS;
}

EFI_STATUS
CompalECMemoryMappingCmdRead (
    IN  UINT8  ECCmd,
    IN  UINT8  ECDataLen,
    IN  UINTN  ECReturnDataLen,
    IN  OUT UINT8  *ECDataBufferPtr
)
{
    UINT8 Temp = 0xFF;

    //Step(1): Wait for CMD Control bit0 = 0
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK )
    {
        CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, 0x00);
        Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(2): Set CMD control bit0(Process) = 1
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, EC_INDEXIO_CMD_CNTL_BIT_PROCESS);

    //Step(3): Move Command & Data Buffer into EC Memory area
    CompalECMemoryMappingCmdDataTranslation(ECCmd,ECDataLen,ECDataBufferPtr);

    //Step(4): Set CMD control bit1(Start) = 1
    Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    Temp |= EC_INDEXIO_CMD_CNTL_BIT_START;
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, Temp);

    //Step(5): wait for EC to excute this command
    Temp = 0xff;
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_START_MASK  )
    {
        Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    }

    //Step (6): move return data buffer to caller pointer
    Temp = 0x00;
    while ( ECReturnDataLen )
    {
        *(ECDataBufferPtr+Temp) = CompalECMemoryMappingRead(EC_INDEXIO_CMD_RETURN_DATA_BUFFER+Temp);
        Temp++;
        ECReturnDataLen--;
    }

    //Step (7): Clear CMD control bit0(Process) = 0, finish all Command process
    Temp = CompalECMemoryMappingRead(EC_INDEXIO_CMD_CNTL);
    Temp = Temp & ~(EC_INDEXIO_CMD_CNTL_BIT_PROCESS);
    CompalECMemoryMappingWrite(EC_INDEXIO_CMD_CNTL, Temp);

    return EFI_SUCCESS;
}

UINT8
CompalECMemoryMappingRead (
    IN  UINTN  Offset
)
{
    UINT8  	MemTemp;
    UINT16	MemOffset;

    MemOffset = (UINT16)(Offset - EC_RAM_START_ADDRESS);

    MemTemp = MmioRead8(EC_MEMORY_MAPPING_BASE_ADDRESS+MemOffset);

    return MemTemp;
}

VOID
CompalECMemoryMappingWrite (
    IN  UINTN  Offset,
    IN  UINT8  WriteData
)
{
    UINT16	MemOffset;

    MemOffset = (UINT16)(Offset - EC_RAM_START_ADDRESS);

    do {
        MmioWrite8 (EC_MEMORY_MAPPING_BASE_ADDRESS+MemOffset,WriteData);
    } while (CompalECMemoryMappingRead(Offset) != WriteData);
}

VOID
CompalECIndexCmdDataTranslation (
    IN  UINT8  ECCmd,
    IN  UINT8  ECDataLen,
    IN  UINT8  *ECDataBufferPtr
)
{
    UINT8 Temp = 0;

    CompalECIndexIOWrite(EC_INDEXIO_CMD_BUFFER, ECCmd);
    while ( ECDataLen )
    {
        CompalECIndexIOWrite(EC_INDEXIO_CMD_BUFFER+Temp+1, *(ECDataBufferPtr+Temp));
        ECDataLen--;
        Temp++;
    }
}

EFI_STATUS
CompalECIndexCmdWrite (
    IN  UINT8  ECCmd,
    IN  UINT8  ECDataLen,
    IN  UINT8  *ECDataBufferPtr
)
{
    UINT8 Temp = 0xFF;

    //Step(1): Wait for CMD Control bit0 = 0
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK ) {
        CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, 0x00);
        Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(2): Set CMD control bit0(Process) = 1
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, EC_INDEXIO_CMD_CNTL_BIT_PROCESS);

    //Step (3): Move Command & Data Buffer into Index IO area
    CompalECIndexCmdDataTranslation(ECCmd,ECDataLen,ECDataBufferPtr);

    //Step(4): Set CMD control bit1(Start) = 1
    Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    Temp |= EC_INDEXIO_CMD_CNTL_BIT_START;
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, Temp);

    //Step(5): wait for EC to excute this command
    Temp = 0xff;
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_START_MASK  ) {
        Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(6): Clear CMD control bit0(Process) = 0, finish all Command process
    Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    Temp = Temp & ~(EC_INDEXIO_CMD_CNTL_BIT_PROCESS);
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, Temp);

    return EFI_SUCCESS;
}

EFI_STATUS
CompalECIndexCmdRead (
    IN  UINT8  ECCmd,
    IN  UINT8  ECDataLen,
    IN  UINTN  ECReturnDataLen,
    IN  OUT UINT8  *ECDataBufferPtr
)
{
    UINT8 Temp = 0xFF;

    //Step(1): Wait for CMD Control bit0 = 0
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK )
    {
        CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, 0x00);
        Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(2): Set CMD control bit0(Process) = 1
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, EC_INDEXIO_CMD_CNTL_BIT_PROCESS);

    //Step(3): Move Command & Data Buffer into Index IO area
    CompalECIndexCmdDataTranslation(ECCmd,ECDataLen,ECDataBufferPtr);

    //Step(4): Set CMD control bit1(Start) = 1
    Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    Temp |= EC_INDEXIO_CMD_CNTL_BIT_START;
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, Temp);

    //Step(5): wait for EC to excute this command
    Temp = 0xff;
    while ( Temp & EC_INDEXIO_CMD_CNTL_BIT_START_MASK  )
    {
        Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    }

    //Step(6): move return data buffer to caller pointer
    Temp = 0x00;
    while ( ECReturnDataLen )
    {
        *(ECDataBufferPtr+Temp) = CompalECIndexIORead(EC_INDEXIO_CMD_RETURN_DATA_BUFFER+Temp);
        Temp++;
        ECReturnDataLen--;
    }

    //Step (7): Clear CMD control bit0(Process) = 0, finish all Command process
    Temp = CompalECIndexIORead(EC_INDEXIO_CMD_CNTL);
    Temp = Temp & ~(EC_INDEXIO_CMD_CNTL_BIT_PROCESS);
    CompalECIndexIOWrite(EC_INDEXIO_CMD_CNTL, Temp);

    return EFI_SUCCESS;
}

UINT8
CompalECIndexIORead (
    IN  UINTN  Offset
)
{
    UINTN   IndexIoBase;
    UINT8		ReturnData;
    UINT8   IndexHighByte;
    UINT8   IndexLowByte;

    IndexIoBase = EC_INDEXIO_BASE;

    IndexHighByte = IoRead8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE));
    IndexLowByte = IoRead8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE));

    //High Byte of Address for EC RAM Offset
    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE), (UINT8)(Offset >> WORD_SHIFT_GET_HByte));
    //Low Byte of Address for EC RAM Offset
    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE), (UINT8) Offset);
    ReturnData = IoRead8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_DATA_BYTE));

    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE), IndexHighByte);
    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE), IndexLowByte);

    return ReturnData;
}

VOID
CompalECIndexIOWrite (
    IN  UINTN  Offset,
    IN  UINT8  WriteData
)
{
    UINTN   IndexIoBase;
    UINT8   IndexHighByte;
    UINT8   IndexLowByte;

    IndexIoBase = EC_INDEXIO_BASE;

    IndexHighByte = IoRead8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE));
    IndexLowByte = IoRead8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE));

    do {
        //High Byte of Address for EC RAM Offset
        IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE), (UINT8)(Offset >> WORD_SHIFT_GET_HByte));
        //Low Byte of Address for EC RAM Offset
        IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE), (UINT8) Offset);
        IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_DATA_BYTE), WriteData);
    } while (CompalECIndexIORead(Offset) != WriteData);

    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_HIGH_BYTE), IndexHighByte);
    IoWrite8 ((IndexIoBase + EC_INDEXIO_BASE_OFFSET_LOW_BYTE), IndexLowByte);
}

VOID
CompalECReadENEData (
    IN  UINT8  LowAddress,
    IN  UINT8  HighAddress,
    OUT UINT8  *OutData
)
{
    UINTN   IOBASE;
    UINT8   HighByte;
    UINT8   LowByte;

    IOBASE = EC_INDEXIO_BASE;

    HighByte = IoRead8 (IOBASE + 1);
    LowByte = IoRead8 (IOBASE + 2);

    IoWrite8 ((IOBASE + 1), HighAddress);
    IoWrite8 ((IOBASE + 2), LowAddress);
    *OutData = IoRead8 (IOBASE +3);

    IoWrite8 ((IOBASE + 1), HighByte);
    IoWrite8 ((IOBASE + 2), LowByte);
}

VOID
CompalECWriteENEData (
    IN  UINT8  LowAddress,
    IN  UINT8  HighAddress,
    IN  UINT8  InputData
)
{
    UINTN   IOBASE;
    UINT8   HighByte;
    UINT8   LowByte;

    IOBASE = EC_INDEXIO_BASE;

    HighByte = IoRead8 (IOBASE + 1);
    LowByte = IoRead8 (IOBASE + 2);

    IoWrite8 ((IOBASE + 1), HighAddress);
    IoWrite8 ((IOBASE + 2), LowAddress);
    IoWrite8 ((IOBASE + 3), InputData);

    IoWrite8 ((IOBASE + 1), HighByte);
    IoWrite8 ((IOBASE + 2), LowByte);
}

UINT8
CompalECMemoryMappingInterfaceCheck()
{
    UINT8 ImplementData_BUF[2]={0};

    ImplementData_BUF[0] = CompalECMemoryMappingRead(EC_MEMMAPPING_IMPLEMENT_OFFSET_1);
    ImplementData_BUF[1] = CompalECMemoryMappingRead(EC_MEMMAPPING_IMPLEMENT_OFFSET_2);

    if ( (ImplementData_BUF[0] == EC_MEMMAPPING_IMPLEMENT_DATA_1) &&
            (ImplementData_BUF[1] == EC_MEMMAPPING_IMPLEMENT_DATA_2) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
CompalECWriteCmdByte (
    IN UINT8   ECCmdPortNum,
    IN UINT8   ECCmd,
    IN UINT8   ECData
)
{
    UINT8   EcCmdBuf[EC_CMD_BUF_LEN];

    EcCmdBuf[0] = ECData;
    CompalECWriteCmd (ECCmdPortNum, ECCmd, 0x01, EcCmdBuf);
}

VOID
CompalECWriteCmdTwoByte (
    IN UINT8   ECCmdPortNum,
    IN UINT8   ECCmd,
    IN UINT8   ECData1,
    IN UINT8   ECData2
)
{
    UINT8   EcCmdBuf[EC_CMD_BUF_LEN];

    EcCmdBuf[0] = ECData1;
    EcCmdBuf[1] = ECData2;
    CompalECWriteCmd (ECCmdPortNum, ECCmd, 0x02, EcCmdBuf);
}

EFI_STATUS
CompalECWriteCmd (
    IN UINT8    ECCmdPortNum,
    IN UINT8    ECCmd,
    IN UINT8    ECDataLen,
    IN UINT8    *ECDataCmdPTR
)
{
    EFI_STATUS  ErrorStatus = EFI_TIMEOUT;
    UINT8       EcDataPort;
    UINT8       EcStatusPort;
    UINT8       Tmp1;
    UINT8       IdleModeFlag = FALSE;
    UINT8       RetryCnt =0;                // Retry 3 times if OBF or IBE error.

    if (ECCmdPortNum == EC_INDEXIO_PORT) {
        CompalECIndexCmdWrite(ECCmd,ECDataLen,ECDataCmdPTR);
    } else if (ECCmdPortNum == EC_MEMORY_MAPPING_PORT) {
        //[COM-START] Auto detect EC interface
        //Check memory mapping implement
        if ( CompalECMemoryMappingInterfaceCheck() )
            CompalECMemoryMappingCmdWrite(ECCmd,ECDataLen,ECDataCmdPTR);
        else //use Index IO
            CompalECIndexCmdWrite(ECCmd,ECDataLen,ECDataCmdPTR);
        //[COM-END]
    } else {
        while ( (ErrorStatus == EFI_TIMEOUT) && (RetryCnt < EC_CMD_RETRY_COUNT) ) {
            RetryCnt++;
            if ( ECCmdPortNum == EC_8042_CMD_PORT ) {
                EcDataPort   = EC_8042_DATA_PORT;
                EcStatusPort = EC_8042_STATUS_PORT;
            } else if ( ECCmdPortNum == EC_EXT_8042_CMD_PORT ) {
                EcDataPort   = EC_EXT_8042_DATA_PORT;
                EcStatusPort = EC_EXT_8042_STATUS_PORT;

                // Chk Busy Flag for 0x6C
                ErrorStatus = CompalECCheckBusyFlag(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            } else {
                EcDataPort   = EC_ACPI_DATA_PORT;
                EcStatusPort = EC_ACPI_STATUS_PORT;
            }

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            if ( (ECCmd == EC_CMD_SYSTEM_NOTIFICATION) &&
                    (*ECDataCmdPTR == EC_CMD_SYSTEM_NOTIFICATION_IDLE_MODE ||
                     *ECDataCmdPTR == EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_FLASH_COMPLETE ||
                     *ECDataCmdPTR == EC_CMD_SYSTEM_NOTIFICATION_WAIT_MODE) ) {
                IdleModeFlag = TRUE;
            }

            if ( ECCmdPortNum == EC_8042_CMD_PORT && !IdleModeFlag) {
                IoWrite8(EC_8042_CMD_PORT,Disable_Keyboard_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;

                IoWrite8(EC_8042_CMD_PORT,Disable_Auxiliary_Device_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            }

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            IoWrite8(ECCmdPortNum,ECCmd);
            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            if ( ECDataLen != EC_NO_DATACMD_BYTE ) {
                for (Tmp1= 0; Tmp1 < ECDataLen; Tmp1++) {
                    IoWrite8(EcDataPort,*(ECDataCmdPTR+Tmp1));
                    ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                    if ( ErrorStatus == EFI_TIMEOUT ) break;
                }
            }
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            if ( ECCmdPortNum == EC_8042_CMD_PORT && !IdleModeFlag ) {
                IoWrite8(EC_8042_CMD_PORT, Enable_Keyboard_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;

                IoWrite8(EC_8042_CMD_PORT, Enable_Auxiliary_Device_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            }

            if ( ECCmdPortNum == EC_EXT_8042_CMD_PORT ) {
                IoWrite8(EC_EXT_8042_CMD_PORT, 0xFF);
            }
        }
    }

    if ( RetryCnt >= EC_CMD_RETRY_COUNT ) return  EFI_TIMEOUT;

    return EFI_SUCCESS;
}

UINT8
CompalECReadCmdByte (
    IN UINT8   ECCmdPortNum,
    IN UINT8   ECCmd,
    IN UINT8   ECData
)
{
    UINT8   EcDataBuf[EC_CMD_BUF_LEN];

    EcDataBuf[0] = ECData;
    CompalECReadCmd(ECCmdPortNum, ECCmd, 0x01, 0x01, EcDataBuf);

    return EcDataBuf[0];
}

EFI_STATUS
CompalECReadCmd (
    IN UINT8        ECCmdPortNum,
    IN UINT8        ECCmd,
    IN UINT8        ECDataLen,
    IN UINTN        ECRetrunDataLen,
    IN OUT UINT8    *ECReturnBufPTR
)
{
    UINT8       EcDataPort;
    UINT8       EcStatusPort;
    UINT8       Tmp1;
    UINT8       Data;
    UINT8       RetryCnt = 0;   // Retry 3 times if OBF or IBE error.

    EFI_STATUS  ErrorStatus = EFI_TIMEOUT;

    if (ECCmdPortNum == EC_INDEXIO_PORT) {
        CompalECIndexCmdRead(ECCmd,ECDataLen,ECRetrunDataLen,ECReturnBufPTR);
    } else if (ECCmdPortNum == EC_MEMORY_MAPPING_PORT) {
        //[COM-START] Auto detect EC interface
        //Check memory mapping implement
        if ( CompalECMemoryMappingInterfaceCheck() ) {
            CompalECMemoryMappingCmdRead(ECCmd,ECDataLen,ECRetrunDataLen,ECReturnBufPTR);
        } else {  //use Index IO
            CompalECIndexCmdRead(ECCmd,ECDataLen,ECRetrunDataLen,ECReturnBufPTR);
        }
        //[COM-END]
    } else {
        while ( (ErrorStatus == EFI_TIMEOUT) && (RetryCnt < EC_CMD_RETRY_COUNT) ) {
            RetryCnt++;
            if ( ECCmdPortNum == EC_8042_CMD_PORT ) {
                EcDataPort   = EC_8042_DATA_PORT;
                EcStatusPort = EC_8042_STATUS_PORT;
            } else if ( ECCmdPortNum == EC_EXT_8042_CMD_PORT ) {
                EcDataPort   = EC_EXT_8042_DATA_PORT;
                EcStatusPort = EC_EXT_8042_STATUS_PORT;

                // Chk Busy Flag for 0x6C
                ErrorStatus = CompalECCheckBusyFlag(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            } else {
                EcDataPort   = EC_ACPI_DATA_PORT;
                EcStatusPort = EC_ACPI_STATUS_PORT;
            }

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            if ( ECCmdPortNum == EC_8042_CMD_PORT ) {
                IoWrite8(EC_8042_CMD_PORT,Disable_Keyboard_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;

                IoWrite8(EC_8042_CMD_PORT,Disable_Auxiliary_Device_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            }

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            IoWrite8(ECCmdPortNum,ECCmd);
            if ( ECDataLen != EC_NO_DATACMD_BYTE ) {
                for (Tmp1= 0; Tmp1 < ECDataLen; Tmp1++) {
                    ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                    if ( ErrorStatus == EFI_TIMEOUT ) break;

                    IoWrite8(EcDataPort, *(ECReturnBufPTR + Tmp1));
                }
            }

            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            for (Tmp1= 0; Tmp1 < ECRetrunDataLen; Tmp1++) {
                ErrorStatus = CompalECWaitOBF(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) break;

                Data = IoRead8(EcDataPort);
                *(ECReturnBufPTR + Tmp1) = Data;
            }

            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitOBFEmpty(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
            if ( ErrorStatus == EFI_TIMEOUT ) continue;

            if ( ECCmdPortNum == EC_8042_CMD_PORT ) {
                IoWrite8(EC_8042_CMD_PORT, Enable_Keyboard_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;

                IoWrite8(EC_8042_CMD_PORT, Enable_Auxiliary_Device_Interface);
                ErrorStatus = CompalECWaitECIBFClr(EcStatusPort);
                if ( ErrorStatus == EFI_TIMEOUT ) continue;
            }

            if ( ECCmdPortNum == EC_EXT_8042_CMD_PORT ) {
                IoWrite8(EC_EXT_8042_CMD_PORT,0xFF);
            }
        }
    }

    if ( RetryCnt >= EC_CMD_RETRY_COUNT ) return  EFI_TIMEOUT;

    return EFI_SUCCESS;
}

EFI_STATUS
CompalECWaitOBF (
    IN UINT8   ECStatusPortNum
)
{
    UINT8 Data;
    UINT16 RetryCnt;

    RetryCnt = 0;
    while ( RetryCnt++ < EC_OBF_SET_TIMEOUT_RETRY_COUNT ) {
        Data = IoRead8(ECStatusPortNum);
        if ( Data & EC_8042_STATUS_BIT_OBF ) return EFI_SUCCESS;

        CompalAcpiStall(EC_OBF_SET_WAIT_TIME);
    }
    return EFI_TIMEOUT;
}

EFI_STATUS
CompalECCheckBusyFlag (
    IN UINT8   ECStatusPortNum
)
{
    UINT16 RetryCnt;

    RetryCnt = 0;
    while ( RetryCnt++ < EC_OBF_EMPTY_TIMEOUT_RETRY_COUNT ) {
        if ( (IoRead8(ECStatusPortNum)) & EC_EXT_8042_BUSY_FLAG ) {
            CompalAcpiStall(EC_OBF_EMPTY_TIME);			//1ms
        } else {
            return EFI_SUCCESS;
        }
    }
    return EFI_TIMEOUT;
}

EFI_STATUS
CompalECWaitOBFEmpty (
    IN UINT8   ECStatusPortNum
)
{
    UINT8    EcDataPort;
    UINT8    Data;
    UINT16   RetryCnt;

    RetryCnt = 0;
    if ( ECStatusPortNum == EC_8042_CMD_PORT ) {
        EcDataPort = EC_8042_DATA_PORT;
    } else if (  ECStatusPortNum == EC_EXT_8042_CMD_PORT ) {
        return EFI_SUCCESS;
    } else {
        EcDataPort = EC_ACPI_DATA_PORT;
    }

    while ( RetryCnt++ < EC_OBF_EMPTY_TIMEOUT_RETRY_COUNT ) {
        if ( (IoRead8(ECStatusPortNum)) & EC_8042_STATUS_BIT_OBF ) {
            Data = IoRead8(EcDataPort);
            CompalAcpiStall(EC_OBF_EMPTY_TIME);			//1ms
        } else {
            return EFI_SUCCESS;
        }
    }
    return EFI_TIMEOUT;
}

EFI_STATUS
CompalECWaitECIBFClr(
    IN UINT8   ECStatusPortNum
)
{
    UINT8   Data;
    UINT16  RetryCnt = 0;

    RetryCnt = 0;
    while (RetryCnt++ < EC_IBF_CLEAR_TIMEOUT_RETRY_COUNT) {
        Data = IoRead8(ECStatusPortNum);
        if ( !(Data & EC_8042_STATUS_BIT_IBF) ) {
            return EFI_SUCCESS;
        }
        CompalAcpiStall(EC_OBF_EMPTY_TIME);			//1ms
    }
    return EFI_TIMEOUT;
}

UINT8
CompalECACPINVSReadByte(
    IN UINT8   NVRAMOffset
)
{
    UINT8   NVRAMReadData;
    UINTN   NVRAMBase;

    if ( CompalECMemoryMappingInterfaceCheck() ) {
        NVRAMBase = ECRAM_BASE + NVRAMOffset;
        NVRAMReadData = CompalECMemoryMappingRead(NVRAMBase);
    } else {
        NVRAMReadData = CompalECACPINVSReadByteByIndexIO(NVRAMOffset);
    }

    return NVRAMReadData;
}

UINT8
CompalECACPINVSReadByteByIndexIO(
    IN UINT8   NVRAMOffset
)
{
    UINT8 OutData;

    CompalECReadENEData((ECRAM_LO+NVRAMOffset),ECRAM_HIGH,&OutData);
    return OutData;
}


VOID
CompalECACPINVSWriteByteByIndexIO(
    IN UINT8   NVRAMOffset,
    IN UINT8   NVRAMData
)
{
    CompalECWriteENEData((ECRAM_LO+NVRAMOffset),ECRAM_HIGH,NVRAMData);
}

VOID
CompalECACPINVSWriteByte(
    IN UINT8   NVRAMOffset,
    IN UINT8   NVRAMData
)
{
    UINTN NVRAMBase;

    if ( CompalECMemoryMappingInterfaceCheck() ) {
        NVRAMBase = ECRAM_BASE + NVRAMOffset;
        CompalECMemoryMappingWrite(NVRAMBase,NVRAMData);
    } else {
        CompalECACPINVSWriteByteByIndexIO(NVRAMOffset,NVRAMData);
    }
}

UINT8
CompalECCrisisStatus(
    VOID
)
{
    UINT8	CrisisStatus;

    CrisisStatus = EC_CMD_GET_EC_INFORMATION_CRISIS_STATUS;
    CompalECReadCmd(EC_MEMORY_MAPPING_PORT,EC_CMD_GET_EC_INFORMATION,01,01,&CrisisStatus);
    if (CrisisStatus == 0x80) {
        return  TRUE;
    } else  {
        return  FALSE;
    }
}


VOID
CompalECStall(
    IN UINTN    Microseconds
)
{
    UINT8    Data8;
    UINTN    Ticks;

    Ticks = ((Microseconds * 10) / EC_8254_METRONOME_TICK_PERIOD) + 1;

    if (((Microseconds * 10) % EC_8254_METRONOME_TICK_PERIOD) > 0) {
        Ticks++;
    }
    for (; Ticks != 0x00; Ticks--) {
        do {
            Data8 = IoRead8(0x61);
            Data8 = Data8 & 0x10;
        } while (Data8 == 0);

        do {
            Data8 = IoRead8(0x61);
            Data8 = Data8 & 0x10;
        } while (Data8 != 0);
    }
}

EFI_STATUS
CompalECWriteEDIDIndex (
    IN OUT UINT8  *InputBuffer
)
{
    CompalECWriteCmd (EC_MEMORY_MAPPING_PORT, EC_CMD_EDID_INDEX, EC_CMD_EDID_INDEX_DATA_LEN, InputBuffer);

    return  EFI_SUCCESS;
}

EFI_STATUS
CompalECReadKBCVersion (
    IN OUT UINT8  *InputBuffer
)
{
    CompalECReadCmd (EC_MEMORY_MAPPING_PORT,EC_CMD_BIOS_REVISION,EC_CMD_BIOS_REVISION_CMD_DATA_LEN,EC_CMD_BIOS_REVISION_RETURN_DATA_LEN,InputBuffer);
    InputBuffer[2] &= 0x0F;

    return  EFI_SUCCESS;
}

BOOLEAN
CompalECCheeckPowerState (
    VOID
)
{
    UINT8       PortDataOut;

//
//  ADPT, 1,        // AC Adapter (0=OffLine, 1=OnLine)     ; A3h.7
//
    PortDataOut = CompalECACPINVSReadByte(0xA3);

    if (PortDataOut & 0x80) {
        return TRUE;
    } else {
        return FALSE;
    }
}


#define GET_DEBUG_MODE_CMD_LENGTH    2
#define GET_DEBUG_RETURN_DATA_LENGTH 1

VOID
CompalEcGetDebugMode (
    IN OUT UINT8  *Mode
)
{
    UINT8 InputBuffer[2];
    InputBuffer[0] = ECDebugModeCmd;
    InputBuffer[1] = EcGetDebugModeCmd;
    CompalECReadCmd(EC_MEMORY_MAPPING_PORT,EC_SYSTEM_NOTIFI_CMD,GET_DEBUG_MODE_CMD_LENGTH,GET_DEBUG_RETURN_DATA_LENGTH,InputBuffer);
    *Mode = InputBuffer[0];
}

VOID
CompalEcSetDebugMode (
    IN UINT8   ECCmdPortNum,
    IN UINT8   Mode
)
{
    CompalECWriteCmdTwoByte (ECCmdPortNum,EC_SYSTEM_NOTIFI_CMD,ECDebugModeCmd,TurnOnDebug);
    CompalECWriteCmdTwoByte (ECCmdPortNum,EC_SYSTEM_NOTIFI_CMD,ECDebugModeCmd,Mode);
}

VOID
CompalEcSendDebugBufferCmd (
    UINT8 DataLength,
    IN OUT UINT8  *InputBuffer
)
{
    CompalECWriteCmd ( EC_MEMORY_MAPPING_PORT,ECDebugMsgCmd,DataLength,&InputBuffer[0]);
}


EFI_STATUS
CompalECReadKBCVersionByIndexIO(
    IN OUT UINT8  *InputBuffer
)
{
    CompalECReadENEData (0x01, 0x40, &InputBuffer[0]);
    CompalECReadENEData (0x02, 0x40, &InputBuffer[1]);
    CompalECReadENEData (0x04, 0x40, &InputBuffer[2]);

    return  EFI_SUCCESS;
}

//[COM-START] Auto redirect to Write data byte either of memory mapping interface(default) or index I/O interface.
VOID
CompalECMemoryWrite (
    IN  UINTN  Offset,
    IN  UINT8  WriteData
)
{
    if ( CompalECMemoryMappingInterfaceCheck() ) {
        CompalECMemoryMappingWrite(Offset, WriteData);
    } else {
        CompalECIndexIOWrite(Offset, WriteData);
    }
}
//[COM-END]

//[COM-START] Auto redirect to read data byte either of memory mapping interface(default) or index I/O interface.
UINT8
CompalECMemoryRead(
    IN UINTN	Offset
)
{
    if ( CompalECMemoryMappingInterfaceCheck() ) {
        return CompalECMemoryMappingRead(Offset);
    } else {
        return CompalECIndexIORead(Offset);
    }
}
//[COM-END]
