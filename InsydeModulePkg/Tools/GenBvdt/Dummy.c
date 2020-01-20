/** @file
  Dummy C file

;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifdef _MSC_VER
  UINT16          CompilerVersion[6] = {0x5f24,0x534d,0x5f43,0x4556,0x3d52,_MSC_VER};
#else
  UINT16          CompilerVersion[6] = {0x5f24,0x534d,0x5f43,0x4556,0x3d52,0};
#endif

/**
  Main entry point for Windows program

  @param Argc   Number of command line arguments
  @param Argv   Array of command line argument strings

  @retval 0       Normal exit
*/
int
main (
  IN  int  Argc,
  IN  char **Argv
  )
{
  UINT16          DummyCompilerVersion;

  //
  // Do not remove, it is for compiler optimization
  //
  DummyCompilerVersion = CompilerVersion[0];
  return 0;
}