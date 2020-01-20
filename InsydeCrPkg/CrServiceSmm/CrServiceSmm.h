/** @file
  Header file for Console redirection SMM drvier
;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _CR_SERVICE_SMM_H_
#define _CR_SERVICE_SMM_H_

#pragma pack (1)

typedef union _CR_SMM_PARAMETER {
  UINT32    Reg32;
  UINT8     Reg8;
  //
  //  EAX
  //
  UINT8   CrsSmi;
  //
  //  EBX
  //
  UINT32  Address;
  //
  //  ECX
  //
  struct _REG {
    UINT32  ReadWrite :1;   //0:Write 1:Read
    UINT32  PortType  :1;   //0:MMIO  1:IO
    UINT32  Reserve1  :6;
    UINT32  Data      :8;
    UINT32  Reserve2  :16;
  } Reg;
  //
  //  EDX
  //
  UINT16  SmiPort;
  //
  //  ESI
  //
  UINT32  Signature;


} CR_SMM_PARAMETER;

typedef struct _CR_REG_ARRAY {
  CR_SMM_PARAMETER         Eax;
  CR_SMM_PARAMETER         Ebx;
  CR_SMM_PARAMETER         Ecx;
  CR_SMM_PARAMETER         Edx;
  CR_SMM_PARAMETER         Esi;
} CR_REG_ARRAY;

#pragma pack ()

typedef enum {
  MEM_W     = 0,
  MEM_R     = 1,
  IO_W      = 2,
  IO_R      = 3,
  QUEUE_CMD = 4
} SMI_FUNC;

#endif

