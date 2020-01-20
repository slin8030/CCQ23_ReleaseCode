//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/


#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
// #include <Library/FileHandleLib.h>

#include <Protocol/AbsolutePointer.h>
#include <Protocol/SimpleFileSystem.h>
#include "VkKeyFont.h"

#define COLOR_WHITE         0x00FFFFFF
#define COLOR_RED           0x00FF0000
#define LINE_WIDTH          1
#define FONT_BORDER         2
#define KEY_MAPPING_FILE    L"VK_MAP.h"


#define NULL_TOUCH_POINT    0x80000000
#define VIRTUAL_STD_KEY_SET 0x00000001
#define VIRTUAL_NUM_KEY_SET 0x00000010

typedef struct {
  //
  // Dis_X? means diaplay location
  //
  UINT16  Dis_Startx;  
  UINT16  Dis_Starty;  
  UINT16  Dis_EndX;  
  UINT16  Dis_Endy;  

  //
  // TouP_X? means Touch panel location
  //

  UINT16  TouP_Startx;  
  UINT16  TouP_Starty;  
  UINT16  TouP_EndX;  
  UINT16  TouP_Endy;  

  VOID    *MappingKeyPtr;
  VOID    *MapingPage1Ptr;  
  VOID    *MapingPage2Ptr;  
  VOID    *MapingPage3Ptr;  

  //
  // Toiuch flag
  //

  // 0x80000000 is null point 
  UINT32  Flag;
} POINT_INFO;


#define PNPID_DEVICE_PATH_NODE(PnpId) \
  { \
    { \
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      { \
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8) \
      }, \
    }, \
    EISA_PNP_ID((PnpId)), \
    0 \
  }

#define gPciRootBridge \
  PNPID_DEVICE_PATH_NODE(0x0A03)
 
 
#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      END_DEVICE_PATH_LENGTH, \
      0 \
    } \
  }
 
#define PCI_DEVICE_PATH_NODE(Func, Dev) \
  { \
    { \
      HARDWARE_DEVICE_PATH, \
      HW_PCI_DP, \
      { \
        (UINT8) (sizeof (PCI_DEVICE_PATH)), \
        (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8) \
      } \
    }, \
    (Func), \
    (Dev) \
  }

#define USB_DEVICE_PATH_NODE(Parent, Interface) \
  { \
    { \
      MESSAGING_DEVICE_PATH, \
      MSG_USB_DP, \
      { \
        (UINT8) (sizeof (USB_DEVICE_PATH)), \
        (UINT8) ((sizeof (USB_DEVICE_PATH)) >> 8) \
      } \
    }, \
    (Parent), \
    (Interface) \
  }
 
#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      END_DEVICE_PATH_LENGTH, \
      0 \
    } \
  }
 
#define END_DEVICE_PATH_LENGTH               (sizeof (EFI_DEVICE_PATH_PROTOCOL))

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_DEVICE_PATH;
 
PLATFORM_PCI_DEVICE_PATH gUsb0DevPath = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x14),
  gEndEntire
};

typedef struct _BLK_INFO BLK_INFO;

struct _BLK_INFO {
  UINT16        MaxRowNum;
  BLK_INFO      *NextPtr;
  POINT_INFO    *PointPtr;
};


typedef struct {
  EFI_ABSOLUTE_POINTER_PROTOCOL *TouchProtocol;
  EFI_HANDLE                    TouchHandle;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GfxProtocol;
  EFI_HANDLE         GfxHandle;
  UINT32             MaxH_Line;
  UINT32             MaxV_Line;
  BLK_INFO           *MapInfoPtr;
  UINT32             MaxNumOfBlkInfo;
  EFI_FILE_HANDLE    FileHandle;
  EFI_FILE_PROTOCOL  *RootFileProtocol;
} VK_APP_STRUCT;


VK_APP_STRUCT    mVkInfo;

