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
SetLocal EnableDelayedExpansion EnableExtensions

if not DEFINED CSE_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_KIT_VERSION" ..\Project.env') DO (
    IF %%a==DEFINE (
      set CSE_Version=%%b
    )
  )
)
REM [-start-160713-IB07400757-add]REM
if not DEFINED TXE_FW_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set TXE_FW_Version=%%b
REM [-start-170526-IB07400873-add]REM
      set CSE_Version=%%b
REM [-end-170526-IB07400873-add]REM
    )
  )
)
REM [-end-160713-IB07400757-add]REM
REM [-start-160601-IB07400738-modify]REM
if not DEFINED ISH_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_ISH_FW_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set ISH_Version=%%b
    )
  )
)
REM [-start-180503-IB07400964-add]REM
if not DEFINED ISH_PDT_FILE (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_ISH_FW_PDT_FILE_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set ISH_PDT_FILE=%%b
    )
  )
)
REM [-end-180503-IB07400964-add]REM
REM [-start-180117-IB07400945-add]REM
if not DEFINED IUINT_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_IUNIT_FW_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set IUINT_Version=%%b
    )
  )
)
REM [-end-180117-IB07400945-add]REM

if not DEFINED PMC_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_PMC_FW_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set PMC_Version=%%b
    )
  )
)
if not DEFINED Patch_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_MICROCODE_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
    IF %%a==DEFINE (
      set Patch_Version=%%b
    )
  )
)
REM [-start-170825-IB07400902-add]REM
if %SOC_STEPPING%==BX (
  if not DEFINED Patch2_Version (
    @FOR /f "tokens=1,4" %%a in ('find "INTEL_MICROCODE_VERSION_EX" ..\Project.env') DO (
      IF %%a==DEFINE (
        set Patch2_Version=%%b
      )
    )
  )
)
REM [-end-170825-IB07400902-add]REM
REM [-start-170330-IB07400853-add]REM
if not DEFINED Ifwi_Smip_Version (
  @FOR /f "tokens=1,4" %%a in ('find "INTEL_SMIP_VERSION" ..\Project.env') DO (
    IF %%a==DEFINE (
      set Ifwi_Smip_Version=%%b
    )
  )
)
REM [-end-170330-IB07400853-add]REM
REM [-start-160629-IB07400750-remove]REM
REM if not DEFINED Smip_PMC_Version (
REM   @FOR /f "tokens=1,4" %%a in ('find "INTEL_PMC_SMIP_VERSION_%SOC_STEPPING%" ..\Project.env') DO (
REM     IF %%a==DEFINE (
REM       set Smip_PMC_Version=%%b
REM     )
REM   )
REM )
REM [-end-160629-IB07400750-remove]REM
REM [-end-160601-IB07400738-modify]REM

REM [-start-160713-IB07400757-add]REM
REM 
REM Error Check
REM
if not exist CSE\%CSE_Version%\APLK\FIT\xml\%FIT_INPUT_XML%.xml (
  echo Error: CSE\%CSE_Version%\APLK\FIT\xml\%FIT_INPUT_XML%.xml do not exist!!
  pause
  goto Error
)
if not defined OUTPUT_BIOS_NAME set OUTPUT_BIOS_NAME=%FIT_INPUT_XML%

echo.------ FIT Input Binary Info (%SOC_STEPPING%) ----------
@echo BIOS Name           : %OUTPUT_BIOS_NAME%
@echo FIT TARGET XML FILE : %FIT_INPUT_XML%
@echo TXE FW Kit Version  : %CSE_Version%
@echo TXE FW Version      : %TXE_FW_Version%
REM [-start-180119-IB07400946-modify]REM
if "%ISH_Version%" NEQ "NA" (
@echo ISH FW Version      : %ISH_Version%
)
REM [-start-180503-IB07400964-add]REM
if "%ISH_PDT_FILE%" NEQ "NA" (
@echo ISH PDT BOM file    : %ISH_PDT_FILE%
)
REM [-end-180503-IB07400964-add]REM
if "%IUINT_Version%" NEQ "NA" (
@echo IUnit FW Version    : %IUINT_Version%
)
REM [-end-180119-IB07400946-modify]REM
@echo PMC FW Version      : %PMC_Version%
@echo Microcode1 Version  : %Patch_Version%
REM [-start-170825-IB07400902-add]REM
if %SOC_STEPPING%==BX (
@echo Microcode2 Version  : %Patch2_Version%
)
REM [-end-170825-IB07400902-add]REM
echo.--------------------------------------------
REM [-end-160713-IB07400757-add]REM

