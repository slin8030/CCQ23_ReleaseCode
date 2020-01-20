/** @file

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

#ifndef _CHIPSET_OEM_BLOCK_DATA_H
#define _CHIPSET_OEM_BLOCK_DATA_H

#include <Base.h>
#include <Manifest.h>

#define CHIPSET_MANIFEST_BLOCK_SIG    SIGNATURE_32('$', 'F', 'U', 'D')
#define CHIPSET_STAGE2_HASH_BLOCK_SIG SIGNATURE_32('$', 'S', 'H', '2')
#define CHIPSET_FOTA_BLOCK_SIG        SIGNATURE_32('$', 'B', 'N', 'F')

#pragma pack(1)

//
// Chipset occupy 200 byte of 400 byte OEM Block
//   (Refer TXE BWG 6.3.1.1)
//

typedef struct _BIOS_MODULE_INFO{
  UINT32 SpiOffset;
  UINT32 Size;
}BIOS_MODULE_INFO;

typedef struct _UCODE_INFO{
  UINT32 SpiOffset;
  UINT32 Size;
}UCODE_INFO;

typedef struct {
  UINT32              Signature;          // Fixed $SH2
  UINT8               Stage2Hash[32];
} CHIPSET_STAGE2_HASH_BLOCK;

//
// FOTA, 44 bytes.
//
typedef struct {
  UINT32              Signature;          // $BNF

  BIOS_MODULE_INFO    StageTwoInfo;
  BIOS_MODULE_INFO    RecoveryModuleInfo;

  UCODE_INFO          UCodeInfo;
  UCODE_INFO          UCodeRecoveryInfo;

  UINT8               Rsvd[8];
} CHIPSET_FOTA_BLOCK;

typedef struct {
  MANIFEST_OEM_BLOCK          ManifestBlock;
  CHIPSET_STAGE2_HASH_BLOCK   Stage2HashBlock;
  CHIPSET_FOTA_BLOCK          FotaBlock;
} CHIPSET_OEM_BLOCK_DATA;

#pragma pack()

#endif

