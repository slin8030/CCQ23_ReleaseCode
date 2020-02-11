/** @file
  This OemService is part of setting Verb Table. The function is created for setting verb table
  to support Multi-Sku and return the table to common code to program.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <MultiSkuInfoCommonDef.h>
#include <Ppi/VerbTable.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/GpioLib.h>
#include <Library/CmosLib.h>

#define N_GPIO_32             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0100     //GPIO_32
#define N_GPIO_33             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0108     //GPIO_33
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sample :                                                                                                      //
//                                                                                                               //
//   #define OEM_VERB_TABLE_ID_(n)         n                                                                     //
//                                                                                                               //
//   #define OEM_VERB_TABLE_(n)_HEADER(a)  0x10EC0880, // Vendor ID / Device ID                                  //
//                                         0x00000000, // SubSystem ID                                           //
//                                         0x02,       // RevisionId                                             //
//                                         0x01,       // Front panel support ( 1 = Yes, 2 = No )                //
//                                         0x000A,     // Number of Rear Jacks = 10                              //
//                                         0x0002      // Number of Front Jacks = 2                              //
//                                                                                                               //
//   #define OEM_VERB_TABLE_(n)_DATA(a)    0x01171CF0,0x01171D11,0x01171E11,0x01171F41, // NID(0x11): 0x411111F0 //
//                                         0x01271C40,0x01271D09,0x01271EA3,0x01271F99, // NID(0x12): 0x99A30940 //
//                                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// To define Verb Table ID.
//
//AMIC-AMP GPIO32,33 (0, 0)
#define OEM_VERB_TABLE_ID_1          1

#define OEM_VERB_TABLE_1_HEADER1     0x80862668, \
                                     0x013214C0, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000F,     \
                                     0x0000 


#define OEM_VERB_TABLE_1_DATA1     0x00172032,  0x00172101,  0x001722C0,  0x00172314, \
                                   0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \
                                   0x01271C00,  0x01271D00,  0x01271E00,  0x01271F40, \
                                   0x01471CF0,  0x01471D11,  0x01471E11,  0x01471F41, \
                                   0x01771CF0,  0x01771D11,  0x01771E11,  0x01771F41, \
                                   0x01871CF0,  0x01871D11,  0x01871E11,  0x01871F41, \
                                   0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                   0x01A71C30,  0x01A71D01,  0x01A71EA7,  0x01A71F90, \
                                   0x01B71C10,  0x01B71D01,  0x01B71E17,  0x01B71F90, \
                                   0x01D71C3D,  0x01D71D3A,  0x01D71E48,  0x01D71F40, \
                                   0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41, \
                                   0x02171C20,  0x02171D10,  0x02171E21,  0x02171F02, \
                                   0x02050038,  0x02044981,  0x02050038,  0x02044981, \
                                   0x05750003,  0x057486A6,  0x02050034,  0x02048204, \
                                   0x0205001B,  0x02040A0B,  0x02050046,  0x02040004, \
                                   0x02050008,  0x02046A0C,  0x02050040,  0x02041800, \
                                   0x02050037,  0x02044A06,  0x0205004C,  0x02044803

//AMIC-Codec GPIO32,33 (1, 0)
#define OEM_VERB_TABLE_ID_2          2

#define OEM_VERB_TABLE_2_HEADER1     0x80862668, \
                                     0x013214C0, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000F,     \
                                     0x0000  

#define OEM_VERB_TABLE_2_DATA1       0x00172032,  0x00172101,  0x001722C0,  0x00172314, \
                                     0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \
                                     0x01271C00,  0x01271D00,  0x01271E00,  0x01271F40, \
                                     0x01471C10,  0x01471D01,  0x01471E17,  0x01471F90, \
                                     0x01771CF0,  0x01771D11,  0x01771E11,  0x01771F41, \
                                     0x01871CF0,  0x01871D11,  0x01871E11,  0x01871F41, \
                                     0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                     0x01A71C30,  0x01A71D01,  0x01A71EA7,  0x01A71F90, \
                                     0x01B71CF0,  0x01B71D11,  0x01B71E11,  0x01B71F41, \
                                     0x01D71C3D,  0x01D71D3A,  0x01D71E48,  0x01D71F40, \
                                     0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41, \
                                     0x02171C20,  0x02171D10,  0x02171E21,  0x02171F02, \
                                     0x02050038,  0x02047981,  0x02050038,  0x02047981, \
                                     0x05750003,  0x057486A6,  0x02050034,  0x02048204, \
                                     0x0205001B,  0x02040A0B,  0x02050046,  0x02040004, \
                                     0x02050008,  0x02046A0C,  0x02050040,  0x02041800, \
                                     0x02050037,  0x02044A06,  0x0205004C,  0x02044803,

//DMIC-AMP GPIO32,33 (0, 1)
#define OEM_VERB_TABLE_ID_3          3

#define OEM_VERB_TABLE_3_HEADER1     0x80862668, \
                                     0x013214C0, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000F,     \
                                     0x0000  

#define OEM_VERB_TABLE_3_DATA1       0x00172032,  0x00172101,  0x001722C0,  0x00172314, \
                                     0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \
                                     0x01271C30,  0x01271D01,  0x01271EA6,  0x01271F90, \
                                     0x01471C00,  0x01471D00,  0x01471E00,  0x01471F40, \
                                     0x01771CF0,  0x01771D11,  0x01771E11,  0x01771F41, \
                                     0x01871CF0,  0x01871D11,  0x01871E11,  0x01871F41, \
                                     0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                     0x01A71CF0,  0x01A71D11,  0x01A71E11,  0x01A71F41, \
                                     0x01B71C10,  0x01B71D01,  0x01B71E17,  0x01B71F90, \
                                     0x01D71C3D,  0x01D71D3A,  0x01D71E88,  0x01D71F40, \
                                     0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41, \
                                     0x02171C20,  0x02171D10,  0x02171E21,  0x02171F02, \
                                     0x02050038,  0x02044981,  0x02050038,  0x02044981, \
                                     0x05750003,  0x057486A6,  0x02050034,  0x02048204, \
                                     0x0205001B,  0x02040A0B,  0x02050046,  0x02040004, \
                                     0x02050008,  0x02046A0C,  0x02050040,  0x02041800, \
                                     0x02050037,  0x02044A06,  0x0205004C,  0x02044803,

//DMIC-Codec GPIO32,33 (1, 1)
#define OEM_VERB_TABLE_ID_4          4

#define OEM_VERB_TABLE_4_HEADER1     0x80862668, \
                                     0x013214C0, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000F,     \
                                     0x0000  

#define OEM_VERB_TABLE_4_DATA1       0x00172032,  0x00172101,  0x001722C0,  0x00172314, \
                                     0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \
                                     0x01271C30,  0x01271D01,  0x01271EA6,  0x01271F90, \
                                     0x01471C10,  0x01471D01,  0x01471E17,  0x01471F90, \
                                     0x01771C00,  0x01771D00,  0x01771E00,  0x01771F40, \
                                     0x01871CF0,  0x01871D11,  0x01871E11,  0x01871F41, \
                                     0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                     0x01A71CF0,  0x01A71D11,  0x01A71E11,  0x01A71F41, \
                                     0x01B71CF0,  0x01B71D11,  0x01B71E11,  0x01B71F41, \
                                     0x01D71C3D,  0x01D71D3A,  0x01D71EE8,  0x01D71F40, \
                                     0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41, \
                                     0x02171C20,  0x02171D10,  0x02171E21,  0x02171F02, \
                                     0x02050038,  0x02047981,  0x02050038,  0x02047981, \
                                     0x05750003,  0x057486A6,  0x02050034,  0x02048204, \
                                     0x0205001B,  0x02040A0B,  0x02050046,  0x02040004, \
                                     0x02050008,  0x02046A0C,  0x02050040,  0x02041800, \
                                     0x02050037,  0x02044A06,  0x0205004C,  0x02044803,

#if 0
//
//  VerbTable: ALC662  (Not used in Apollo Lake CRB)
//  Revision ID = 0xff
//  Codec Vendor: 0x10EC0662
//

//
//    NID 0x12 : 0x40130000
//    NID 0x14 : 0x01014010
//    NID 0x15 : 0x01011012
//    NID 0x16 : 0x01016011
//    NID 0x18 : 0x01A19030
//    NID 0x19 : 0x02A19040
//    NID 0x1A : 0x0181303F
//    NID 0x1B : 0x0221401F
//    NID 0x1C : 0x411111F0
//    NID 0x1D : 0x4045E601
//    NID 0x1E : 0x01441120
//

#define OEM_VERB_TABLE_1_HEADER2     0x10EC0662, \
                                     0x0000,     \
                                     0xFF,       \
                                     0x01,       \
                                     0x000C,     \
                                     0x0002

#define OEM_VERB_TABLE_1_DATA2       0x00172000,  0x00172100,  0x00172286,  0x00172380, \   //;HDA Codec Subsystem ID  : 0x80860000
                                     0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \   //;Widget node 0x01 :
                                     0x01271C00,  0x01271D00,  0x01271E13,  0x01271F40, \   //;Pin widget 0x12 - DMIC
                                     0x01471C10,  0x01471D40,  0x01471E01,  0x01471F01, \   //;Pin widget 0x14 - FRONT (Port-D)
                                     0x01571C12,  0x01571D10,  0x01571E01,  0x01571F01, \   //;Pin widget 0x15 - SURR (Port-A)
                                     0x01671C11,  0x01671D60,  0x01671E01,  0x01671F01, \   //;Pin widget 0x16 - CEN/LFE (Port-G)
                                     0x01871C30,  0x01871D90,  0x01871EA1,  0x01871F01, \   //;Pin widget 0x18 - MIC1 (Port-B)
                                     0x01971C40,  0x01971D90,  0x01971EA1,  0x01971F02, \   //;Pin widget 0x19 - MIC2 (Port-F)
                                     0x01A71C3F,  0x01A71D30,  0x01A71E81,  0x01A71F01, \   //;Pin widget 0x1A - LINE1 (Port-C)
                                     0x01B71C1F,  0x01B71D40,  0x01B71E21,  0x01B71F02, \   //;Pin widget 0x1B - LINE2 (Port-E)
                                     0x01C71CF0,  0x01C71D11,  0x01C71E11,  0x01C71F41, \   //;Pin widget 0x1C - CD-IN
                                     0x01D71C01,  0x01D71DE6,  0x01D71E45,  0x01D71F40, \   //;Pin widget 0x1D - BEEP-IN
                                     0x01E71C20,  0x01E71D11,  0x01E71E44,  0x01E71F01, \   //;Pin widget 0x1E - S/PDIF-OUT
                                     0x02050004,  0x02040001,  0x02050004,  0x02040001, \   //;Widget node 0x20 :
                                     0x02050005,  0x02040080,  0x02050001,  0x0204A9B8      //;Widget node 0x20 - 1 :
#endif

DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_1);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_1);

DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_2);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_2);

DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_3);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_3);

DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_4);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_4);

/*
#ifdef OEM_VERB_TABLE_ID_1
#ifdef OEM_VERB_TABLE_1_HEADER2
//
// 2 verb tables
//
DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_2 (OEM_VERB_TABLE_ID_1);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_2 (OEM_VERB_TABLE_ID_1);
#else
//
// 1 verb table
//
DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_1);
COLLECT_DEFINE_VERB_TABLE_LOCAL_HEADER_DATA_1 (OEM_VERB_TABLE_ID_1);
#endif
#endif
*/
//[-end-151123-IB08450329-modify]//