if not exist CSE\%CSE_Version%\APLK\FIT\input mkdir CSE\%CSE_Version%\APLK\FIT\input
REM [-start-160629-IB07400750-modify]REM
if not exist ..\BIOS\bios.bin (
  echo BIOS\bios.bin do not exist...
  pause
  goto Error
)
copy /y ..\BIOS\bios.bin CSE\%CSE_Version%\APLK\FIT\input\bios.bin > NUL
if not exist ..\BIOS\oemkeymn2.bin (
  echo BIOS\oemkeymn2.bin do not exist...
  pause
  goto Error
)
copy /y ..\BIOS\oemkeymn2.bin CSE\%CSE_Version%\APLK\FIT\input\oemkeymn2.bin > NUL 
REM [-start-160601-IB07400738-modify]REM
REM [-start-160713-IB07400757-modify]REM
if %SOC_STEPPING%==AX (
  if not exist CSE\%TXE_FW_Version%\APLK\Silicon\cse_image.bin (
    echo CSE\%TXE_FW_Version%\APLK\Silicon\cse_image.bin do not exist...
    pause
    goto Error
  )
  copy /y CSE\%TXE_FW_Version%\APLK\Silicon\cse_image.bin CSE\%CSE_Version%\APLK\FIT\input\cse_image.bin > NUL
)
if %SOC_STEPPING%==BX (
  if not exist CSE\%TXE_FW_Version%\APLK\Silicon\cse_image_b0.bin (
    echo CSE\%TXE_FW_Version%\APLK\Silicon\cse_image_b0.bin do not exist...
    pause
    goto Error
  )
  copy /y CSE\%TXE_FW_Version%\APLK\Silicon\cse_image_b0.bin CSE\%CSE_Version%\APLK\FIT\input\cse_image.bin > NUL
)
REM [-end-160713-IB07400757-modify]REM
REM [-end-160601-IB07400738-modify]REM
REM [-start-180119-IB07400946-modify]REM
REM [-start-180503-IB07400964-modify]REM
if "%ISH_Version%" == "NA" goto SkipIshFw
if not exist ISH\%ISH_Version%\%ISH_Version%.bin (
  echo ISH\%ISH_Version%\%ISH_Version%.bin do not exist...
  pause
  goto Error
)
copy /y ISH\%ISH_Version%\%ISH_Version%.bin CSE\%CSE_Version%\APLK\FIT\input\ISHC.bin > NUL

