/** @file
  Generation of Flash layout information
;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <stdio.h>
#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>

#define PRINT_FLASH_LAYOUT_PCD(FdLayoutPcd)                \
          if (PcdGet32(FdLayoutPcd) != 0) {                \
            fprintf(FileIa32, "#define _PCD_VALUE_");                 \
            fprintf(FileX64, "#define _PCD_VALUE_");                 \
            fprintf(FileIa32, #FdLayoutPcd);                          \
            fprintf(FileX64, #FdLayoutPcd);                          \
            fprintf(FileIa32, "  0x%08XU\n", PcdGet32(FdLayoutPcd)); \
            fprintf(FileX64, "  0x%08XU\n", PcdGet32(FdLayoutPcd)); \
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

  FILE *FileIa32 = NULL;
  FILE *FileX64 = NULL;
  CHAR8 Ia32FilePath[256];
  CHAR8 X64FilePath[256];

  AsciiSPrint(Ia32FilePath, 256, "%a/IA32/FdLayout.h", Argv[1]);
  AsciiSPrint(X64FilePath, 256, "%a/X64/FdLayout.h", Argv[1]);
  FileIa32 = fopen(Ia32FilePath, "w");
  if (FileIa32 == NULL) {
    printf("Create file failure!\n");
    return -1;
  }

  FileX64 = fopen(X64FilePath, "w");
  if (FileX64 == NULL) {
    printf("Create file failure!\n");
    fclose(FileIa32);
    return -1;
  }
$(PcdDefinitions)

  fclose(FileIa32);
  fclose(FileX64);

  return 0;
}