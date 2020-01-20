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
#define OEM_VERB_TABLE_ID_1          1

//[-start-151123-IB08450329-modify]//
//
//  VerbTable: (Realtek ALC298)
//  Revision ID = 0xff
//  Codec Vendor: 0x10EC0286
//

//
//    NID 0x12 : 0x90A60140
//    NID 0x13 : 0x40000000
//    NID 0x14 : 0x90170120
//    NID 0x17 : 0x411111F0
//    NID 0x18 : 0x04A11030
//    NID 0x19 : 0x411111F0
//    NID 0x1A : 0x411111F0
//    NID 0x1D : 0x4066A545
//    NID 0x1E : 0x411111F0
//    NID 0x1F : 0x411111F0
//    NID 0x21 : 0x04211010
//

#define OEM_VERB_TABLE_1_HEADER1     0x10EC0298, \
                                     0x00000000, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000F,     \
                                     0x0000

#define OEM_VERB_TABLE_1_DATA1       0x0017209A,  0x0017209A,  0x001722EC,  0x00172310, \
                                     0x0017FF00,  0x0017FF00,  0x0017FF00,  0x0017FF00, \
                                     0x01271C40,  0x01271D01,  0x01271EA6,  0x01271F90, \
                                     0x01371C00,  0x01371D00,  0x01371E00,  0x01371F40, \
                                     0x01471C20,  0x01471D01,  0x01471E17,  0x01471F90, \
                                     0x01771CF0,  0x01771D11,  0x01771E11,  0x01771F41, \
                                     0x01871C30,  0x01871D10,  0x01871EA1,  0x01871F04, \
                                     0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                     0x01A71CF0,  0x01A71D11,  0x01A71E11,  0x01A71F41, \
                                     0x01D71C45,  0x01D71DA5,  0x01D71E66,  0x01D71F40, \
                                     0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41, \
                                     0x01F71CF0,  0x01F71D11,  0x01F71E11,  0x01F71F41, \
                                     0x02171C10,  0x02171D10,  0x02171E21,  0x02171F04, \
                                     0x0205004F,  0x02045009,  0x0205004F,  0x02045009, \
                                     0x0205006D,  0x02048908,  0x0205006D,  0x02048908 

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

//[-start-171219-IB07400935-add]//
//
//  VerbTable: (Realtek ALC269)
//  Revision ID = 0xff
//  Codec Vendor: 0x10EC0269
//

//
//    SSID     : 0x10EC111A
//    NID 0x12 : 0xB7A60140
//    NID 0x14 : 0x90170120
//    NID 0x15 : 0x04214010
//    NID 0x17 : 0x40000000
//    NID 0x18 : 0x04A19030
//    NID 0x19 : 0x411111F0
//    NID 0x1A : 0x411111F0
//    NID 0x1B : 0x411111F0
//    NID 0x1D : 0x40E4A105
//    NID 0x1E : 0x411111F0
//
#define OEM_VERB_TABLE_1_HEADER2     0x10EC0269, \
                                     0x00000000, \
                                     0xFF,       \
                                     0x01,       \
                                     0x000B,     \
                                     0x0000

#define OEM_VERB_TABLE_1_DATA2       0x0017201A,  0x00172111,  0x001722EC,  0x00172310, \
                                     0x01271C40,  0x01271D01,  0x01271EA6,  0x01271FB7, \
                                     0x01471C20,  0x01471D01,  0x01471E17,  0x01471F90, \
                                     0x01571C10,  0x01571D40,  0x01571E21,  0x01571F04, \
                                     0x01771C00,  0x01771D00,  0x01771E00,  0x01771F40, \
                                     0x01871C30,  0x01871D90,  0x01871EA1,  0x01871F04, \
                                     0x01971CF0,  0x01971D11,  0x01971E11,  0x01971F41, \
                                     0x01A71CF0,  0x01A71D11,  0x01A71E11,  0x01A71F41, \
                                     0x01B71CF0,  0x01B71D11,  0x01B71E11,  0x01B71F41, \
                                     0x01D71C05,  0x01D71DA1,  0x01D71EE4,  0x01D71F40, \
                                     0x01E71CF0,  0x01E71D11,  0x01E71E11,  0x01E71F41
//[-end-171219-IB07400935-add]//
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
  *VerbTableHeaderDataAddress = VERB_TABLE_HEADER_DATA_BUFFER_ADDRESS (OEM_VERB_TABLE_ID_1);

//[-start-160318-IB03090425-modify]//
//[-start-171219-IB07400935-modify]//
  return EFI_MEDIA_CHANGED; //For using OEM verb table
  //return EFI_UNSUPPORTED; //For using chipset default verb table
//[-end-171219-IB07400935-modify]//
//[-end-160318-IB03090425-modify]//
}
