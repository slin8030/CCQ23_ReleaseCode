@REM ******************************************************************************
@REM * Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off
SetLocal EnableDelayedExpansion EnableExtensions

set OUTPUT_FILE_PATH=%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%

REM #===============================================================================#
REM #                                                                               #
REM # Gen Config EMMC GPP batch and relative files                                  #
REM #                                                                               #
REM #===============================================================================#
REM

set GEN_BATCH_FILE=%OUTPUT_FILE_PATH%\DNX_CONFIG_EMMC_GPP.bat

@FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
  IF %%a==DEFINE (
    set CSE_Version=%%b
  )
)

REM
REM Copy DNX files
REM	
pushd ..\stitch
if exist CSE\%CSE_Version%\APLK\DnX\DNX_Module_%SOC_STEPPING%.bin copy CSE\%CSE_Version%\APLK\DnX\DNX_Module_%SOC_STEPPING%.bin ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%\DNX_Module.bin > NUL
if exist CSE\%CSE_Version%\APLK\DnX\cfgpart.xml copy CSE\%CSE_Version%\APLK\DnX\cfgpart.xml ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%\cfgpart.xml > NUL
popd

if exist %GEN_BATCH_FILE% goto GenDnxConfigGppDone

echo @REM ******************************************************************************                      > %GEN_BATCH_FILE%
echo @REM * Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.                             >> %GEN_BATCH_FILE%
echo @REM *                                                                                                   >> %GEN_BATCH_FILE%
echo @REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,                      >> %GEN_BATCH_FILE%
echo @REM * transmit, broadcast, present, recite, release, license or otherwise exploit                       >> %GEN_BATCH_FILE%
echo @REM * any part of this publication in any form, by any means, without the prior                         >> %GEN_BATCH_FILE%
echo @REM * written permission of Insyde Software Corporation.                                                >> %GEN_BATCH_FILE%
echo @REM *                                                                                                   >> %GEN_BATCH_FILE%
echo @REM ******************************************************************************                      >> %GEN_BATCH_FILE%
echo @REM                                                                                                     >> %GEN_BATCH_FILE%
echo @REM Auto Gen DnX Config eMMC GPP batch file                                                             >> %GEN_BATCH_FILE%
echo @REM                                                                                                     >> %GEN_BATCH_FILE%
echo @echo off                                                                                                >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo set PlatformFlashToolPath32="C:\Program Files\Intel\Platform Flash Tool"                                 >> %GEN_BATCH_FILE%
echo set PlatformFlashToolPath64="C:\Program Files (x86)\Intel\Platform Flash Tool"                           >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo if exist %%PlatformFlashToolPath64%% goto DoConfigGpp                                                    >> %GEN_BATCH_FILE%
echo if exist %%PlatformFlashToolPath32%% goto DoConfigGpp                                                    >> %GEN_BATCH_FILE%
echo goto Error                                                                                               >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :DoConfigGpp                                                                                             >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo echo Configure the GPPs on an eMMC based platform.....                                                   >> %GEN_BATCH_FILE%
echo dnxFwDownloader.exe --command configpart --fw_dnx DNX_Module.bin --path cfgpart.xml --device emmc --idx 0 >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo pause                                                                                                    >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo goto Exit                                                                                                >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :Error                                                                                                   >> %GEN_BATCH_FILE%
echo echo. Please install Intel Platform Flash Tool                                                           >> %GEN_BATCH_FILE%
echo pause                                                                                                    >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :Exit                                                                                                    >> %GEN_BATCH_FILE%

:GenDnxConfigGppDone

REM #===============================================================================#
REM #                                                                               #
REM # Gen DnX eMMC flash batch files                                                #
REM #                                                                               #
REM #===============================================================================#
REM

set DNX_BIOS_NAME=DNX_EMMC_%OUTPUT_BIOS_NAME%.bin
set GEN_BATCH_FILE=%OUTPUT_FILE_PATH%\DNX_EMMC_%OUTPUT_BIOS_NAME%.bat

echo @REM ******************************************************************************                      > %GEN_BATCH_FILE%
echo @REM * Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.                             >> %GEN_BATCH_FILE%
echo @REM *                                                                                                   >> %GEN_BATCH_FILE%
echo @REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,                      >> %GEN_BATCH_FILE%
echo @REM * transmit, broadcast, present, recite, release, license or otherwise exploit                       >> %GEN_BATCH_FILE%
echo @REM * any part of this publication in any form, by any means, without the prior                         >> %GEN_BATCH_FILE%
echo @REM * written permission of Insyde Software Corporation.                                                >> %GEN_BATCH_FILE%
echo @REM *                                                                                                   >> %GEN_BATCH_FILE%
echo @REM ******************************************************************************                      >> %GEN_BATCH_FILE%
echo @REM                                                                                                     >> %GEN_BATCH_FILE%
echo @REM Auto Gen DnX eMMC flash batch file                                                                  >> %GEN_BATCH_FILE%
echo @REM                                                                                                     >> %GEN_BATCH_FILE%
echo @echo off                                                                                                >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo set PlatformFlashToolPath32="C:\Program Files\Intel\Platform Flash Tool"                                 >> %GEN_BATCH_FILE%
echo set PlatformFlashToolPath64="C:\Program Files (x86)\Intel\Platform Flash Tool"                           >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo if exist %%PlatformFlashToolPath64%% goto DoDnxFlash                                                     >> %GEN_BATCH_FILE%
echo if exist %%PlatformFlashToolPath32%% goto DoDnxFlash                                                     >> %GEN_BATCH_FILE%
echo goto Error                                                                                               >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :DoDnxFlash                                                                                              >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo echo Clear eMMC RPMB/GPP4 .....                                                                          >> %GEN_BATCH_FILE%
echo dnxFwDownloader.exe --command clearrpmb --fw_dnx DNX_Module.bin --device emmc --idx 0                    >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo echo Flash eMMC Binary .....                                                                             >> %GEN_BATCH_FILE%
echo dnxFwDownloader.exe --command downloadfwos --fw_dnx DNX_Module.bin --fw_image %DNX_BIOS_NAME% --flags 0  >> %GEN_BATCH_FILE%                                                                                >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo pause                                                                                                    >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo goto Exit                                                                                                >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :Error                                                                                                   >> %GEN_BATCH_FILE%
echo echo. Please install Intel Platform Flash Tool                                                           >> %GEN_BATCH_FILE%
echo pause                                                                                                    >> %GEN_BATCH_FILE%
echo.                                                                                                         >> %GEN_BATCH_FILE%
echo :Exit                                                                                                    >> %GEN_BATCH_FILE%

EndLocal
