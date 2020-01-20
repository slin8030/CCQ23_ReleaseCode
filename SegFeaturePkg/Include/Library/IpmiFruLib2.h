/** @file
 H2O IPMI FRU library header file.

 This file contains functions prototype that can easily access FUR data via
 using H2O IPMI FRU protocol.

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
//
// Server Feature - Insyde IPMI Package V1.0
//

#ifndef _IPMI_FRU_LIB2_H_
#define _IPMI_FRU_LIB2_H_

/**
 Read specified string/data based on field from FRU.

 @param[in]         FruId               FRU ID that want to read from.
 @param[in]         Field               Field id want to get.
 @param[in, out]    StringBufferSize    When in, the size of string buffer to store the result;
                                        When out, the real size of string buffer.
 @param[out]        StringBuffer        Buffer to store string/data.

 @retval EFI_INVALID_PARAMETER          StinrBufferSize or StringBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot get data.
 @retval EFI_BUFFER_TOO_SMALL           Buffer is smaller than return data size.
*/
EFI_STATUS
IpmiLibFruReadFieldData2 (
  IN     UINT8                          FruId,
  IN     UINT16                         Field,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT8                          *StringBufferSize,
  OUT    UINT8                          *StringBuffer
  );


/**
 Write specified string/data based on field to FRU.

 @param[in]         FruId               FRU ID that want to write to.
 @param[in]         Field               Field id want to write.
 @param[in, out]    StringBufferSize    When in, the size of string buffer to store the result;
                                        When out, the real size of write string buffer.
 @param[in]         StringBuffer        Buffer to store string/data.

 @retval EFI_INVALID_PARAMETER          StinrBufferSize or StringBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot write data.
 @retval EFI_BUFFER_TOO_SMALL           Buffer is smaller than write data size.
*/
EFI_STATUS
IpmiLibFruWriteFieldData2 (
  IN     UINT8                          FruId,
  IN     UINT16                         Field,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT8                          *StringBufferSize,
  IN     UINT8                          *StringBuffer
  );


/**
 Get Chassis Type value/string field in Chassis info area from FRU.

 @param[in]         FruId               FRU ID that want to read from.
 @param[in,out]     ChassisTypeStrSize  When in, the size of chassis type string in bytes to store the result;
                                        When out, the real size of string buffer in bytes.
 @param[out]        ChassisType         Enumeration value for chassis type field.
 @param[out]        ChassisTypeString   Chassis type string.

 @retval EFI_UNSUPPORTED                Common header didn't exist.
 @retval EFI_BUFFER_TOO_SMALL           Buffer is smaller than return data size.
*/
EFI_STATUS
IpmiLibFruGetChassisType2 (
  IN     UINT8                          FruId,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT8                          *ChassisTypeStrSize,
  OUT    UINT8                          *ChassisType,
  OUT    UINT8                          *ChassisTypeString
  );


/**
 Get EFI_TIME convert from MFG Date / Time field in Board area from FRU.

 @param[in]         FruId               FRU ID that want to read from.
 @param[out]        MfgDateTime         The EFI_TIME struct convert from MFG Date / Time field in Board Info Area.

 @retval EFI_SUCCESS                    Get MFG date and time successfully.
 @retval EFI_UNSUPPORTED                Get MFG date and time fail.
*/
EFI_STATUS
IpmiLibFruGetMfgDateTime2 (
  IN  UINT8                             FruId,
  OUT EFI_TIME                          *MfgDateTime,
//[-start-161121-IB09330314-add]//
  IN  BOOLEAN                           IsHeci
//[-end-161121-IB09330314-add]//
  );


