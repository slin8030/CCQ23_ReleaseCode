/** @file

   The definition of DEBUG message Variable.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _DEBUG_MESSAGE_VARIABLE_H_
#define _DEBUG_MESSAGE_VARIABLE_H_


//
// Define Dump DEBUG() Message ('_DDMG_') signature
//
#define DUMP_DEBUG_MESSAGE_INSTANCE_SIGNATURE   SIGNATURE_64('_', 'D', 'D', 'M', 'G', '_', 0, 0)
//[-start-170413-IB08400444-add]//
#define SMM_DMSC_COMMUNICATION_BUFFER_SIZE      (OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof (SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE))
#define SMM_COMM_UNREGISTER_HANDLER             0x01
//[-end-170413-IB08400444-add]//

//
// Ensure proper structure formats
//
#pragma pack(1)

//[-start-180518-IB08400595-modify]//
typedef struct {
  UINT64                  DumpDebugMessageSignature; // '_DDMG_'"
  EFI_PHYSICAL_ADDRESS    DumpDebugMessageHeadAddr;  // Debug Message Memory buffer Entry Address
} DEBUG_MESSAGE_MEMORY_ADDR;
//[-end-180518-IB08400595-modify]//

typedef struct {
  UINT64                  DumpDebugMessageSignature; // '_DDMG_'"
  UINT32                  TotalSize;                 // Total size of the memory
  UINT32                  Reserved;
  UINT32                  AdmgStartAddr;             // ASCII code DEBUG Message Start Address
  UINT32                  AdmgEndAddr;               // ASCII code DEBUG Message End Address
  UINT32                  AdmgSize;                  // Size of ASCII code DEBUG Message   
  UINT32                  AdmgReserved;
} DEBUG_MESSAGE_HEAD_STRUCTURE;

typedef struct {
  UINT32                  AdmgStartAddr;             // ASCII code DEBUG Message Start Address
  UINT32                  AdmgEndAddr;               // ASCII code DEBUG Message End Address
  UINT32                  AdmgSize;                  // Size of ASCII code DEBUG Message   
  UINT32                  AdmgReserved;
} PEI_DEBUG_MESSAGE_HEAD_STRUCTURE;

//[-start-170413-IB08400444-add]//
typedef struct {
  UINTN                          Function;
  EFI_STATUS                     ReturnStatus;
  BOOLEAN                        UnregisterService;             
} SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE;
//[-end-170413-IB08400444-add]//

#pragma pack()

extern EFI_GUID gH2ODebugMessageVariableGuid;
//[-start-170413-IB08400444-add]//
extern EFI_GUID gH2ODebugMessageStatusCodeCommunicationGuid;
//[-end-170413-IB08400444-add]//

#endif