EFI_STATUS
DrawingLine( 
  IN  VK_APP_STRUCT                *VkInfo,
  OUT UINT32                       *MaxH_Line,
  OUT UINT32                       *MaxV_Line
  ) 
{ 
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_GRAPHICS_OUTPUT_PROTOCOL         *GfxProtocolptr;
  EFI_ABSOLUTE_POINTER_MODE            *TouchModeInfo;
  EFI_STATUS Status;
  UINTN      StartX;
  UINTN      StartY;
  UINTN      TStartX;
  UINTN      TStartY;
  UINTN      Range;
  UINTN      TRange;
  UINTN      MaxRangeY;
  UINTN      MaxRangeX;
  UINTN      PointX;
  UINTN      PointY;
  UINT32     White;
  UINT32     RowCount;
  UINT32     CloumCount;
  BLK_INFO   *PreBlkInfo;
  BLK_INFO   *BlkInfo;
  POINT_INFO *PointArryPtr;
  UINT32     DeltaX;
  UINT32     DeltaY;
  UINTN      TouchPointY;
  UINTN      TouchPointX;
  BOOLEAN    DivFlag;
  UINT64     AbsoluteMaxY;
  UINT64     AbsoluteMaxX;
  UINT64     Remainder;
 
  //
  // Variable initialize
  //
  PreBlkInfo = NULL;
  BlkInfo = NULL;
  *MaxH_Line = 0;
  *MaxV_Line = 0;
  DeltaX = 0;
  DeltaY = 0;
  TouchPointX = 0;
  TouchPointY = 0;
  CloumCount = 0;
  Remainder = 0;
  TRange = 0;
  TStartX = 0;

  GfxProtocolptr = VkInfo->GfxProtocol;
  Info = GfxProtocolptr->Mode->Info;
  Print (L"Info->HorizontalResolution: 0x%08x\n", Info->HorizontalResolution);
  Print (L"Info->VerticalResolution: 0x%08x\n", Info->VerticalResolution);


  DEBUG (( DEBUG_ERROR,"Info->HorizontalResolution: 0x%08x\n", Info->HorizontalResolution));
  DEBUG (( DEBUG_ERROR,"Info->VerticalResolution: 0x%08x\n", Info->VerticalResolution));
  //
  // Calculate the Delta value
  //
  //
  // Delta = (Display Max resolution) / (Touch Max resolution)
  //
  //
  // DisplayResolution / DisplayAddress = TouchResolution / TouchAddress(?)
  //
  // TouchAddress(?) = TouchResolution / DisplayResolution * DisplayAddress
  //
  // ;; 1 / Delta = TouchResolution / DisplayResolution
  //
  // TouchAddress(?) = DisplayAddress / Delta
  //
  TouchModeInfo = VkInfo->TouchProtocol->Mode;
  DEBUG (( DEBUG_ERROR,"TouchModeInfo->AbsoluteMaxX: 0x%08x\n", TouchModeInfo->AbsoluteMaxX));
  DEBUG (( DEBUG_ERROR,"TouchModeInfo->AbsoluteMaxY: 0x%08x\n", TouchModeInfo->AbsoluteMaxY));
  AbsoluteMaxX = TouchModeInfo->AbsoluteMaxX;
  AbsoluteMaxY = TouchModeInfo->AbsoluteMaxY;
  
  if (AbsoluteMaxX == 0xAAB) { 
    AbsoluteMaxX++;
  }

  if (AbsoluteMaxY == 0x5FF) { 
    AbsoluteMaxY++;
  }
  
  if (Info->HorizontalResolution > TouchModeInfo->AbsoluteMaxX) { 
    DivFlag = TRUE;
    DeltaX = (UINT32)DivU64x64Remainder((UINT64)Info->HorizontalResolution,
                                        AbsoluteMaxX,
                                        NULL
                                       );
  
    DeltaY = (UINT32)DivU64x64Remainder((UINT64)Info->VerticalResolution,
                                        AbsoluteMaxY,
                                        NULL
                                       );

  } else { 
    DivFlag = FALSE;
    DeltaX = (UINT32)DivU64x64Remainder(AbsoluteMaxX,
                                       (UINT64)Info->HorizontalResolution,
                                        NULL
                                       );
  
    DeltaY = (UINT32)DivU64x64Remainder(AbsoluteMaxY,
                                        (UINT64)Info->VerticalResolution,
                                        NULL
                                       );
  }
  Print (L"DivFlag: %x | DeltaX = 0x%08x | DeltaY = 0x%08x\n", DivFlag, DeltaX, DeltaY);
  DEBUG (( DEBUG_ERROR,"DivFlag: %x | DeltaX = 0x%08x | DeltaY = 0x%08x\n", DivFlag, DeltaX, DeltaY ));

  //
  // Calculate the every lines address  
  //
  StartY = Info->VerticalResolution % 12;
  StartY = StartY / 2;
  Range = Info->VerticalResolution / 12;
  MaxRangeY = Info->VerticalResolution - StartY;
  
//   TRange = AbsoluteMaxY / 12;
//   TStartY = (AbsoluteMaxY % 12) / 2;
  TRange = (UINTN)(UINT32)DivU64x64Remainder(AbsoluteMaxY,
                                             12,
                                             &Remainder
                                            );  
  
  TStartY = (UINTN)Remainder / 2;
  StartX = Info->HorizontalResolution % Range;
  StartX = StartX / 2;


  TStartX = (UINTN)(UINT32)DivU64x64Remainder(AbsoluteMaxX,
                                             TRange,
                                             &Remainder
                                            );  

  TStartX = (UINTN)Remainder / 2;
//   TStartX = TStartX / 2;
  
  MaxRangeX = Info->HorizontalResolution - StartX;

  RowCount = Info->VerticalResolution / (UINT32)Range;

  //
  // Create Mapping first node information
  //
  VkInfo->MapInfoPtr = (BLK_INFO*)AllocateZeroPool(sizeof(BLK_INFO));
//   VkInfo->MapInfoPtr->PointPtr = AllocateZeroPool(RowCount * sizeof(POINT_INFO));
//   VkInfo->MapInfoPtr->PointPtr = (POINT_INFO*)gBS->AllocatePool(RowCount * sizeof(POINT_INFO));
  gBS->AllocatePool(EfiBootServicesData,
                    RowCount * sizeof(POINT_INFO),
                    &VkInfo->MapInfoPtr->PointPtr
                   );
//   DEBUG (( DEBUG_ERROR, "VK: Allocate VkInfo->MapInfoPtr 0x%08x\n",VkInfo->MapInfoPtr ));
//   DEBUG (( DEBUG_ERROR, "VK: Allocate VkInfo->MapInfoPtr->PointPtr 0x%08x\n",VkInfo->MapInfoPtr->PointPtr ));
//   DEBUG (( DEBUG_ERROR, "VK: RowCount:%0d, Total byte:%d\n",RowCount, RowCount * sizeof(POINT_INFO)));


  PointArryPtr = VkInfo->MapInfoPtr->PointPtr;
  VkInfo->MapInfoPtr->MaxRowNum = (UINT16)RowCount;
  VkInfo->MapInfoPtr->NextPtr = NULL;

  //
  // Draw the horizontal line
  //
  White = COLOR_WHITE;

DEBUG (( DEBUG_ERROR, "VK: VkInfo->MapInfoPtr->PointPt addr 0x%08x\n",VkInfo->MapInfoPtr->PointPtr));
  for (PointX = StartX, PointY = StartY, TouchPointY = TStartY, RowCount = 0; 
                    PointY <= MaxRangeY; PointY += Range, TouchPointY += TRange, RowCount++) { 


    Print (L"Line S_X: 0x%08x, S_Y: 0x%08x)\n", StartX,PointY);
    Status = GfxProtocolptr->Blt ( GfxProtocolptr,
                                   (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&White,
                                   EfiBltVideoFill,
                                   0,//StartX,
                                   0,//StartY,
                                   StartX,//EndX,
                                   PointY,
                                   (MaxRangeX - StartX), //EndX - 10,
                                   LINE_WIDTH, // Line Width
                                   0 );

    *MaxH_Line += 1; 

    //
    // Put Information
    //
    if (RowCount != 0) { 
      if (RowCount < VkInfo->MapInfoPtr->MaxRowNum) { 
        PointArryPtr[RowCount].Dis_Starty = (UINT16)PointY;
        PointArryPtr[RowCount].TouP_Starty = (UINT16)TouchPointY;      
      }
  
      PointArryPtr[RowCount - 1].Dis_Endy = (UINT16)PointY;
      PointArryPtr[RowCount - 1].TouP_Endy = (UINT16)TouchPointY;      
      DEBUG (( DEBUG_ERROR, "VK: Allocate RowCount: %d :: PointArryPtr[RowCount - 1] addr 0x%08x\n",RowCount,&PointArryPtr[RowCount - 1]));
      DEBUG (( DEBUG_ERROR, "VK: Allocate PointArryPtr[RowCount] addr 0x%08x\n",&PointArryPtr[RowCount]));
    } else { 
      //
      // This is first line
      //
      PointArryPtr[RowCount].Dis_Starty = (UINT16)PointY;
      PointArryPtr[RowCount].TouP_Starty = (UINT16)TouchPointY;      
    } // if (RowCount != 0) { ... end this
    
//     if (RowCount <= VkInfo->MapInfoPtr->MaxRowNum) { 
// 
//       TouchPointY = PointY / DeltaY;
//       if (RowCount < VkInfo->MapInfoPtr->MaxRowNum) { 
//         PointArryPtr[RowCount].Dis_Starty = (UINT16)PointY;
//         PointArryPtr[RowCount].TouP_Starty = (UINT16)TouchPointY;
//       }
//   
//       if (RowCount != 0) { 
//         //
//         // This is not first node
//         //
//         PointArryPtr[RowCount - 1].Dis_Endy = (UINT16)PointY;
//         PointArryPtr[RowCount - 1].TouP_Endy = (UINT16)TouchPointY;
//         
//       } // if (PreBlkInfo == NULL) {  end this
//     } else { 
//       //
//       //  This is the end of line
//       //
//        
//       PointArryPtr[RowCount - 1].Dis_Endy = (UINT16)PointY;
//       PointArryPtr[RowCount - 1].TouP_Endy = (UINT16)TouchPointY;
// 
//     } // if (RowCount < VkInfo->MapInfoPtr->Max... { end this
    
  } // for (PointX = StartX, PointY = StartY..... { end this

  Print (L"RowCount: %d , MaxH_Line: %d\n", RowCount,*MaxH_Line);
  Print (L"VkInfo->MapInfoPtr->MaxRowNum:0x%08x \n", VkInfo->MapInfoPtr->MaxRowNum); 
  Print (L"PointY:0x%08x \n", PointY);
  if (PointY > MaxRangeY) { 
    PointY -= Range ;
  }
  Print (L"PointY:0x%08x \n", PointY);

  //
  // Draw the vertical line
  //

  White = COLOR_RED;

//  if ((MaxRangeX / Range) != (AbsoluteMaxX / TRange)) { 
//    TRange = (UINTN)(UINT32)(AbsoluteMaxX / (MaxRangeX / Range));
    TRange = (UINTN)(UINT32)DivU64x64Remainder (AbsoluteMaxX,
                                                (MaxRangeX / Range),
                                                &Remainder
                                                );  
//  TStartX = (UINTN)(UINT32)DivU64x64Remainder(AbsoluteMaxX,
//                                             TRange,
//                                             &Remainder
//                                            );  
//  }

  if ((MaxRangeX / Range) != DivU64x32 (AbsoluteMaxX, (UINT32)TRange)) {
    TRange = (UINTN)DivU64x32 (AbsoluteMaxX, (UINT32)(MaxRangeX / Range));
  }

#if 1
  TStartX = 0;
  for (PointX = StartX, CloumCount = 0, TouchPointX = TStartX ; PointX <= MaxRangeX; 
                                        PointX += Range, TouchPointX += TRange , CloumCount++) { 

//    if (CloumCount == 12)
//      break;

    Status = GfxProtocolptr->Blt ( GfxProtocolptr,
                                   (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&White,
                                   EfiBltVideoFill,
                                   0,//StartX,
                                   0,//StartY,
                                   PointX,//EndX,
                                   StartY,
                                   LINE_WIDTH, //EndX - 10,
                                   PointY, // Line Width
                                   0 );

  
    *MaxV_Line += 1; 

    //
    // Put Information
    //
    if (CloumCount != 0) { 

      //
      // Find the previous node
      //
      PreBlkInfo = VkInfo->MapInfoPtr;
      
      while (PreBlkInfo->NextPtr != NULL) { 
        PreBlkInfo = PreBlkInfo->NextPtr;
      }

      //
      // Fill end of X address in previous node
      //
      PointArryPtr = PreBlkInfo->PointPtr;

//       if (DivFlag == TRUE) { 
//         TouchPointX = PointX / DeltaX;
//       } else { 
//         TouchPointX = PointX * DeltaX;
//       }
//       TouchPointX = PointX * DeltaX;
//       TouchPointX = TStartX;
      Print(L" Check V_Line addr %08x \n", TouchPointX);
      DEBUG ((DEBUG_ERROR, "Check V_Line addr %08x \n\n", TouchPointX));

      for (RowCount = 0; RowCount < PreBlkInfo->MaxRowNum; RowCount++) { 
        PointArryPtr[RowCount].Dis_EndX = (UINT16)PointX;
        PointArryPtr[RowCount].TouP_EndX = (UINT16)TouchPointX;
        PointArryPtr[RowCount].MappingKeyPtr = NULL;
        PointArryPtr[RowCount].MapingPage1Ptr = NULL;
        PointArryPtr[RowCount].MapingPage2Ptr = NULL;
        PointArryPtr[RowCount].MapingPage3Ptr = NULL;
      }      

      //
      // if the [X address] + [Range] over the MaxRangeX,
      // it means this is the latest line did not need create node
      //
      if ((PointX + Range) <= MaxRangeX) { 
        //
        // Create node
        //
        BlkInfo = AllocateZeroPool(sizeof(BLK_INFO));
        
        //
        // Link to Previous node
        //
        PreBlkInfo->NextPtr = BlkInfo;
        BlkInfo->PointPtr = AllocateZeroPool(sizeof(POINT_INFO) * PreBlkInfo->MaxRowNum);
        DEBUG (( DEBUG_ERROR, "VK: Allocate PreBlkInfo 0x%08x\n",PreBlkInfo ));
        DEBUG (( DEBUG_ERROR, "VK: Allocate PreBlkInfo->NextPtr 0x%08x\n",PreBlkInfo->NextPtr ));
        DEBUG (( DEBUG_ERROR, "VK: Allocate BlkInfo 0x%08x\n",BlkInfo ));
        DEBUG (( DEBUG_ERROR, "VK: Allocate BlkInfo->PointPtr 0x%08x\n",BlkInfo->PointPtr ));
        
        //
        // Copy information
        //      
        CopyMem(BlkInfo->PointPtr, PreBlkInfo->PointPtr,(sizeof(POINT_INFO) * PreBlkInfo->MaxRowNum));
        
        //
        // Fill the node information
        //
        BlkInfo->NextPtr = NULL;
        BlkInfo->MaxRowNum = PreBlkInfo->MaxRowNum;
        PointArryPtr = BlkInfo->PointPtr;
      Print(L" Check Clum data Index:%02x] S_X:%08x ;; Pre_X:%08x \n",CloumCount,\
      TouchPointX,PreBlkInfo->PointPtr->TouP_Startx);

        for (RowCount = 0; RowCount < PreBlkInfo->MaxRowNum; RowCount++) { 
          PointArryPtr[RowCount].Dis_Startx = (UINT16)PointX;
          PointArryPtr[RowCount].TouP_Startx = (UINT16)TouchPointX;
          PointArryPtr[RowCount].MappingKeyPtr = NULL;
          PointArryPtr[RowCount].MapingPage1Ptr = NULL;
          PointArryPtr[RowCount].MapingPage2Ptr = NULL;
          PointArryPtr[RowCount].MapingPage3Ptr = NULL;

//       Print(L" Check Clum data Index:%02x] S_Y:%08x ;; E_Y:%08x \n",RowCount,\
//       PointArryPtr[RowCount].TouP_Starty,PointArryPtr[RowCount].TouP_Endy);
        }      
        
      } // if ((PointX + Range) < MaxRangeX) { end this
//       else { 
// 
//         //
//         // Debug....
//         //
//         Print(L" Check else  V_Line addr %08x Range %08x MaxRange %08x\n",PointX + Range,Range, MaxRangeX);
// 
//       }// if ((PointX + Range) < MaxRangeX) { end this else end
    } else { 
        
      //
      // This is first line, fill the X address info
      //
//       TouchPointX = PointX / DeltaX;
//       if (DivFlag == TRUE) { 
//         TouchPointX = PointX / DeltaX;
//       } else { 
//         TouchPointX = PointX * DeltaX;
//       }

      PointArryPtr = VkInfo->MapInfoPtr->PointPtr;

      for (RowCount = 0; RowCount < VkInfo->MapInfoPtr->MaxRowNum; RowCount++) { 
        PointArryPtr[RowCount].Dis_Startx = (UINT16)PointX;
        PointArryPtr[RowCount].TouP_Startx = (UINT16)TouchPointX;

//       Print(L" Check Clum data Index:%02x] S_Y:%08x ;; E_Y:%08x \n",RowCount,\
//       PointArryPtr[RowCount].TouP_Starty,PointArryPtr[RowCount].TouP_Endy);

      }
      
    } // if (CloumCount != 0) {  end this
    
  } // for (PointX = StartX, CloumCount = 0 ; PointX <=..... { end this
#endif 
  VkInfo->MaxNumOfBlkInfo = CloumCount;
  
  return EFI_SUCCESS;
}

EFI_STATUS
FreeBlackInfoMem( 
  IN VK_APP_STRUCT *VkMainPtr,
  IN BLK_INFO      *FreePtr
  ) 
{ 
  BLK_INFO      *PreviousPtr;

  //
  // Check parameter
  //
  if ((VkMainPtr == NULL) || (FreePtr == NULL)) { 
    return EFI_SUCCESS;
  }
  
  //
  // Catch the node previous line and next link
  //
  PreviousPtr = VkMainPtr->MapInfoPtr;

  if (PreviousPtr == NULL) { 
    return EFI_SUCCESS;
  }

  //
  // Check the node is head or not
  //
  if (VkMainPtr->MapInfoPtr == FreePtr) { 
    //
    // It is head node
    //

    //
    // Link data
    //
    VkMainPtr->MapInfoPtr = FreePtr->NextPtr;
      
    DEBUG (( DEBUG_ERROR, "VK: Free VkMainPtr->MapInfoPtr == FreePtr\n"));
    DEBUG (( DEBUG_ERROR, "VK: Free PreviousPtr->NextPtr 0x%08x\n",VkMainPtr->MapInfoPtr ));
    DEBUG (( DEBUG_ERROR, "VK: FreePtr 0x%08x\n",FreePtr ));
  } else { 
    //
    // It isn't head node and find out the previous node
    //      

    while (PreviousPtr->NextPtr != FreePtr) { 
      PreviousPtr = FreePtr->NextPtr;
      // NULL ??
    }

    //
    // Link data
    //
    PreviousPtr->NextPtr = FreePtr->NextPtr;
    DEBUG (( DEBUG_ERROR, "VK: Free PreviousPtr 0x%08x\n",PreviousPtr ));
    DEBUG (( DEBUG_ERROR, "VK: Free PreviousPtr->NextPtr 0x%08x\n",PreviousPtr->NextPtr ));
    DEBUG (( DEBUG_ERROR, "VK: FreePtr 0x%08x\n",FreePtr ));
    
  } // if (VkMainPtr->MapInfoPtr == FreePtr) { end this
   

  //
  // Free the Point node
  //
  DEBUG (( DEBUG_ERROR, "VK: FreePtr->PointPtr 0x%08x\n",FreePtr->PointPtr ));
  gBS->FreePool(FreePtr->PointPtr);
  FreePtr->MaxRowNum = 0;

  //
  // Free the current Black info
  //
  DEBUG (( DEBUG_ERROR, "VK:  FreePool(FreePtr) 0x%08x\n",FreePtr->PointPtr ));
  FreePool(FreePtr);
  
  return EFI_SUCCESS;
}

EFI_STATUS
FreeMapInfoMem( 
  IN VK_APP_STRUCT *VkMainPtr
  )
{ 
  
  //
  // Free BlackInfo Information
  //
  while (VkMainPtr->MapInfoPtr != NULL) { 
//     Print (L"VkMainPtr->MapInfoPtr: %08x\n", VkMainPtr->MapInfoPtr);
    FreeBlackInfoMem(VkMainPtr, VkMainPtr->MapInfoPtr);
    VkMainPtr->MaxNumOfBlkInfo--;
  }

  return EFI_SUCCESS;
}

#if 1
EFI_STATUS
GetTouchEvent( 
  IN   EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointerProtocol,
  IN   EFI_ABSOLUTE_POINTER_STATE    *TouchPoint,
  IN   UINTN                         TimeOutCount
  )
{ 

  EFI_STATUS Status;
  UINTN      Index;

  Index = 0;
  do { 
  	 Status = gBS->WaitForEvent(1, &AbsolutePointerProtocol->WaitForInput, &Index) ;
    TouchPoint->ActiveButtons = 0;
    if (Status == EFI_SUCCESS) { 
      Status = AbsolutePointerProtocol->GetState(AbsolutePointerProtocol,TouchPoint);
//       Print (L"2 X: %Lx Y: %Lx Button: %08x\n", TouchPoint->CurrentX, TouchPoint->CurrentY,TouchPoint->ActiveButtons);
      DEBUG ((DEBUG_ERROR,"2 X: %Lx Y: %Lx Button: %08x\n", TouchPoint->CurrentX, TouchPoint->CurrentY,TouchPoint->ActiveButtons));
      //
      // Finger release that is the latest point
      //
      if (TouchPoint->ActiveButtons == 0) { 
        return EFI_SUCCESS;
      }
    }
	  
  } while (--TimeOutCount != 0);

  return EFI_TIMEOUT;
}

EFI_STATUS
CheckMatchPoint( 
  IN   EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointerProtocol,
  IN   BLK_INFO                      *RootRowMapPtr,
  OUT  POINT_INFO                    **PointPtr
  )
{ 
  EFI_ABSOLUTE_POINTER_STATE  Touch;
  EFI_STATUS                  Status;
  POINT_INFO                  *CloumPtr;
  BLK_INFO                    *RowMapPtr;
  UINTN                       Index;  
  UINT16                      TouchX;
  UINT16                      TouchY;


  Status = GetTouchEvent(AbsolutePointerProtocol,&Touch,0);

  if (EFI_ERROR(Status)) { 
    return EFI_ABORTED;
  }

  TouchX = (UINT16)Touch.CurrentX;
  TouchY = (UINT16)Touch.CurrentY;
  
  DEBUG ((DEBUG_ERROR,"CheckMatchPoint:: TouchX: %08x TouchY: %08x \n", TouchX, TouchY));
  RowMapPtr = RootRowMapPtr;
  while (RowMapPtr != NULL) { 
    CloumPtr = RowMapPtr->PointPtr;
//     Print (L"CheckMatchPoint:: StartX: %08x EndY: %08x \n", CloumPtr[0].TouP_Startx, CloumPtr[0].TouP_EndX);
    DEBUG ((DEBUG_ERROR,"CheckMatchPoint:: StartX: %08x EndY: %08x \n", CloumPtr[0].TouP_Startx, CloumPtr[0].TouP_EndX));

    if ((CloumPtr[0].TouP_Startx < TouchX) && \
             (CloumPtr[0].TouP_EndX > TouchX)) { 
      for (Index = 0; Index < RowMapPtr->MaxRowNum; Index++) { 
//
       if ((CloumPtr[Index].TouP_Starty < TouchY) && \
                  (CloumPtr[Index].TouP_Endy > TouchY)) { 
          *PointPtr = &CloumPtr[Index];
          if ((CloumPtr[Index].Flag & NULL_TOUCH_POINT) != 0x00) { 
            return EFI_ACCESS_DENIED;
          }
          return EFI_SUCCESS;
        } // if ((CloumPtr[0].TouP_Starty < TouchX) && \.... end this
       
      } // for (Index = 0; Index....  end this

    } // if ((CloumPtr[Index].TouP_Startx < TouchX) && \....end this

    // find to next row block
    RowMapPtr = RowMapPtr->NextPtr; 
  } // while (RowMapPtr != NULL) { ..... end this

  return EFI_NOT_FOUND;
}

#if 1
EFI_STATUS
DrawCharacter (
  IN   EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput,
  IN   POINT_INFO                   *KeyInfoPtr
  )
{ 
  UINT32  KeyStartX;
  UINT32  KeyStartY;
  UINT32  CountRow;
  UINT32  CountColumn;
  UINT32  PixelHigh;
  UINT32  PixelWidth;
  UINT32     White;
  EFI_STATUS Status;
  VKMAP_DESC *FontDescPtr;


  White = COLOR_WHITE;
  FontDescPtr = (VKMAP_DESC*)KeyInfoPtr->MappingKeyPtr;

  //
  //    ______________
  //    |              }-------------------->  
  //    |   .<---------KeyStartX, KeyStartY
  //    |
  //    |
  //    |
  //    |
  //    |
  //    |
  //    |_____________
  //
  //
  //

  KeyStartX = KeyInfoPtr->Dis_Startx + FONT_BORDER;
  KeyStartY = KeyInfoPtr->Dis_Starty + FONT_BORDER;
  PixelWidth = ((KeyInfoPtr->Dis_EndX - FONT_BORDER) - KeyStartX) / FontDescPtr->Width;
  PixelHigh = ((KeyInfoPtr->Dis_Endy - FONT_BORDER) - KeyStartY) / FontDescPtr->High;

  DEBUG (( DEBUG_ERROR,"[KeyInfoPtr->End_X : 0x%08x ][ LineWidth : 0x%08x]\n",KeyInfoPtr->Dis_EndX, FONT_BORDER));
  DEBUG (( DEBUG_ERROR,"[PixelWidth : 0x%08x ][ PixelHigh : 0x%08x]\n",PixelWidth, PixelHigh));
  DEBUG (( DEBUG_ERROR,"[KeyStartX : 0x%08x ][ KeyStartY : 0x%08x]\n",KeyStartX,KeyStartY));

  for (CountRow = 0; CountRow < 19; CountRow ++, KeyStartY += PixelHigh) { 

    KeyStartX = KeyInfoPtr->Dis_Startx + FONT_BORDER;
    for (CountColumn = 0; CountColumn <  FontDescPtr->Width; CountColumn++, KeyStartX += PixelWidth) { 
      if (((FontDescPtr->BitMapPtr[CountRow] >> (FontDescPtr->Width - CountColumn))& 0x01) != 0) { 
 
  
        Status = GraphicsOutput->Blt ( GraphicsOutput,
                                       (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&White,
                                       EfiBltVideoFill,
                                       0,
                                       0,
                                       KeyStartX,
                                       KeyStartY,
                                       PixelWidth,
                                       PixelHigh,
                                       0 );
 
 
      } 

      
    }
  }
  
  
  return EFI_SUCCESS;
}
#endif

EFI_STATUS
MappingKeyAndTouch( 
  IN VK_APP_STRUCT *VkMainPtr
  )
{ 
  EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointerProtocol;
  EFI_ABSOLUTE_POINTER_STATE    UplTpoint;
  EFI_ABSOLUTE_POINTER_STATE    UprTpoint;
  EFI_ABSOLUTE_POINTER_STATE    lowlTpoint;
  EFI_ABSOLUTE_POINTER_STATE    lowrTpoint;
  VKMAP_DESC                    *KeyPtr;
  BLK_INFO                      *RowMapPtr;
  POINT_INFO                    *CloumPtr;
  EFI_STATUS                    Status;
  UINTN                         Index;
  UINT16                        MinYPoint;
  UINT16                        MaxYPoint;
  UINT16                        MinXPoint;
  UINT16                        MaxXPoint;
  UINT32                        SwitchKeyPageFlag;
  

  RowMapPtr = VkMainPtr->MapInfoPtr;

  KeyPtr = NULL;
  Index = 0;
  SwitchKeyPageFlag = 0;
  AbsolutePointerProtocol = VkMainPtr->TouchProtocol;


  Print(L"Please touch EFI_SHELL area upper left:");

  Status = GetTouchEvent(AbsolutePointerProtocol,&UplTpoint,0);
  
  Print (L"2 X: %Lx Y: %Lx Z: %Lx\n", UplTpoint.CurrentX, UplTpoint.CurrentY,UplTpoint.CurrentZ);

  Print(L"Please touch EFI_SHELL area upper right:");

  Status = GetTouchEvent(AbsolutePointerProtocol,&UprTpoint,0);
  Print (L"2 X: %Lx Y: %Lx Z: %Lx\n", UprTpoint.CurrentX, UprTpoint.CurrentY,UprTpoint.CurrentZ);

  Print(L"Please touch EFI_SHELL area lower left:");
  Status = GetTouchEvent(AbsolutePointerProtocol,&lowlTpoint,0);
  Print (L"2 X: %Lx Y: %Lx Z: %Lx\n", lowlTpoint.CurrentX, lowlTpoint.CurrentY,lowlTpoint.CurrentZ);

 
  Print(L"Please touch EFI_SHELL area lower right:");
  Status = GetTouchEvent(AbsolutePointerProtocol,&lowrTpoint,0);
  Print (L"2 X: %Lx Y: %Lx Z: %Lx\n", lowrTpoint.CurrentX, lowrTpoint.CurrentY,lowrTpoint.CurrentZ);

  //
  //
  //
  //         .<-------------------------}  Touch top point select minimum point
  //                                    } 
  //                       .<-----------}
  //
  //
  //
  //
  //                        .<----------}  select maximum point
  //       .<---------------------------}                 
  //
  //    
  MinYPoint = (UplTpoint.CurrentY < UprTpoint.CurrentY)?\
                        (UINT16)UplTpoint.CurrentY : (UINT16)UprTpoint.CurrentY;
  
  MinXPoint = (UplTpoint.CurrentX < lowlTpoint.CurrentX)?\
                        (UINT16)UplTpoint.CurrentX : (UINT16)lowlTpoint.CurrentX;

  MaxYPoint = (lowlTpoint.CurrentY > lowrTpoint.CurrentY)?\
                        (UINT16)lowlTpoint.CurrentY : (UINT16)lowrTpoint.CurrentY;
  
  MaxXPoint = (UprTpoint.CurrentX > lowrTpoint.CurrentX)?\
                        (UINT16)UprTpoint.CurrentX : (UINT16)lowrTpoint.CurrentX;
  //
  // Skip EFI SHELL area
  //
  while (RowMapPtr != NULL) {  
    CloumPtr = RowMapPtr->PointPtr;
    
    if (((CloumPtr->TouP_Startx < MinXPoint) && (CloumPtr->TouP_EndX < MinXPoint))\
        ||((CloumPtr->TouP_Startx > MaxXPoint) && (CloumPtr->TouP_EndX > MaxXPoint))) {  

    } else { 
      //
      // The Cloum is in range, now select row
      //
      for (Index = 0; Index < RowMapPtr->MaxRowNum; Index++) { 
         if (((CloumPtr[Index].TouP_Starty < MinYPoint) && (CloumPtr[Index].TouP_Endy < MinYPoint))\
              ||((CloumPtr[Index].TouP_Starty > MaxYPoint) && (CloumPtr[Index].TouP_Endy > MaxYPoint))) { 
//            Print (L"Isn't Shell Area Star_X: %04x\n", Index);
//            Print (L"Isn't Shell Area Star_X: %08x End_X: %08x\n", CloumPtr->TouP_Startx, CloumPtr[Index].TouP_EndX);
//            Print (L"Isn't Shell Area Star_Y: %08x End_Y: %08x\n", CloumPtr->TouP_Starty, CloumPtr[Index].TouP_Endy);
           
         } else { 
           //
           // This is EFI SHELL area, have to skip this area. 
           // So, set null flag
           //
           Print (L"Shell Area Star_X: %04x\n", Index);
           Print (L"Shell Area Star_X: %08x End_X: %08x\n", CloumPtr[Index].TouP_Startx, CloumPtr[Index].TouP_EndX);
           Print (L"Shell Area Star_Y: %08x End_Y: %08x\n", CloumPtr[Index].TouP_Starty, CloumPtr[Index].TouP_Endy);
           CloumPtr[Index].Flag = NULL_TOUCH_POINT;
         } 
      } //for (Index = 0; Index < RowMapPtr->MaxRowNum;..... end this
    }//     if ((CloumPtr->TouP_Startx > MinXPoint) &&\ ... end this

    // find to next row block
    RowMapPtr = RowMapPtr->NextPtr; 
  } // while (RowMapPtr != NULL) { ..... end this

//   while (RowMapPtr != NULL) { 
//     CloumPtr = RowMapPtr->PointPtr;
// 
//     if ((CloumPtr->TouP_Startx > MinXPoint) &&\
//              (CloumPtr->TouP_EndX < MaxXPoint) || ) { 
// 
//       Print (L"Shell Area SX: %08x Ex: %08x\n", CloumPtr->TouP_Startx, CloumPtr[Index].TouP_EndX);
//       for (Index = 0; Index < RowMapPtr->MaxRowNum; Index++) { 
//         if ((CloumPtr[Index].TouP_Starty > MinYPoint) &&\
//               (CloumPtr[Index].TouP_Endy < MaxYPoint)) { 
//           //
//           // This is EFI SHELL area, have to skip this area. 
//           // So, set null flag
//           //
//           Print (L"Shell Area SY: %08x EY: %08x\n", CloumPtr->TouP_Starty, CloumPtr[Index].TouP_Endy);
//           CloumPtr[Index].Flag = NULL_TOUCH_POINT;
//         } // if ((CloumPtr[Index].TouP_Starty > MinYPoint) &&\.... end this
// 
//       } // for (Index = 0; Index < RowMapPtr->MaxRowNum; Index++) { end this
// 
//     } // if (CloumPtr->TouP_Startx > MinXPoint) &&\.... end this
// 
//     // find to next row block
//     RowMapPtr = RowMapPtr->NextPtr; 
//   } // while (RowMapPtr != NULL) { ..... end this
  
  //
  // Check Key arry
  //
#if 1
  for (Index = 0; Index < DIM(VkMapArryStd); Index++) { 
//   for (Index = 0; Index < 3; Index++) { 
    Print(L"Please check the key address %s ", VkMapArryStd[Index].PrintString);
    if (VkMapArryStd[Index].ShiftChgString != NULL) { 
      Print(L" Shift key is  %s \n", VkMapArryStd[Index].ShiftChgString);
    } else { 
      Print(L"\n");
    }
    
    Status = CheckMatchPoint(AbsolutePointerProtocol,VkMainPtr->MapInfoPtr,&CloumPtr);
    
    if (Status == EFI_ACCESS_DENIED) { 
      Print(L" This point touch EFI Shell area, please try others place\n");
      Index--;
    } else if(Status == EFI_SUCCESS) { 
      //
      // Check this point did link or not
      //
      if (CloumPtr->MappingKeyPtr == NULL) { 
        CloumPtr->MappingKeyPtr = &VkMapArryStd[Index];
        CloumPtr->Flag = VIRTUAL_STD_KEY_SET;
        //
        // Draw the font to the select key
        //
        if (VkMapArryStd[Index].BitMapPtr != NULL) { 
          DrawCharacter(VkMainPtr->GfxProtocol, CloumPtr);
        }
      } else { 
        KeyPtr = (VKMAP_DESC*)CloumPtr->MappingKeyPtr;
        Print(L"This point touch overlap with \"%s\"\n",KeyPtr->Unicode);
        Print(L"Please try again\n");
        Index--;      
      } // if (CloumPtr->MappingKeyPtr == NULL.... end this
      
    } else { 
      Print(L" This point touch couldn't match and data skip\n");
    } // if (Status == EFI_ALREADY_STARTED) {  end this
  } //   for (Index = 0; Index < 3; Index++) { end this

  //
  // Check NUM page key support or not
  // ???

  // for temp debug
//   SwitchKeyPageFlag = VIRTUAL_NUM_KEY_SET;
   SwitchKeyPageFlag = VIRTUAL_STD_KEY_SET;

#if 1
  for (Index = 0; Index < DIM(VkMapArryNum); Index++) { 
    Print(L"Please check the key address %s ", VkMapArryNum[Index].PrintString);

    Status = CheckMatchPoint(AbsolutePointerProtocol,VkMainPtr->MapInfoPtr,&CloumPtr);
    
    if (Status == EFI_ACCESS_DENIED) { 
      Print(L" This point touch EFI Shell area, please try others place\n");
      Index--;
    } else if(Status == EFI_SUCCESS) { 
      //
      // Check this point did link or not
      //
      if (SwitchKeyPageFlag == VIRTUAL_NUM_KEY_SET) { 
        //
        // User select num page
        //
        KeyPtr = (VKMAP_DESC*)CloumPtr->MapingPage1Ptr;
      } else { 
        KeyPtr = (VKMAP_DESC*)CloumPtr->MappingKeyPtr;
      }

      if (KeyPtr == NULL) { 
        
        if (SwitchKeyPageFlag == VIRTUAL_NUM_KEY_SET) {
          CloumPtr->MapingPage1Ptr = &VkMapArryNum[Index];
        } else { 
          CloumPtr->MappingKeyPtr = &VkMapArryNum[Index];
        }  // if (SwitchKeyPageFlag == VIRTUAL_NUM_KEY_SET) {..end this
        
        CloumPtr->Flag = VIRTUAL_STD_KEY_SET;
        //
        // Draw the font to the select key
        //
        if (VkMapArryNum[Index].BitMapPtr != NULL) { 
          DrawCharacter(VkMainPtr->GfxProtocol, CloumPtr);
        }
      } else { 
        Print(L"This point touch overlap with \"%s\"\n",KeyPtr->Unicode);
        Print(L"Please try again\n");
        Index--;      
      } // if (CloumPtr->MappingKeyPtr == NULL.... end this
      
      
    } else { 
      Print(L" This point touch couldn't match and data skip\n");
    } // if (Status == EFI_ALREADY_STARTED) {  end this
  } //   for (Index = 0; Index < 3; Index++) { end this
#endif   
#endif
 
  return EFI_SUCCESS;
}

#endif

EFI_STATUS
OpenTheNewFile( 
  IN VK_APP_STRUCT *VkMainPtr
  )
{ 
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_DEVICE_PATH_PROTOCOL         *DevicePath;
//   EFI_FILE_HANDLE                  *Root;
  EFI_FILE_PROTOCOL                *Root;
  EFI_STATUS   Status;
  EFI_HANDLE   *HandleArray;
//   EFI_FILE     FileHandle;
  UINTN        HandleArrayCount;
  UINTN        Index;

  Root = NULL;
  
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    Print(L"LocateHandleBuffer failed at \n");
    return Status;
  }
  Print(L"LocateHandleBuffer gEfiPartTypeSystemPartGuid Count = %0d \n",HandleArrayCount);
  

  for (Index = 0; Index < HandleArrayCount; Index++) { 
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Print(L"Foun File system \n");
  
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Print(L"Get Device Path sizeof(gUsb0DevPath) = %x\n",sizeof(gUsb0DevPath));
  
    if (CompareMem(DevicePath, &gUsb0DevPath, sizeof(gUsb0DevPath) - 4) != 0){  
      continue;
    }
  
    //
    // Open the root directory, get z
    //
    Status = Fs->OpenVolume (Fs, &Root);
    if (EFI_ERROR (Status)) {
      Print(L"OpenVolume\n");
      continue;
    }
    Print(L"OpenVolme successful \n");

//     Root = VkMainPtr->FileRoot;
    Status = Root->Open (Root, &VkMainPtr->FileHandle, KEY_MAPPING_FILE, EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ|EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR (Status)) {
      Print(L"VKTEST APP: OpenFile failed %r\n", Status);
      continue;
    }
  
    break;
  
  }

  VkMainPtr->RootFileProtocol = Root;
//   VkMainPtr->FileHandle = FileHandle;
//   Status = Root->Close (Root);
//   Print(L"VKTEST APP: Close file status %r\n", Status);

  return EFI_SUCCESS;
}

#define FILE_ENTER 0x0D0A

UINT8
HexToAscII( 
  UINT8   Data
  )
{ 
  if (Data < 0x0A) { 
    return (0x30 + Data);
  } else { 
    return (0x41 + (Data - 0x0A));
  }

}

EFI_STATUS
UINT8toAscii( 
  UINT8     Data,
  UINT8     *BufferPtr,
  UINTN     BufferSize
  ) 
{ 
  
  if (BufferSize < 2) { 
    return EFI_BUFFER_TOO_SMALL;
  }

  BufferPtr[0] = HexToAscII((UINT8)(Data >> 4) & 0x0f);
  BufferPtr[1] = HexToAscII((UINT8)(Data) & 0x0f);

  return EFI_SUCCESS;

}


EFI_STATUS
UINT16toAscii( 
  UINT16    Data,
  UINT8     *BufferPtr,
  UINTN     BufferSize
  ) 
{ 
  EFI_STATUS Status;
  
  if (BufferSize < 4) { 
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = UINT8toAscii((UINT8)(Data >> 8) & 0xff,
                         &BufferPtr[0],
                         BufferSize
                        );

  if (EFI_ERROR(Status)) { 
    return Status;
  }
   
  Status = UINT8toAscii((UINT8)(Data) & 0xff,
                         &BufferPtr[2],
                         BufferSize - 2
                        );

  if (EFI_ERROR(Status)) { 
    return Status;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
UINT32toAscii( 
  UINT32    Data,
  UINT8     *BufferPtr,
  UINTN     BufferSize
  ) 
{ 
  EFI_STATUS Status;
  
  if (BufferSize < 8) { 
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = UINT16toAscii((UINT16)((Data >> 16) & 0xffff),
                         &BufferPtr[0],
                         BufferSize
                        );

  if (EFI_ERROR(Status)) { 
    return Status;
  }
   
  Status = UINT16toAscii((UINT16)((Data) & 0xffff),
                         &BufferPtr[4],
                         BufferSize - 4
                        );

  if (EFI_ERROR(Status)) { 
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
TxFontDataTofile(  
  IN EFI_FILE_PROTOCOL   *RootFsPtr,
  IN VKMAP_DESC          *FontDescPtr
  )
{ 
  EFI_STATUS Status;
  UINT8      *RootBufferPtr;
  UINT8      *BufferPtr;
  UINTN      ToTalBufSize;
//   UINTN      FileBufferSize;
  UINTN      BufferSize;
  UINTN      FontSize;
  UINT8      Count;
  UINT8      *FontBufPtr;
  
                           // offset 0 - 8
                           // offset 9 - 23 offset Structure name
                           // offset 24- 38 offset Unicode
                           // offset 39- 53 offset High
                           // offset 54- 68 offset Width
                           // Row data
  UINT8      Template[] = {'U','I','N','T','3','2',' ',0x0d,0x0a,
                   '_','0','x','1','2','3','4','[',']',' ','=',' ','{',0x0d,0x0a,
                   ' ',' ','0','x','1','2','3','4','5','6','7','8',',',0x0d,0x0a,
                   ' ',' ','0','x','1','2','3','4','5','6','7','8',',',0x0d,0x0a,
                   ' ',' ','0','x','1','2','3','4','5','6','7','8',',',0x0d,0x0a};                                           

  UINT8      RowTemplate[] = 
                  {' ',' ','0','x','1','2','3','4','5','6','7','8',',',0x0d,0x0a};
  
  Print(L"Set Font to file the unicode is 0x%02x\n",FontDescPtr->Unicode);
  //
  // Initial variable
  //
  ToTalBufSize = 4 * 1024;
  RootBufferPtr = AllocateZeroPool(ToTalBufSize);
  BufferPtr = RootBufferPtr;
  BufferSize = ToTalBufSize;
  
  //
  // Read file and check the end of file position
  //
  Status = RootFsPtr->Read(RootFsPtr, &BufferSize, RootBufferPtr);

  Print(L"Read file buffer %r\n",Status);
  if (EFI_ERROR(Status)) { 
    return EFI_ABORTED;
  }
  
//   //
//   // Set to the latest position
//   //
//   Status = RootFsPtr->SetPosition(RootFsPtr, BufferSize);
//   Print(L"Set Position buffer %r\n",Status);
//   if (EFI_ERROR(Status)) { 
//     return EFI_OUT_OF_RESOURCES;
//   }
  
  //
  // Check the Font data size
  //  

  //
  // comment ??
  //
  //         _0x[ASCIIcode]     = { [0x0D0A]
  //                              0x[UINT32],[0x0D0A]
  //                              ....      ,[0x0D0A]
  //                              ...       ....
  //                              ..
  //                              }; [0x0D0A]
  //
//   FontSize = sizeof(Template) + \
//                ((FontDescPtr->High) * (sizeof("  0x12345678,") + 2)) \
//                + sizeof(" };") + 2;
//   FontBufPtr = AllocateZeroPool(FontSize);
  FontBufPtr = RootBufferPtr;
  FontSize = BufferSize;
  //
  // Create the buffer for write font structure
  //
  Status = UINT16toAscii(FontDescPtr->Unicode,
                         &Template[12],
                         DIM(Template) - 12
                        );
  
  if (EFI_ERROR(Status)) { 
    return EFI_LOAD_ERROR;
  }

  //
  // Unicode
  //
  Status = UINT32toAscii((UINT32)FontDescPtr->Unicode,
                         &Template[28],
                         DIM(Template) - 28
                        );
  
  if (EFI_ERROR(Status)) { 
    return EFI_LOAD_ERROR;
  }

  //
  // NumOfRow
  //
  Status = UINT32toAscii((UINT32)FontDescPtr->High,
                         &Template[43],
                         DIM(Template) - 43
                        );
  
  if (EFI_ERROR(Status)) { 
    return EFI_LOAD_ERROR;
  }

  //
  // Width
  //
  Status = UINT32toAscii((UINT32)FontDescPtr->Width,
                         &Template[58],
                         DIM(Template) - 58
                        );
  
  if (EFI_ERROR(Status)) { 
    return EFI_LOAD_ERROR;
  }

//   BufferPtr = &Template[17];
//   BufferSize = FontSize - 17;
  
  //
  // Copy Row data
  //
  CopyMem(FontBufPtr, Template, DIM(Template));

  BufferPtr = &FontBufPtr[DIM(Template)];
  BufferSize = FontSize - DIM(Template);
  
  for (Count = 0; Count < FontDescPtr->High; Count++) { 
    Status = UINT32toAscii(FontDescPtr->BitMapPtr[Count],
                           &RowTemplate[4],
                           BufferSize
                          );
    if (EFI_ERROR(Status)) { 
      return EFI_LOAD_ERROR;
    }

    CopyMem(&BufferPtr[0],&RowTemplate[0],DIM(RowTemplate));
    
    BufferPtr = &BufferPtr[DIM(RowTemplate)];
    BufferSize -= DIM(RowTemplate);
  }

  Print(L"Length = %08x, remain 0x%08x\n",BufferSize);
  //
  // the end of data
  //
  BufferPtr[0] = '}';
  BufferPtr[1] = ';';
  BufferPtr[2] = 0x0d;
  BufferPtr[3] = 0x0a;
  BufferSize -= 4;
  

  //
  // Write to file.
  //
  BufferSize = FontSize - BufferSize;
  Status = RootFsPtr->Write(RootFsPtr, &BufferSize, FontBufPtr);

  Print(L"Write font buffer size 0x%08x and Status = %r\n",BufferSize,Status);
  if (EFI_ERROR(Status)) { 
    return Status;
  }

  //
  // Done
  //
  FreePool(RootBufferPtr);
  FreePool(FontBufPtr);
  

  return EFI_SUCCESS;
}

EFI_STATUS
SetDataTofile( 
  IN VK_APP_STRUCT *VkMainPtr
  )
{ 
  EFI_FILE_HANDLE Root;  
  VKMAP_DESC      *FontDescPtr;
  EFI_STATUS      Status;
  BLK_INFO        *ClumPtr;
  POINT_INFO      *RowPtr;
  UINT8           *BufferPtr;
  UINT8           *RootBufferPtr;
  UINT8           *TempFileRootPtr;
  UINTN           TotalBufferSize;
  UINTN           BufferSize;
  UINTN           Count;
  CHAR8           Hdr[] = {'V','K','S','T','R','U',' ',\
                           'X','m','a','n','[',']',' ','=',' ','{',0x0d,0x0a};
  CHAR8           Endstruct[] = {'}',';',0x0d,0x0a};
   //
   //     StartX, Starty,   EndX,   EndY, Tou_SX, Tou_SY, Tou_EX, Tou_EY,    Addr,    NULL,    NULL,    NULL
   //  {  0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, 0x1234, _0x0031,    NULL,    NULL,    NULL},0x0d,0x0a
   //  0123      11      19      27      35      43      51      59      67       76 79    85 88    94  97 
   //
   //  StartX locate at 0x05
   //  Starty locate at 0x0D
   //  EndX   locate at 0x15
   //  EndY   locate at 0x1D
   //  Tou_SX locate at 0x25
   //  Tou_SY locate at 0x2D
   //  Tou_Ex locate at 0x35
   //  Tou_Ey locate at 0x3D
   //  Point  locate at 0x46, 0x4F, 0x58 and 0x61
   //                              0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
  UINT8           RowDataTem[] = {'{',' ',' ','0','x','1','2','3','4',',',' ','0','x','1','2','3',
                                  '4',',',' ','0','x','1','2','3','4',',',' ','0','x','1','2','3',
                                  '4',',',' ','0','x','1','2','3','4',',',' ','0','x','1','2','3',
                                  '4',',',' ','0','x','1','2','3','4',',',' ','0','x','1','2','3',
                                  '4',',',' ',' ',' ',' ','N','U','L','L',',',' ',' ',' ',' ','N',
                                  'U','L','L',',',' ',' ',' ',' ','N','U','L','L',',',' ',' ',' ',
                                  ' ','N','U','L','L','}',',',0x0d,0x0a};
  UINTN           TestCount;

  TempFileRootPtr = NULL;
  BufferPtr = NULL;
  BufferSize = 0;
  FontDescPtr = NULL;
  TotalBufferSize = 4 * 1024;
  
  RootBufferPtr = AllocateZeroPool(TotalBufferSize);
  BufferSize = TotalBufferSize;


  Root = VkMainPtr->FileHandle;

  if (Root == NULL) { 
    return EFI_LOAD_ERROR;
  }

  ClumPtr = VkMainPtr->MapInfoPtr;
  
//   Status = Root->SetPosition(Root,0);
  //
  // Write font bitmap to file 
  //
  TestCount = 0;
  while (ClumPtr != NULL) { 
    RowPtr = ClumPtr->PointPtr;
    for (Count = 0; Count < ClumPtr->MaxRowNum; Count++) { 
      if (RowPtr[Count].MappingKeyPtr != NULL) { 
        TxFontDataTofile(Root,RowPtr[Count].MappingKeyPtr);
      } 

      if (RowPtr[Count].MapingPage1Ptr != NULL) { 
        TxFontDataTofile(Root,RowPtr[Count].MapingPage1Ptr);
      } 

      if (RowPtr[Count].MapingPage2Ptr != NULL) { 
        TxFontDataTofile(Root,RowPtr[Count].MapingPage2Ptr);
      } 

      if (RowPtr[Count].MapingPage3Ptr != NULL) { 
        TxFontDataTofile(Root,RowPtr[Count].MapingPage3Ptr);
      } 
    }    
    ClumPtr = ClumPtr->NextPtr;
    TestCount++;
  } // while (ClumPtr != NULL) { ... end this
  
  Print(L"Test Count = %0d\n",TestCount);
  //
  // write every x,y date to file
  //

  BufferSize = sizeof(Hdr);
  Status = Root->Write(Root, &BufferSize, &Hdr[0]);
  
  BufferPtr = RootBufferPtr;
  BufferSize = TotalBufferSize;

  
  ClumPtr = VkMainPtr->MapInfoPtr;

  //
  // Xman [] = { // StartX, Starty, EndX, EndY, Tou_SX, Tou_SY, Tou_EX,  Tou_EY, Addr, NULL, NULL, NULL
  //             {    0x00,   0x00, 0x00, 0x00,   0x00,   0x00,   0x00,    0x00, &_0x0031, NULL, NULL, NULL},
  //             {    0x00,   0x00, 0x00, 0x00,   0x00,   0x00,   0x00,    0x00, &_0x0031, NULL, NULL, NULL},
  //             ........      
  //           };
  //
  //  StartX locate at 0x05
  //  Starty locate at 0x0D
  //  EndX   locate at 0x15
  //  EndY   locate at 0x1D
  //  Tou_SX locate at 0x25
  //  Tou_SY locate at 0x2D
  //  Tou_Ex locate at 0x35
  //  Tou_Ey locate at 0x3D
  //  Point  locate at 0x46, 0x4F, 0x58 and 0x61
  //
#if 1
  TestCount = 0;
  while (ClumPtr != NULL) { 
    TestCount++;
    RowPtr = ClumPtr->PointPtr;
    Print(L"MaxRowNum Count = %0d\n", ClumPtr->MaxRowNum);
    for (Count = 0; Count < ClumPtr->MaxRowNum; Count++) { 

      if ((RowPtr[Count].Flag & NULL_TOUCH_POINT) != 0) { 
        //
        // This point didn't use, skip this
        //
        continue;
      }


      CopyMem(BufferPtr,&RowDataTem[0],sizeof(RowDataTem));

      //
      // StartX
      // 
      Status = UINT16toAscii(RowPtr[Count].Dis_Startx,
                             &BufferPtr[0x05],
                             4);

      //
      // StartY
      // 
      Status = UINT16toAscii(RowPtr[Count].Dis_Starty,
                             &BufferPtr[0x0D],
                             4);
     
      //
      // Endx
      // 
      Status = UINT16toAscii(RowPtr[Count].Dis_EndX,
                             &BufferPtr[0x15],
                             4);



      //
      // Endy
      // 
      Status = UINT16toAscii(RowPtr[Count].Dis_Endy,
                             &BufferPtr[0x1D],
                             4);


      //
      // Touch StartX
      // 
      Status = UINT16toAscii(RowPtr[Count].TouP_Startx,
                             &BufferPtr[0x25],
                             4);

      //
      // Touch Starty
      // 
      Status = UINT16toAscii(RowPtr[Count].TouP_Starty,
                             &BufferPtr[0x2D],
                             4);

      //
      // Touch Endx
      //
      Status = UINT16toAscii(RowPtr[Count].TouP_EndX,
                             &BufferPtr[0x35],
                             4);

      //
      // Touch Endy
      //
      Status = UINT16toAscii(RowPtr[Count].TouP_Endy,
                             &BufferPtr[0x3D],
                             4);

      //
      // Point 0
      //
      if (RowPtr[Count].MappingKeyPtr != NULL) { 
        FontDescPtr = RowPtr[Count].MappingKeyPtr;
        BufferPtr[0x43] = '_';
        BufferPtr[0x44] = '0';
        BufferPtr[0x45] = 'x';
        Status = UINT16toAscii(FontDescPtr->Unicode,
                               &BufferPtr[0x46],
                               4);
                
      } 

      //
      // Point 1
      //
      if (RowPtr[Count].MapingPage1Ptr != NULL) { 
        FontDescPtr = RowPtr[Count].MapingPage1Ptr;
        BufferPtr[0x4C] = '_';
        BufferPtr[0x4D] = '0';
        BufferPtr[0x4E] = 'x';
        Status = UINT16toAscii(FontDescPtr->Unicode,
                               &BufferPtr[0x4F],
                               4);
                
      } 

      //
      // Point 2
      //
      if (RowPtr[Count].MapingPage2Ptr != NULL) { 
        FontDescPtr = RowPtr[Count].MapingPage2Ptr;
        BufferPtr[0x55] = '_';
        BufferPtr[0x56] = '0';
        BufferPtr[0x57] = 'x';
        Status = UINT16toAscii(FontDescPtr->Unicode,
                               &BufferPtr[0x58],
                               4);
                
      } 

      //
      // Point 3
      //
      if (RowPtr[Count].MapingPage3Ptr != NULL) { 
        FontDescPtr = RowPtr[Count].MapingPage3Ptr;
        BufferPtr[0x5E] = '_';
        BufferPtr[0x5F] = '0';
        BufferPtr[0x60] = 'x';
        Status = UINT16toAscii(FontDescPtr->Unicode,
                               &BufferPtr[0x61],
                               4);
                
      } 

      BufferSize -= sizeof(RowDataTem);
      BufferPtr = &BufferPtr[sizeof(RowDataTem)];
      
      if (BufferSize < sizeof(RowDataTem)) { 
        //
        // Buffer full, write data to the file
        //
        BufferSize = TotalBufferSize - BufferSize;
        Status = Root->Write(Root, &BufferSize, RootBufferPtr);

        //
        // Buffer pointer return to the TOP
        //
        BufferPtr = RootBufferPtr;
        BufferSize = TotalBufferSize;
        
      } // if (BufferSize < sizeof(RowDataTem)) {  end this
      
    } //  for (Count = 0; Count < ClumPtr->MaxRowNum; Count++) {  end this   
    ClumPtr = ClumPtr->NextPtr;

  } //  while (ClumPtr != NULL) {  end this
  Print(L"Test2 Count = %0d, left BufferSize = %0d\n",TestCount, BufferSize);

#endif 

  //
  // Write to file.
  //
//   BufferSize = sizeof(Endstruct);
//   Status = Root->Write(Root, &BufferSize, Endstruct);
  Print(L"Copy memory\n");
  CopyMem(BufferPtr,Endstruct,sizeof(Endstruct));
  BufferSize -= sizeof(Endstruct);
  BufferPtr = &BufferPtr[sizeof(Endstruct)];
  BufferSize = TotalBufferSize - BufferSize;
  Print(L"Write file\n");
  Status = Root->Write(Root, &BufferSize, RootBufferPtr);
  Print(L"FreePool....\n");



  //
  // Done
  //
  FreePool(RootBufferPtr);
    
  return EFI_SUCCESS;
}

INTN
EFIAPI 
ShellAppMain (
  IN UINTN Argc, 
  IN CHAR16 **Argv
  )
{
  EFI_STATUS                      Status= EFI_SUCCESS;
  EFI_ABSOLUTE_POINTER_PROTOCOL   *AbsolutePointerProtocol;
  EFI_HANDLE                      *HandleBuffer = NULL;
  UINTN	                          count = 0;
//   EFI_INPUT_KEY                   Key;
  EFI_HANDLE                      *GraphicsHandles;
  UINTN                           GrpahicsHandleCount;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsoutProtocol;

  
  //
  // Locate touched device 
  //
  Status = gBS->LocateHandleBuffer(   
									ByProtocol,
									&gEfiAbsolutePointerProtocolGuid,
									NULL,
									&count,
									&HandleBuffer											
									);
	 Print (L"Locate AbsolutePointerProtocol Handle buffer %r handle count = %d\n", Status,count);
		if (EFI_ERROR (Status)) {
  	 Print (L"Locate AbsolutePointerProtocol fail exit..\n");
				return Status;
		}

		ASSERT(count == 2);

  Status = gBS->OpenProtocol (
                    HandleBuffer[1],
                    &gEfiAbsolutePointerProtocolGuid,
                    (VOID **) &AbsolutePointerProtocol,
                    gImageHandle,
                    HandleBuffer[1],
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
  
	 Print (L"OpenProtocol buffer %r\n", Status);
  if (EFI_ERROR (Status)) {
  	 Print (L"Open AbsolutePointerProtocol fail exit..\n");
   	return Status;
  }

  mVkInfo.TouchHandle = HandleBuffer[1];
  mVkInfo.TouchProtocol = AbsolutePointerProtocol;

  //
  // Locate graphics   
  //
  Status = gBS->LocateHandleBuffer(   
									ByProtocol,
									&gEfiGraphicsOutputProtocolGuid,
									NULL,
									&GrpahicsHandleCount,
									&GraphicsHandles											
									);

	 Print (L"Locate gEfiGraphicsOutputProtocolGuid Handle buffer %r handle count = %d\n", Status,GrpahicsHandleCount);
  if (EFI_ERROR (Status)) {
  	 Print (L"Locate gEfiGraphicsOutputProtocol fail exit..\n");
   	return Status;
  }
  
  Status = gBS->OpenProtocol (
                    GraphicsHandles[ 0 ],
                    &gEfiGraphicsOutputProtocolGuid,
                    (VOID**)&GraphicsoutProtocol,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL 
                    );
  

	 Print (L"OpenProtocol buffer %r\n", Status);
  if (EFI_ERROR (Status)) {
  	 Print (L"Open AbsolutePointerProtocol fail exit..\n");
   	return Status;
  }

  mVkInfo.GfxHandle = GraphicsHandles[ 0 ];
  mVkInfo.GfxProtocol = GraphicsoutProtocol;
  //
  // Draw the lines in display
  // Save display and touch address 
  //
  DrawingLine(&mVkInfo,&mVkInfo.MaxH_Line, &mVkInfo.MaxV_Line);

#if 1
  //
  // Open the file for saving user setting data
  //
  OpenTheNewFile(&mVkInfo);
  
  //
  // Create timer event for detect the user setting
  //
  
  //
  // Check every keys address  
  //
  
  MappingKeyAndTouch(&mVkInfo);
 
  //
  // Save to setting file
  //
  SetDataTofile(&mVkInfo);  


  //
  // test AP exiting.
  //
  FreeMapInfoMem(&mVkInfo);
#endif

  if (mVkInfo.RootFileProtocol != NULL) { 
    mVkInfo.RootFileProtocol->Close(mVkInfo.RootFileProtocol);
  }
  
  FreeMapInfoMem(&mVkInfo);

  gBS->CloseProtocol (
  						HandleBuffer[1],
  						&gEfiAbsolutePointerProtocolGuid,
  						gImageHandle,
  						HandleBuffer[1]
  					);
					

  gBS->CloseProtocol (
  						GraphicsHandles[0],
  						&gEfiGraphicsOutputProtocolGuid,
  						NULL,
  						NULL
  					);

   return (INTN)Status;
}
