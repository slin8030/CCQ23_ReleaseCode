//;******************************************************************************
//;* Copyright (c) 1983-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; 
//; Module Name:
//; 
//;   xtuCpuItem.c
//; 
//; Abstract:
//; 
//;   
//; 

#include "xtuCpuItem.h"


EFI_STATUS
XTUCPUOption (
  IN EFI_HII_HANDLE                     HiiHandle
  )
{
  SYSTEM_CONFIGURATION                  SetupNvData;
  UINT16                                QuestionId = 0;
  EFI_HII_PROTOCOL                      *Hii;
  VOID                                  *HobList;
  XTU_INFO_HOB                          *XTUInfoDataPtr = NULL;
  EFI_PEI_HOB_POINTERS                  Hob;
  UINTN                                 BufferSize;
  EFI_STATUS                            Status;
  UINT8                                 Index = 0;
  UINT16                                factoryValue = 0;
  EFI_HII_UPDATE_DATA                   *UpdateData;
  EFI_IFR_NUMERIC                       *Numeric;
  ITEM_OPTION                           CpuItem[] =
  {
  	//
  	// 1 Core Ratio
  	//
  	{
  		(STRING_REF)STRING_TOKEN(STR_1_CORE_RATIO_LIMIT_STRING),  //PromptToken
  		(STRING_REF)STRING_TOKEN(STR_1_CORE_RATIO_LIMIT_HELP),    //HelpToken
  		38,                                                       //Max
  		12,                                                       //Min
  		1,                                                        //Step
  		CORE1_RATIO_LABEL                                         //LABEL
  	},
  	//
  	// 2 Core Ratio
  	//
  	{
  		(STRING_REF)STRING_TOKEN(STR_2_CORE_RATIO_LIMIT_STRING),  //PromptToken
  		(STRING_REF)STRING_TOKEN(STR_2_CORE_RATIO_LIMIT_HELP),    //HelpToken
  		38,                                                       //Max
  		12,                                                       //Min
  		1,                                                        //Step
  		CORE2_RATIO_LABEL                                         //LABEL
  	},
  	//
  	// 3 Core Ratio
  	//
  	{
  		(STRING_REF)STRING_TOKEN(STR_3_CORE_RATIO_LIMIT_STRING),  //PromptToken
  		(STRING_REF)STRING_TOKEN(STR_3_CORE_RATIO_LIMIT_HELP),    //HelpToken
  		38,                                                       //Max
  		12,                                                       //Min
  		1,                                                        //Step
  		CORE3_RATIO_LABEL                                         //LABEL
  	},
  	//
  	// 4 Core Ratio
  	//
  	{
  		(STRING_REF)STRING_TOKEN(STR_4_CORE_RATIO_LIMIT_STRING),  //PromptToken
  		(STRING_REF)STRING_TOKEN(STR_4_CORE_RATIO_LIMIT_HELP),    //HelpToken
  		38,                                                       //Max
  		12,                                                       //Min
  		1,                                                        //Step
  		CORE4_RATIO_LABEL                                         //LABEL
  	},
  	{
  		0xff,
  		0xff,
  		0xff,
  		0xff,
  		0xff
  	}
  };
  DEBUG ((EFI_D_ERROR, " XTUCPUOption\n"));
  Hii = gSUBrowser->Hii;

    QuestionId = (UINT16)((UINTN)(&SetupNvData.TdpLimitValue) - (UINTN)(&SetupNvData));
  //
  // Get the HOB list.  If it is not present, then ASSERT.
  //
  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
  ASSERT_EFI_ERROR (Status);
  //
  // Search for the XTU  GUID HOB and copy to table structure
  //
  Hob.Raw = HobList;
  BufferSize = 0;
  while (!END_OF_HOB_LIST(Hob)) {
    Status = GetNextGuidHob (
               &HobList,
               &gXTUInfoHobGuid,
               &XTUInfoDataPtr,
               &BufferSize
               );
    Hob.Raw = GET_NEXT_HOB (Hob);

    if (Status == EFI_SUCCESS ) {
    	break;
    }
  }
  DEBUG ((EFI_D_ERROR, "In SCU  MSRTurboPowerCurrentLimitDefault %x:  \n",XTUInfoDataPtr->Data.MSRTurboPowerCurrentLimitDefault));
  DEBUG ((EFI_D_ERROR, "In SCU  MSRTurboRatioLimitDefault %x:  \n",XTUInfoDataPtr->Data.MSRTurboRatioLimitDefault));
  UpdateData = EfiLibAllocateZeroPool (0x1000);
  while (CpuItem[Index].Min != 0xFF){
    switch (Index) {
      //
    	//core1 ratio
    	//
    	case 0:
    		factoryValue = \
    		(UINT16)(XTUInfoDataPtr->Data.MSRTurboRatioLimitDefault & B_EFI_MSR_TURBO_RATIO_LIMIT_1C) >> N_EFI_MSR_TURBO_RATIO_LIMIT_1C;
    	  QuestionId = (UINT16)((UINTN)(&SetupNvData.RatioLimit1Core) - (UINTN)(&SetupNvData));
    	  break;

    	//
    	//core2 ratio
    	//
    	case 1:
    		factoryValue = \
    		(XTUInfoDataPtr->Data.MSRTurboRatioLimitDefault & B_EFI_MSR_TURBO_RATIO_LIMIT_2C) >> N_EFI_MSR_TURBO_RATIO_LIMIT_2C;
    	  QuestionId = (UINT16)((UINTN)(&SetupNvData.RatioLimit2Core) - (UINTN)(&SetupNvData));
    	  break;

    	//
    	//core3 ratio
    	//
    	case 2:
    		factoryValue = \
    		(XTUInfoDataPtr->Data.MSRTurboRatioLimitDefault & B_EFI_MSR_TURBO_RATIO_LIMIT_3C) >> N_EFI_MSR_TURBO_RATIO_LIMIT_3C;
    	  QuestionId = (UINT16)((UINTN)(&SetupNvData.RatioLimit3Core) - (UINTN)(&SetupNvData));
    	  break;

    	//
    	//core4 ratio
    	//
    	case 3:
    		factoryValue = \
    		(XTUInfoDataPtr->Data.MSRTurboRatioLimitDefault & B_EFI_MSR_TURBO_RATIO_LIMIT_4C) >> N_EFI_MSR_TURBO_RATIO_LIMIT_4C;
    	  QuestionId = (UINT16)((UINTN)(&SetupNvData.RatioLimit4Core) - (UINTN)(&SetupNvData));
    	  break;

    	default:
    		break;

    }
    DEBUG ((EFI_D_ERROR, "%x: factoryValue = %x  \n",Index,factoryValue));
    DEBUG ((EFI_D_ERROR, "%x: QuestionId = %x  \n",Index,QuestionId));
  	CreateNumericOpCode (
                   QuestionId,                                //QuestionId
                   1,                                         //DataWidth
                   (STRING_REF)CpuItem[Index].PromptToken,    //PromptToken
                   (STRING_REF)CpuItem[Index].HelpToken,      //HelpToken
                   (UINT16)CpuItem[Index].Min,                //Minimum
                   (UINT16)CpuItem[Index].Max,                //Maximum
                   (UINT16)CpuItem[Index].Step,               //Step
                   factoryValue,                              //Default
                   EFI_IFR_FLAG_DEFAULT,                      //Flags
                   0,                                         //Key
                   &UpdateData->Data                          //FormBuffer
                   );
    Numeric = (EFI_IFR_NUMERIC *)&UpdateData->Data;

    UpdateData->DataCount =  1;
    //
    // Add op-code
    //
    Hii->UpdateForm(Hii, HiiHandle, (EFI_FORM_LABEL)CpuItem[Index].label, TRUE, UpdateData);

    ++Index;
  }
  gBS->FreePool (UpdateData);
  DEBUG ((EFI_D_ERROR, "<-- XTUCPUOption\n"));

  return EFI_SUCCESS;
}
