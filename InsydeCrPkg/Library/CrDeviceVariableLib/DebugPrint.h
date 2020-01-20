
#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

//
// DEBUG_ERROR > DEBUG_VERBOSE > DEBUG_INFO > DEBUG_WARN
//
#define D_LEVEL  DEBUG_INFO

#if 0
//
// D_DEBUG (DEBUG_INFO, "asdf <%a.%d> \n", __FUNCTION__, __LINE__);
// D_DEBUG (DEBUG_INFO, "asdf <%a> %s\n", __FUNCTION__,ConvertDevicePathToText ((const EFI_DEVICE_PATH *)CrDevice[Count].DevPath,FALSE, TRUE));
//
#define D_DEBUG(Level, ...)           \
          if (Level >= D_LEVEL) {    \
            AsciiPrint  (##__VA_ARGS__); \
          }

//
// D_DUMPHEX (DEBUG_WARN, 2, 0, VarSize, CrDeviceVar);
//
#define D_DUMPHEX(Level, Indent, Offset, DataSize, UserData)                \
          if (Level >= D_LEVEL) {             \
            DumpHex (Indent, Offset, DataSize, UserData);  \
          }

//
// D_ASSERT (!EFI_ERROR(Status));
//
#define D_ASSERT(Expression)                                                     \
          if (!(Expression)) {                                                   \
            Print (L"Assert <%a.%d> %a\n", __FUNCTION__, __LINE__, #Expression); \
          }
#else
#define D_DEBUG(...)                                          DEBUG ((##__VA_ARGS__))
#define D_DUMPHEX(Level, Indent, Offset, DataSize, UserData)  DumpHex (Indent, Offset, DataSize, UserData)
#define D_ASSERT(Expression)                                  ASSERT (Expression)
#endif

VOID
DumpHex (
  IN UINTN        Indent,
  IN UINTN        Offset,
  IN UINTN        DataSize,
  IN CONST VOID   *UserData
  );

#endif

