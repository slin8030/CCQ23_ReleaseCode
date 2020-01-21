
#include "CompalPlatformInitPei.h"
#include <Ppi/Stall.h>
#include <PchRegs/PchRegsHda.h>
#include <Library/DebugLib.h>

#define HDA_BASE_ADDRESS                   0xF9000000

EFI_STATUS
CompalPlatformInitPeiIntelAudioVerbTable (
    IN    CONST EFI_PEI_SERVICES   **PeiServices,
    IN    PCH_AZALIA_VERB_TABLE    *CompalVerbTable,
    IN    UINT8                    NumberOfVerbTable
)
{
    EFI_STATUS                    Status;
    UINT32                        Index;
    UINT32                        VendorDeviceId;
    UINT8                         RevisionId;
    UINT8                         ByteReg;
    UINTN                         AzaliaBase;
    UINT32                        AzaliaSDINo;
    UINT32                        HDABar;
    UINT32                        TmpData;
    UINT32                        i;
    UINT32                        LoopTime;
    PCH_AZALIA_VERB_TABLE_HEADER  *VerbHeaderTable;
    UINT8                         VerbTableNum;
    EFI_PEI_STALL_PPI             *StallPpi;
    PCH_AZALIA_VERB_TABLE         *AzaliaVerbTable;

    AzaliaBase = PCI_LIB_ADDRESS (0,PCI_DEVICE_NUMBER_PCH_AZALIA,PCI_FUNCTION_NUMBER_PCH_AZALIA,0);
    //
    // Firstly Initialize Azalia to be not started.
    //

    //
    // Assign HDBAR ( Please take care for memory overlap )
    //
    HDABar = HDA_BASE_ADDRESS;

    //
    // System BIOS should ensure that the High Definition Audio HDBARD27:F0:Reg 10-17h contains a valid address value
    // and is enabled by setting D27:F0:Reg 04h[1].
    //
    PciWrite32(AzaliaBase+R_PCH_HDA_HDBARL,HDABar);
    PciWrite32(AzaliaBase+R_PCH_HDA_HDBARU,0);
    PciOr8((AzaliaBase+R_PCH_HDA_COMMAND),( UINT8 )( B_PCH_HDA_COMMAND_MSE | B_PCH_HDA_COMMAND_BME ));
    if ( AZALIA_DS == PCH_DEVICE_DISABLE ) {
        PciAnd8((AzaliaBase+R_PCH_HDA_DCKSTS),( UINT8 )( ~B_PCH_HDA_DCKSTS_DS ));
    } else if ( AZALIA_DA != PCH_DEVICE_DISABLE ) {
        if ( ( PciRead8 ( AzaliaBase+ R_PCH_HDA_DCKSTS ) & B_PCH_HDA_DCKSTS_DM ) == 0 ) {
            PciOr8((AzaliaBase+R_PCH_HDA_DCKCTL),( UINT8 )( B_PCH_HDA_DCKCTL_DA ));
        }
    }

    //
    // For HDMI codec initial, only for UMA.
    //
    if (  PciRead16 ( PCI_LIB_ADDRESS (0,2,0,0) )   != 0xFFFF ) {
        PciOr32 ( (AzaliaBase + 0xC4), (UINT32) BIT1);
        PciOr8  ( (AzaliaBase + 0x43), (UINT8) BIT6);
    }

    //
    // System BIOS should also ensure that the Controller Reset# bit of Global Control register
    // in memory-mapped space (HDBAR+08h[0]) is set to 1 and read back as 1.
    // Deassert the HDA controller RESET# to start up the link
    //
    MmioOr32((( UINTN )( HDABar )+R_HDA_GCTL),( UINT32 )( B_HDA_GCTL_CRST ));
    ByteReg =  PciRead8( AzaliaBase+ R_HDA_GCTL );

    //
    // Clear the "State Change Status Register" STATESTS bits for
    // each of the "SDIN Stat Change Status Flag"
    //
    MmioOr8( ( UINTN )( HDABar )+ R_HDA_STATESTS,( UINT8 )( AZALIA_MAX_SID_MASK ) );
    Status = ( **PeiServices ).LocatePpi (
                 PeiServices,
                 &gEfiPeiStallPpiGuid,
                 0,
                 NULL,
                 &StallPpi
             );
    //ASSERT_PEI_ERROR ( PeiServices, Status );

    //
    // Turn off the link and poll RESET# bit until it reads back as 0 to get hardware reset report
    //
    MmioAnd32( ( UINTN )( HDABar )+ R_HDA_GCTL,( UINT32 )( ~B_HDA_GCTL_CRST ) );
    for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
        if ( ( MmioRead32( HDABar+R_HDA_GCTL ) & B_HDA_GCTL_CRST ) == 0 ) {
            break;
        } else {
            StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
        }
    }

    if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
        //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Reset High Definition Audio ( Azalia ) Codec Time Out - 1! \n" ) );
        Status = EFI_TIMEOUT;
        goto ExitInitAzalia;
    }

    //
    // Turn on the link and wait for 1ms then poll RESET# bit until it reads back as 1
    //
    MmioOr32( HDABar+R_HDA_GCTL,B_HDA_GCTL_CRST );
    //
    // For some combo card that will need this delay because each codec has different latency to come out from RESET.
    // This delay can make sure all codecs be recognized by BIOS after RESET sequence.
    // Additional delay might be required to allow codec coming out of reset prior to subsequent operations,
    // please contact your codec vendor for detail. When clearing this bit and setting it afterward,
    // BIOS must ensure that minimum link timing requirements (minimum RESET# assertion time, etc.) are met..
    //
    StallPpi->Stall ( PeiServices, StallPpi,AZALIA_DEFAULT_RESET_WAIT_TIMER );

    for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
        if ( ( MmioRead32 ( HDABar+R_HDA_GCTL ) & B_HDA_GCTL_CRST ) == B_HDA_GCTL_CRST ) {
            break;
        } else {
            StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
        }
    }

    if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
        //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Reset High Definition Audio ( Azalia ) Codec Time Out - 2! \n" ) );
        Status = EFI_TIMEOUT;
        goto ExitInitAzalia;
    }

    //
    // Read the "State Change Status Register" STATESTS bits to find out if any SDIN is connected
    // to a codec.
    //

    for ( i = 0, ByteReg = 0 ; i < 6 ; i++ ) {
        ByteReg = MmioRead8(HDABar+R_HDA_STATESTS) & AZALIA_MAX_SID_MASK;
        if ( ByteReg != 0 ) {
            break;
        }

        StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
    }

    //
    // BIT3(1000) -- SDI3
    // BIT2(0100) -- SDI2
    // BIT1(0010) -- SDI1
    // BIT0(0001) -- SDI0
    //
    if ( ByteReg == 0 ) {
        //
        // No Azalia Detected
        //
        //
        // Turn off the link
        //
        //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "No Azalia device is detected.\n" ) );
        MmioAnd32(HDABar+R_HDA_GCTL,( UINT32 )( ~B_HDA_GCTL_CRST ));
        Status = EFI_DEVICE_ERROR;
        goto ExitInitAzalia;
    }

    for ( i = 0 ; i < AZALIA_MAX_SID_NUMBER ; i++, ByteReg >>= 1 ) {
        if ( ( ByteReg & 0x1 ) == 0 ) {
            //
            // SDI i has no Azalia Device
            //
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "MDC%d has no Azalia device.\n", i + 1 ) );
            continue;
        }

        AzaliaSDINo = i;

        //
        // PME Enable
        //
        if ( AZALIA_PME != PCH_DEVICE_DISABLE ) {
            MmioOr16(HDABar+R_HDA_WAKEEN,( UINT16 )( ( B_HDA_WAKEEN_SDI_0 | B_HDA_WAKEEN_SDI_1 | B_HDA_WAKEEN_SDI_2 | B_HDA_WAKEEN_SDI_3 ) ));
            PciOr32(AzaliaBase+R_PCH_HDA_PCS, ( UINT32 )( B_PCH_HDA_PCS_PMEE ) );
        }
        //
        // Poll the ICB bit of the IRS HDA register until it returns 0
        //
        for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
            if ( ( MmioRead16 ( HDABar+R_HDA_IRS ) & B_HDA_IRS_ICB ) == 0 ) {
                break;
            } else {
                StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
            }
        }

        if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Reset High Definition Audio ( Azalia ) Codec Time Out - 3! \n" ) );
            Status = EFI_TIMEOUT;
            goto ExitInitAzalia;
        }
        //
        // Verb:  31~28   27  26~20                   19~0
        //         CAd    1    NID   Verb Command and data
        //       0/1/2
        //
        // Read the Vendor ID/Device ID pair from the attached codec
        //
        MmioWrite32(HDABar+R_HDA_IC,( 0x000F0000 | ( AzaliaSDINo << 28 ) ));
        MmioOr16 ( ( UINTN )( HDABar )+( R_HDA_IRS ), ( UINT16 )( ( B_HDA_IRS_IRV | B_HDA_IRS_ICB ) ) );
        for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
            if ( ( MmioRead16 ( HDABar+R_HDA_IRS ) & ( B_HDA_IRS_IRV | B_HDA_IRS_ICB ) ) == 0x02 ) {
                break;
            } else {
                StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
            }
        }

        if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Reset High Definition Audio ( Azalia ) Codec Time Out - 4! \n" ) );
            Status = EFI_TIMEOUT;
            goto ExitInitAzalia;
        }

        VendorDeviceId = MmioRead32(HDABar+R_HDA_IR);
        //
        // Read the Revision ID from the attached codec
        //
        MmioWrite32(HDABar+R_HDA_IC,( 0x000F0002 | ( AzaliaSDINo << 28 ) ));
        MmioOr16 ( ( UINTN )( HDABar ) + ( R_HDA_IRS ), ( UINT16 )( ( B_HDA_IRS_IRV | B_HDA_IRS_ICB ) ) );
        for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
            if ( ( MmioRead16 ( HDABar+R_HDA_IRS ) & ( B_HDA_IRS_IRV | B_HDA_IRS_ICB ) ) == 0x02 ) {
                break;
            } else {
                StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
            }
        }

        if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Reset High Definition Audio ( Azalia ) Codec Time Out - 5! \n" ) );
            Status = EFI_TIMEOUT;
            goto ExitInitAzalia;

        }

        RevisionId = MmioRead8(HDABar+R_HDA_IR + 1);
        AzaliaVerbTable  = CompalVerbTable;

        //
        // Get the match codec verb table, RevID of 0xFF applies to all steppings.
        //
        for ( VerbTableNum = 0, VerbHeaderTable = NULL ; VerbTableNum < NumberOfVerbTable ; VerbTableNum++ ) {
            if (
                ( VendorDeviceId == AzaliaVerbTable[VerbTableNum].VerbTableHeader.VendorDeviceId ) &&
                (
                    ( RevisionId == AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId ) ||
                    ( AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId == 0xFF )
                )
            ) {
                VerbHeaderTable = &AzaliaVerbTable[VerbTableNum].VerbTableHeader;


                //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Detected High Definition Audio ( Azalia ) Codec with verb table, VendorID = 0x%X", VendorDeviceId ) );
                //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, " on MDC%d, revision = 0x%08x.\n", AzaliaSDINo + 1, RevisionId ) );
                //
                // Send the entire list of verbs in the matching verb table one by one to the codec
                //
                for ( Index = 0 ; Index < ( UINT32 )( ( VerbHeaderTable->NumberOfFrontJacks + VerbHeaderTable->NumberOfRearJacks ) * 4 ) ; Index++ ) {
                    //
                    // Wait for the codec to be free
                    //
                    for ( LoopTime = 0 ; LoopTime < AZALIA_MAX_LOOP_TIME ; LoopTime++ ) {
                        if ( ( MmioRead16 ( HDABar+R_HDA_IRS ) & B_HDA_IRS_ICB ) == 0 ) {
                            break;
                        } else {
                            StallPpi->Stall ( PeiServices, StallPpi, AZALIA_WAIT_PERIOD );
                        }
                    }

                    if ( LoopTime >= AZALIA_MAX_LOOP_TIME ) {
                        //
                        // Skip the Azalia verb table loading when find the verb table content is not
                        // properly matched with the HDA hardware, though IDs match.
                        //
                        //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Detected High Definition Audio ( Azalia ) Codec, error occurs during loading verb table.\n" ) );
                        break;
                    }
                    TmpData = AzaliaVerbTable[VerbTableNum].VerbTableData[Index] ;
                    MmioWrite32( HDABar+R_HDA_IC ,( TmpData | ( AzaliaSDINo << 28 ) ));
                    MmioOr16 ( ( UINTN )( HDABar )+( R_HDA_IRS ), ( UINT16 )( ( B_HDA_IRS_IRV | B_HDA_IRS_ICB ) ) );
                }
                break;
            }
        }
        if ( VerbTableNum >= NumberOfVerbTable ) {
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, "Detected High Definition Audio (Azalia) Codec, VendorID = 0x%08x on MDC%d,", VendorDeviceId, AzaliaSDINo + 1 ) );
            //PEI_DEBUG ( ( PeiServices, EFI_D_ERROR, " but no matching verb table found.\n" ) );
        }

    }
    //
    // end of for
    //
    Status          = EFI_SUCCESS;
    ByteReg         = PciRead8 ( AzaliaBase+ R_PCH_HDA_HDCTL );

ExitInitAzalia:

    //
    // Clear AZBAR and disable memory map access
    //
    PciAnd8 ( ( UINTN )( AzaliaBase )+ ( R_PCH_HDA_COMMAND ), ( UINT8 )( ~B_PCH_HDA_COMMAND_MSE ) );
    PciWrite32 ( AzaliaBase+ R_PCH_HDA_HDBARL,0);
    PciWrite32 ( AzaliaBase+ R_PCH_HDA_HDBARU,0);
    return EFI_SUCCESS;
}