if not exist ISH\%ISH_Version%\%ISH_PDT_FILE% (
  echo ISH\%ISH_Version%\%ISH_PDT_FILE% do not exist...
  pause
  goto Error
)
copy /y ISH\%ISH_Version%\%ISH_PDT_FILE% CSE\%CSE_Version%\APLK\FIT\input\INTC_pdt_BXT_AOB_BOM_SENSORS > NUL
:SkipIshFw
REM [-end-180503-IB07400964-modify]REM
REM [-end-180119-IB07400946-modify]REM
REM [-start-160816-IB07400771-remove]REM
REM [-start-180117-IB07400945-add]REM
REM [-start-180119-IB07400946-modify]REM\
if "%IUINT_Version%" == "NA" goto SkipIunitFw
if not exist IUnit\%IUINT_Version%\IUnit.bin (
  echo IUnit\%IUINT_Version%\IUnit.bin do not exist...
  pause
  goto Error
)
copy /y IUnit\%IUINT_Version%\IUnit.bin CSE\%CSE_Version%\APLK\FIT\input\IUnit.bin > NUL
:SkipIunitFw
REM [-end-180119-IB07400946-modify]REM
REM [-end-180117-IB07400945-add]REM
REM if not exist CSE\%CSE_Version%\APLK\smip_cse.bin (
REM   echo CSE\%CSE_Version%\APLK\smip_cse.bin not exist...
REM   pause
REM   goto Error
REM )
REM copy /y CSE\%CSE_Version%\APLK\smip_cse.bin  CSE\%CSE_Version%\APLK\FIT\input > NUL
REM [-end-160816-IB07400771-remove]REM
REM [-start-170330-IB07400853-modify]REM
if not exist SMIP\%Ifwi_Smip_Version%.bin (
REM [-start-160714-IB07400758-modify]REM
  echo SMIP\%Ifwi_Smip_Version%.bin do not exist...
REM [-end-160714-IB07400758-modify]REM
  pause
  goto Error
)
copy /y SMIP\%Ifwi_Smip_Version%.bin CSE\%CSE_Version%\APLK\FIT\input\smip_iafw.bin > NUL
REM [-end-170330-IB07400853-modify]REM
if not exist PMC\%PMC_Version%\Silicon\pmcp.bin (
  echo PMC\%PMC_Version%\Silicon\pmcp.bin do not exist...
  pause
  goto Error
)
copy /y PMC\%PMC_Version%\Silicon\pmcp.bin CSE\%CSE_Version%\APLK\FIT\input > NUL
Patch\GenFw.exe -m Patch\%Patch_Version%.inc -o Patch\%Patch_Version%.bin > NUL
if not exist Patch\%Patch_Version%.bin (
  echo Patch\%Patch_Version%.bin do not exist...
  pause
  goto Error
)
copy /y Patch\%Patch_Version%.bin CSE\%CSE_Version%\APLK\FIT\input\upatch1.bin > NUL
REM [-end-160629-IB07400750-modify]REM
REM [-start-170825-IB07400902-add]REM
if %SOC_STEPPING%==BX (
  Patch\GenFw.exe -m Patch\%Patch2_Version%.inc -o Patch\%Patch2_Version%.bin > NUL
  if not exist Patch\%Patch2_Version%.bin (
    echo Patch\%Patch2_Version%.bin do not exist...
    pause
    goto Error
  )
  copy /y Patch\%Patch2_Version%.bin CSE\%CSE_Version%\APLK\FIT\input\upatch2.bin > NUL
)
REM [-end-170825-IB07400902-add]REM

pushd CSE\%CSE_Version%\APLK\FIT
  fit -b -f xml/%FIT_INPUT_XML%.xml -o %OUTPUT_BIOS_NAME%.bin > NUL
popd
REM [-start-160912-IB07400783-add]REM
if NOT "%errorlevel%" == "0" (
  echo FIT error: %errorlevel%
  pause
  type CSE\%CSE_Version%\APLK\FIT\fit.log
  pause
  goto error
)
REM [-end-160912-IB07400783-add]REM
REM [-start-190220-IB07401083-add]REM
REM
REM Gen OBB layout
REM
set TXE_FW_SOC_NAME=APLK
@if not exist CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.map goto GenObbLyoutDone

@set ObbAddr1=""

@for /f "tokens=1,3,4" %%a in ('find "OBB Partition" CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.map') do (
  @if %%c == OBB (
    @if !ObbAddr1! == "" (
      @set ObbAddr1=%%a
      @set ObbSize1=%%b
    ) else (
      @set ObbAddr2=%%a
      @set ObbSize2=%%b
    ) 
  )
)

@if exist ..\BIOS\ObbLayout.exe ..\BIOS\ObbLayout.exe !ObbAddr1! !ObbSize1! !ObbAddr2! !ObbSize2!
@if exist ObbLayout.h (
  echo. >> CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.map
  type ObbLayout.h >> CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.map
  del ObbLayout.h
)
:GenObbLyoutDone
REM [-end-190220-IB07401083-add]REM
REM [-start-180806-IB07400995-add]REM
if not exist ..\..\GenSecurityFlashBiosPkg\GenSecurityFlashBiosPkg.bat goto SkipH2oSignBios