/**
 Read data from specified record.

 @param[in]         FruId               FRU ID that want to read from.
 @param[in]         RecordType          Specified record type.
 @param[in]         RecordIndex         Index of specified record, if multiple instances, start from 0.
 @param[in, out]    DataBufferSize      IN:  Data buffer size that can store read data.
                                        OUT: Real read data.
 @param[out]        DataBuffer          Buffer to store read data.
 @param[out]        DataHeader          Record header of specified multi record.

 @retval EFI_INVALID_PARAMETER          DataBufferSize or DataBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot get data.
 @retval EFI_BUFFER_TOO_SMALL           Buffer is smaller than return data size.
 @retval EFI_SUCCESS                    Get success.
*/
EFI_STATUS
IpmiLibFruReadMultiRecord2 (
  IN     UINT8                          FruId,
  IN     UINT8                          RecordType,
  IN     UINTN                          RecordIndex,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT8                          *DataBufferSize,
  OUT    VOID                           *DataBuffer,
  OUT    VOID                           *DataHeader OPTIONAL
  );


/**
 Write data to specified record.

 @param[in]         FruId               FRU ID that want to write to.
 @param[in]         RecordType          Specified record type.
 @param[in]         RecordIndex         Index of specified record, if multiple instances, start from 0.
 @param[in, out]    DataBufferSize      IN:  Data buffer size that want to write;
                                        OUT: Data size that can be written.
 @param[out]        DataBuffer          Buffer to store write data.

 @retval EFI_INVALID_PARAMETER          DataBufferSize or DataBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot write data.
 @retval EFI_BUFFER_TOO_SMALL           Record buffer is smaller than write data.
 @retval EFI_SUCCESS                    Get success.
*/
EFI_STATUS
IpmiLibFruWriteMultiRecord2 (
  IN     UINT8                          FruId,
  IN     UINT8                          RecordType,
  IN     UINTN                          RecordIndex,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT8                          *DataBufferSize,
  OUT    VOID                           *DataBuffer
  );


/**
 Read specified size of raw data from specified offset.

 @param[in]         FruId               FRU ID that want to read from.
 @param[in]         Offset              Offset of FRU that user wants to read from.
 @param[in, out]    DataBufferSize      IN:  Data buffer size. It also means data size that want to read.
                                        OUT: FRU data size that can be read from specfied offset.
 @param[out]        DataBuffer          Buffer to store read data.

 @retval EFI_INVALID_PARAMETER          DataBufferSize or DataBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot get data.
 @retval EFI_OUT_OF_RESOURCES           If Offset is larger then total FRU size.
 @retval EFI_BUFFER_TOO_SMALL           If Offset + DataBufferSize is larger then total FRU size.
 @retval EFI_SUCCESS                    Get success.
*/
EFI_STATUS
IpmiLibFruReadRawData2 (
  IN     UINT8                          FruId,
  IN     UINT16                         Offset,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT16                         *DataBufferSize,
  OUT    VOID                           *DataBuffer
  );


/**
 Write specified size of raw data to specified offset.

 @param[in]         FruId               FRU ID that want to write to.
 @param[in]         Offset              Offset of FRU that user wants to write to.
 @param[in, out]    DataBufferSize      IN:  Data buffer size that want to write.
                                        OUT: FRU data size that can be written from specfied offset.
 @param[out]        DataBuffer          Buffer to store read data.

 @retval EFI_INVALID_PARAMETER          DataBufferSize or DataBuffer is NULL.
 @retval EFI_UNSUPPORTED                Cannnot get data.
 @retval EFI_OUT_OF_RESOURCES           If Offset is larger then total FRU size.
 @retval EFI_BUFFER_TOO_SMALL           If Offset + DataBufferSize is larger then total FRU size.
 @retval EFI_SUCCESS                    Get success.
*/
EFI_STATUS
IpmiLibFruWriteRawData2 (
  IN     UINT8                          FruId,
  IN     UINT16                         Offset,
//[-start-161121-IB09330314-add]//
  IN     BOOLEAN                        IsHeci,
//[-end-161121-IB09330314-add]//
  IN OUT UINT16                         *DataBufferSize,
  IN     VOID                           *DataBuffer
  );

#endif

