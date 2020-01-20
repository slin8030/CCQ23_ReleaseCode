@REM
@REM Project build batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off

REM [-start-160713-IB07400758-remove]REM
REM title InsydeH2O 5.0 Build Environment
REM [-end-160713-IB07400758-remove]REM

REM Select Tool Chain, Please Refer \BaseTools\H2ORev50.bat for more detail
set TOOL_CHAIN=VS2015x86

REM
REM Workaround SDK version of VS2015 tool chain for H2ORev50.bat
REM
set SDK_DIR=Program Files (x86)\Windows Kits\10

REM
REM Workaround SDK version path of VS2015 tool chain for H2ORev50.bat
REM Please check your SDK path, for example:
REM   C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10586.0
REM   C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0
REM   => SDK version path = 10.0.10586.0
REM
set SDK_VERSION_PATH=10.0.14393.0

REM Setting Tool Chain Path, example: "B:\BuildCode\DEVTLS-T190"
REM Tool Chain Drive (C:, D:, E:, etc..)
set TOOL_DRV=C:

REM Tool Chain folder Path
REM set DEVTLS_DIR=BuildCode\DEVTLS-T190

REM ASL Compiler Path, example: "B:\BuildCode\DEVTLS-T190\Asl\Acpi5.0"
set ASL_PATH=B:\BuildCode\DEVTLS-T190\Asl\Acpi5.0

REM CRB Board Name
set CRB_BOARD_NAME=OxbowHill

REM SOC Name
set SOC_NAME=ApolloLakeI

REM [-start-160421-IB07400719-add]REM
REM 
REM XML file for SPI & eMMC (In $(WORKSPACE)\ApolloLakeBoardPkg\Stitch\CSE\$(TXE_Version)\APLK\FIT\xml)
REM Note:BIOS name is XML file name
REM 
set INPUT_FIT_XML_FOR_SPI=bxt_spi_8mb_OxbowHill
set INPUT_FIT_XML_FOR_EMMC=bxt_emmc_8mb_OxbowHill
REM [-end-160421-IB07400719-add]REM

REM BIOS Image Name in BIOS folder. If not defined, use SOC Name & CRB borad Name.
REM set BIOS_NAME=%SOC_NAME%

REM Include user path, example: "B:\BuildCode\DEVTLS-T190\MyBAT5.0"
path=B:\BuildCode\DEVTLS-T190\MyBAT5.0;%path%

REM Call Original ProjectBuild.bat in Board Package
ProjectBuild.bat