set TXE_FW_SOC_NAME=APLK
REM
REM Sign BIOS with 32-bit secure flash tool binary.
REM
copy /y CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.bin ..\..\GenSecurityFlashBiosPkg\bios32.bin > NUL
pushd ..\..\GenSecurityFlashBiosPkg\
call GenSecurityFlashBiosPkg.bat > NUL
popd
if exist ..\..\GenSecurityFlashBiosPkg\bios32_signed.bin (
  copy /y ..\..\GenSecurityFlashBiosPkg\bios32_signed.bin CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%_SIGNED32.bin > NUL
)
REM
REM Sign BIOS with 64-bit secure flash tool binary.
REM
copy /y CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%.bin ..\..\GenSecurityFlashBiosPkg\bios64.bin > NUL
pushd ..\..\GenSecurityFlashBiosPkg\
call GenSecurityFlashBiosPkg.bat > NUL
popd
if exist ..\..\GenSecurityFlashBiosPkg\bios64_signed.bin (
  copy /y ..\..\GenSecurityFlashBiosPkg\bios64_signed.bin CSE\%CSE_Version%\%TXE_FW_SOC_NAME%\FIT\%OUTPUT_BIOS_NAME%_SIGNED64.bin > NUL
)
if exist ..\..\GenSecurityFlashBiosPkg\bios32.bin del ..\..\GenSecurityFlashBiosPkg\bios32.bin /f > NUL
if exist ..\..\GenSecurityFlashBiosPkg\bios32_signed.bin del ..\..\GenSecurityFlashBiosPkg\bios32_signed.bin /f > NUL
if exist ..\..\GenSecurityFlashBiosPkg\bios64.bin del ..\..\GenSecurityFlashBiosPkg\bios64.bin /f > NUL
if exist ..\..\GenSecurityFlashBiosPkg\bios64_signed.bin del ..\..\GenSecurityFlashBiosPkg\bios64_signed.bin /f > NUL

:SkipH2oSignBios
REM [-end-180806-IB07400995-add]REM
REM [-start-170214-IB07400840-modify]REM
if not exist ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING% mkdir ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%
if exist CSE\%CSE_Version%\APLK\FIT\DNX_SPI_%OUTPUT_BIOS_NAME%.bin del CSE\%CSE_Version%\APLK\FIT\DNX_SPI_%OUTPUT_BIOS_NAME%.bin > NUL
if exist CSE\%CSE_Version%\APLK\FIT\dnx_spi.bin rename CSE\%CSE_Version%\APLK\FIT\dnx_spi.bin DNX_SPI_%OUTPUT_BIOS_NAME%.bin> NUL
if exist CSE\%CSE_Version%\APLK\FIT\DNX_EMMC_%OUTPUT_BIOS_NAME%.bin del CSE\%CSE_Version%\APLK\FIT\DNX_EMMC_%OUTPUT_BIOS_NAME%.bin > NUL
if exist CSE\%CSE_Version%\APLK\FIT\dnx_emmc.bin rename CSE\%CSE_Version%\APLK\FIT\dnx_emmc.bin DNX_EMMC_%OUTPUT_BIOS_NAME%.bin > NUL
copy /y CSE\%CSE_Version%\APLK\FIT\*.bin ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING% > NUL
copy /y CSE\%CSE_Version%\APLK\FIT\*.map ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING% > NUL
del ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%\Smip* /q
del ..\BIOS\%OUTPUT_BIOS_FOLDER%\%FIT_INPUT_XML%\%SOC_STEPPING%\vsccommn.bin /q
REM [-end-170214-IB07400840-modify]REM
 
REM ------------  Clear Temp files  ------------
REM [-start-160912-IB07400783-add]REM
@FOR /f "tokens=1,4" %%a in ('find "INTEL_FIT_METADATA_CLEANUP" ..\Project.env') DO (
  IF %%a==DEFINE (
    IF %%b==NO (
      goto StitchBiosOnlyDone
    )
  )
)
REM [-end-160912-IB07400783-add]REM

rmdir CSE\%CSE_Version%\APLK\FIT\input /s /q
rmdir CSE\%CSE_Version%\APLK\FIT\Int /s /q
REM [-start-170214-IB07400840-modify]REM
if exist CSE\%CSE_Version%\APLK\FIT\DnxRecoveryImage.xml del CSE\%CSE_Version%\APLK\FIT\DnxRecoveryImage.xml /q 
REM [-end-170214-IB07400840-modify]REM
del CSE\%CSE_Version%\APLK\FIT\*.log /q
del CSE\%CSE_Version%\APLK\FIT\Smip* /q
move CSE\%CSE_Version%\APLK\FIT\vsccommn.bin CSE\%CSE_Version%\APLK\FIT\vsccommn.bi > NUL
del CSE\%CSE_Version%\APLK\FIT\*.bin /q
del CSE\%CSE_Version%\APLK\FIT\*.map /q
move CSE\%CSE_Version%\APLK\FIT\vsccommn.bi CSE\%CSE_Version%\APLK\FIT\vsccommn.bin > NUL

REM [-start-160913-IB07400783-modify]REM
goto StitchBiosOnlyDone

:Error
exit /b 1

:StitchBiosOnlyDone
REM [-end-160913-IB07400783-modify]REM
EndLocal
