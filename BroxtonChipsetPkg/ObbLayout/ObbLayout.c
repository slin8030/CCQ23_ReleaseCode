/** @file
  Generation of Flash layout information
;******************************************************************************
;* Copyright (c) 2012 - 2019, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <stdio.h>
#include <stdLib.h>
#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>

UINT8
PrintFlashLayoutInfo (
  IN FILE   *File,
  IN UINT32 ObbAddr1,
  IN UINT32 ObbSize1,
  IN UINT32 ObbAddr2,
  IN UINT32 ObbSize2
  )
{
  UINT32 CurAddr;
  UINT32 CurSize;
  UINT32 PartitionSize;
  UINT32 StartAddr;

  if (ObbAddr2 != 0) {
    StartAddr = ObbAddr2;
    PartitionSize = ObbSize2;
  } else {
    StartAddr = ObbAddr1;
    PartitionSize = ObbSize1;
  }

  CurAddr = StartAddr;
  fprintf(File, "                    \n");
  fprintf(File, "    OBB Partition   \n");
  fprintf(File, "                    \n");
  fprintf(File, "Start (hex)    End (hex)    Length (hex)    Area Name\n");
  fprintf(File, "-----------    ---------    ------------    ---------\n");
  CurSize = 0x1000;
  fprintf(File, "%08x       %08x     %08x        OBBP Sub Partition\n", CurAddr, CurAddr + CurSize - 1, CurSize);
  CurAddr += CurSize;
  CurSize = 0x1000;
  fprintf(File, "%08x       %08x     %08x            FDM region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
  CurAddr += CurSize;
  if (PcdGet32(PcdFlashNvStorageBvdtSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageBvdtSize);
    fprintf(File, "%08x       %08x     %08x            BVDT region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
  if (PcdGet32(PcdFlashNvStorageDmiSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageDmiSize);
    fprintf(File, "%08x       %08x     %08x            DMI region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
  if (PcdGet32(PcdFlashNvStorageMsdmDataSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageMsdmDataSize);
    fprintf(File, "%08x       %08x     %08x            MSDM region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
//  if (PcdGet32(PcdFlashNvStorageVariableDefaultsSize) != 0) {
//    CurSize = PcdGet32(PcdFlashNvStorageVariableDefaultsSize);
//    fprintf(File, "%08x       %08x     %08x            VarDef region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
//    CurAddr += CurSize;
//  }
//  if (PcdGet32(PcdFlashPciOptionRomSizeRvp) != 0) {
//    CurSize = PcdGet32(PcdFlashPciOptionRomSizeRvp);
//    fprintf(File, "%08x       %08x     %08x            OpROM region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
//    CurAddr += CurSize;
//  }
  if (PcdGet32(PcdFlashFvMainSize) != 0) {
    CurSize = PcdGet32(PcdFlashFvMainSize);
    if (((CurAddr + CurSize - StartAddr) > PartitionSize) && (ObbAddr2 != 0)) { // eMMC mode, split to two region
      CurSize = PcdGet32(PcdFlashFvMainSize) - ((CurAddr + CurSize - StartAddr) - PartitionSize);
      fprintf(File, "%08x       %08x     %08x            OBB(FvMain) region 1  \n", CurAddr, CurAddr + CurSize - 1, CurSize);
      CurAddr = ObbAddr1;
      CurSize = PcdGet32(PcdFlashFvMainSize) - CurSize;
      fprintf(File, "%08x       %08x     %08x            OBB(FvMain) region 2  \n", CurAddr, CurAddr + CurSize - 1, CurSize);
      CurAddr += CurSize;
    } else {
      fprintf(File, "%08x       %08x     %08x            OBB(FvMain) region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
      CurAddr += CurSize;
    }
  }
  if (PcdGet32(PcdFlashNvStorageVariableSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageVariableSize);
    fprintf(File, "%08x       %08x     %08x            NvStorage Region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
  if (PcdGet32(PcdFlashNvStorageFtwWorkingSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageFtwWorkingSize);
    fprintf(File, "%08x       %08x     %08x            FtwWork Region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
  if (PcdGet32(PcdFlashNvStorageFtwSpareSize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageFtwSpareSize);
    fprintf(File, "%08x       %08x     %08x            FtwBackup Region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
  if (PcdGet32(PcdFlashNvStorageFactoryCopySize) != 0) {
    CurSize = PcdGet32(PcdFlashNvStorageFactoryCopySize);
    fprintf(File, "%08x       %08x     %08x            FactoryCopy Region    \n", CurAddr, CurAddr + CurSize - 1, CurSize);
    CurAddr += CurSize;
  }
#if 0
  //
  // OBB Region Layout (Refer Base 0x00)
  //
  CurAddr = BaseAddr;
  fprintf(File, "    OBB Partition   \n");
  fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
  fprintf(File, "  | OBBP Sub Par.  |    (0x%08x)\n", 0x1000);
  CurAddr += 0x1000;
  fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
  fprintf(File, "  |   FDM region   |    (0x%08x)\n", 0x1000);
  CurAddr += 0x1000;
  if (PcdGet32(PcdFlashNvStorageBvdtSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |  BVDT region   |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageBvdtSize));
    CurAddr += PcdGet32(PcdFlashNvStorageBvdtSize);
  }
  if (PcdGet32(PcdFlashNvStorageDmiSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |   DMI region   |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageDmiSize));
    CurAddr += PcdGet32(PcdFlashNvStorageDmiSize);
  }
  if (PcdGet32(PcdFlashNvStorageMsdmDataSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |  MSDM region   |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageMsdmDataSize));
    CurAddr += PcdGet32(PcdFlashNvStorageMsdmDataSize);
  }
  if (PcdGet32(PcdFlashNvStorageVariableDefaultsSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |  VarDef region |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageVariableDefaultsSize));
    CurAddr += PcdGet32(PcdFlashNvStorageVariableDefaultsSize);
  }
  if (PcdGet32(PcdFlashPciOptionRomSizeRvp) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |  OpROM region  |    (0x%08x)\n", PcdGet32(PcdFlashPciOptionRomSizeRvp));
    CurAddr += PcdGet32(PcdFlashPciOptionRomSizeRvp);
  }
  if (PcdGet32(PcdFlashFvMainSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |  OBB (FvMain)  |    (0x%08x)\n", PcdGet32(PcdFlashFvMainSize));
    CurAddr += PcdGet32(PcdFlashFvMainSize);
  }
  if (PcdGet32(PcdFlashNvStorageVariableSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |NvStorage Region|    (0x%08x)\n", PcdGet32(PcdFlashNvStorageVariableSize));
    CurAddr += PcdGet32(PcdFlashNvStorageVariableSize);
  }
  if (PcdGet32(PcdFlashNvStorageFtwWorkingSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  | FtwWork Region |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageFtwWorkingSize));
    CurAddr += PcdGet32(PcdFlashNvStorageFtwWorkingSize);
  }
  if (PcdGet32(PcdFlashNvStorageFtwSpareSize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  |FtwBackup Region|    (0x%08x)\n", PcdGet32(PcdFlashNvStorageFtwSpareSize));
    CurAddr += PcdGet32(PcdFlashNvStorageFtwSpareSize);
  }
  if (PcdGet32(PcdFlashNvStorageFactoryCopySize) != 0) {
    fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
    fprintf(File, "  | FactoryCopy RG |    (0x%08x)\n", PcdGet32(PcdFlashNvStorageFactoryCopySize));
    CurAddr += PcdGet32(PcdFlashNvStorageFactoryCopySize);
  }

  fprintf(File, "  ================== <-- 0x%08x\n", CurAddr);
#endif  
  return 0;
}

/*
  FlashLayout utillity to generate FdLayout.h automatically

  @param[in] Argc               Number of command line arguments
  @param[in] Argv               Array of command line argument strings
  @param[in] Envp               Array of environmemt variable strings


  @retval 0                     Normal exit
  @retval others                Abnormal exit
*/
int
main (
  IN  INTN  Argc,
  IN  CHAR8 **Argv,
  IN  CHAR8 **Envp
  )
{

  FILE    *File = NULL;
  CHAR8   FilePath[256];
  UINT32  ObbAddr1;
  UINT32  ObbSize1;
  UINT32  ObbAddr2;
  UINT32  ObbSize2;

  AsciiSPrint(FilePath, 256, "OBBLayout.h", Argv[1]);
  File = fopen(FilePath, "w");
  if (File == NULL) {
    printf("Create file failure!\n");
    return -1;
  }
  
  ObbAddr1 = 0;
  ObbSize1 = 0;
  ObbAddr2 = 0;
  ObbSize2 = 0;
  if (Argc == 3) {
    //
    // Argv[1] <-- ObbAddr1
    // Argv[2] <-- ObbSize1
    //
    if (Argv[1] != NULL) {
      ObbAddr1 = (UINT32)strtol(Argv[1], NULL,16);
    }
    if (Argv[2] != NULL) {
      ObbSize1 = (UINT32)strtol(Argv[2], NULL,16);
    }
  }
  
  if (Argc == 5) {
    //
    // Argv[1] <-- ObbAddr1
    // Argv[2] <-- ObbSize1
    // Argv[3] <-- ObbAddr2
    // Argv[4] <-- ObbSize2
    //
    if (Argv[1] != NULL) {
      ObbAddr1 = (UINT32)strtol(Argv[1], NULL,16);
    }
    if (Argv[2] != NULL) {
      ObbSize1 = (UINT32)strtol(Argv[2], NULL,16);
    }
    if (Argv[3] != NULL) {
      ObbAddr2 = (UINT32)strtol(Argv[3], NULL,16);
    }
    if (Argv[4] != NULL) {
      ObbSize2 = (UINT32)strtol(Argv[4], NULL,16);
    }
  }
  
  PrintFlashLayoutInfo(File, ObbAddr1, ObbSize1, ObbAddr2, ObbSize2);

  fclose(File);

  return 0;
}