/**
  This OemService is part of setting Verb Table. The function is created for setting verb table
  to support Multi-Sku and return the table to common code to program.

  @param[out]  *VerbTableHeaderDataAddress    A pointer to VerbTable data/header

  @retval      EFI_UNSUPPORTED                Returns unsupported by default.
  @retval      EFI_SUCCESS                    The service is customized in the project.
  @retval      EFI_MEDIA_CHANGED              The value of IN OUT parameter is changed.
  @retval      Others                         Depends on customization.
**/
EFI_STATUS
OemSvcGetVerbTable (
  OUT COMMON_CHIPSET_AZALIA_VERB_TABLE      **VerbTableHeaderDataAddress
  )
{
  UINT32                              Data32;
//AMIC-AMP GPIO32,33 (0, 0)
//  *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_1);
//AMIC-Codec GPIO32,33 (1, 0)
//  *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_2);
//DMIC-AMP GPIO32,33 (0, 1)
//  *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_3);
//DMIC-Codec GPIO32,33 (1, 1)
//  *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_4);

  Data32 = GpioPadRead (N_GPIO_32);
  Data32 &= BIT0;

//WriteCmos8 (0x6C,0x50);

  if (Data32) {    
    //Codec
    Data32 = GpioPadRead (N_GPIO_33);
    Data32 &= BIT0;
    if (Data32) {  
      //DMIC
        *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_4);
//      WriteCmos8 (0x6C,0x51); 
    } else {
      //AMIC
        *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_2);
//      WriteCmos8 (0x6C,0x52);
    }
  } else {
    //AMP
    Data32 = GpioPadRead (N_GPIO_33);
    Data32 &= BIT0;
    if (Data32) {  
      //DMIC
        *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_3);
//      WriteCmos8 (0x6C,0x53);
    } else {
      //AMIC
        *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_1);
//      WriteCmos8 (0x6C,0x54);
    }
  }


  return EFI_MEDIA_CHANGED;

}
