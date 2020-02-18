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

REM [-start-160712-IB07400757-add]REM
REM #===============================================================================#
REM #                                                                               #
REM # OEM Gen Full BIOS config                                                      #
REM #                                                                               #
REM #===============================================================================#
REM
REM INPUT_FIT_XML_FOR_SPI & INPUT_FIT_XML_FOR_EMMC set in Projectbuild_xxxx.bat
REM OEM can setting default XML file here:
REM set INPUT_FIT_XML_FOR_SPI=bxt_spi_8mb_OxbowHill for 8M BIOS image
REM set INPUT_FIT_XML_FOR_SPI=bxt_spi_16mb_OxbowHill for 16M BIOS image
REM set INPUT_FIT_XML_FOR_EMMC=bxt_emmc_8mb_OxbowHill
REM
set DEFAULT_INPUT_FIT_XML_FOR_SPI=bxt_spi_8mb_OxbowHill
set DEFAULT_INPUT_FIT_XML_FOR_EMMC=bxt_emmc_8mb_OxbowHill

REM #===============================================================================#
REM #                                                                               #
REM # CRB Gen Full BIOS config                                                      #
REM #                                                                               #
REM #===============================================================================#
set CRB_GEN_APL_RVP1_FULL_BIOS=YES
set CRB_GEN_APL_RVP2_FULL_BIOS=YES
set CRB_GEN_APLI_OXH_FULL_BIOS=YES
REM [-start-170214-IB07400840-add]REM
set CRB_GEN_BIOS_SIZE=8M
REM [-end-170214-IB07400840-add]REM

REM [-start-170214-IB07400840-add]REM
if %CRB_GEN_BIOS_SIZE%==8M (
  REM 
  REM For 8M BIOS 
  REM
  set OUTPUT_BIOS_FOLDER=BIOS_8M
  set CRB_APL_RVP1_XML_FOR_SPI=bxt_spi_8mb_RVP1
  set CRB_APL_RVP2_XML_FOR_SPI=bxt_spi_8mb_RVP2
  set CRB_APLI_OXH_XML_FOR_SPI=bxt_spi_8mb_OxbowHill
  set CRB_APL_RVP1_XML_FOR_EMMC=bxt_emmc_8mb_RVP1
  set CRB_APL_RVP2_XML_FOR_EMMC=bxt_emmc_8mb_RVP2
  set CRB_APLI_OXH_XML_FOR_EMMC=bxt_emmc_8mb_OxbowHill
)
if %CRB_GEN_BIOS_SIZE%==16M (
  REM 
  REM For 16M BIOS 
  REM
  set OUTPUT_BIOS_FOLDER=BIOS_16M
  set CRB_APL_RVP1_XML_FOR_SPI=bxt_spi_16mb_RVP1
  set CRB_APL_RVP2_XML_FOR_SPI=bxt_spi_16mb_RVP2
  set CRB_APLI_OXH_XML_FOR_SPI=bxt_spi_16mb_OxbowHill
  set CRB_APL_RVP1_XML_FOR_EMMC=bxt_emmc_8mb_RVP1
  set CRB_APL_RVP2_XML_FOR_EMMC=bxt_emmc_8mb_RVP2
  set CRB_APLI_OXH_XML_FOR_EMMC=bxt_emmc_8mb_OxbowHill
)
REM [-end-170214-IB07400840-add]REM

REM 
REM Search BIOS name
REM
if exist BiosNameList.txt del BiosNameList.txt
for /f %%i in ('dir bios_*.bin /b') do echo %%i >> BiosNameList.txt

REM
REM Check EMMC/SPI Gen setting
REM
@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_SPI_BINARY" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==YES goto AutoGenSpiBinaryYes
  )
)
set AUTO_GEN_SPI_BINARY=NO
goto AutoGenSpiBinaryCheckDone
:AutoGenSpiBinaryYes
set AUTO_GEN_SPI_BINARY=YES
:AutoGenSpiBinaryCheckDone

@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_EMMC_BINARY" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==YES goto AutoGenEmmcBinaryYes
  )
)
set AUTO_GEN_EMMC_BINARY=NO
goto AutoGenEmmcBinaryCheckDone
:AutoGenEmmcBinaryYes
set AUTO_GEN_EMMC_BINARY=YES
:AutoGenEmmcBinaryCheckDone

REM [-end-160712-IB07400757-add]REM


@FOR /f "tokens=1,4" %%a in ('find "APOLLOLAKE_CRB" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==YES goto CRBGen8M
  )
)

REM [-start-170216-IB07400841-add]REM
set OUTPUT_BIOS_FOLDER=BIOS_Full
REM [-end-170216-IB07400841-add]REM

REM [-start-160505-IB07400722-add]REM
REM [-start-170214-IB07400840-modify]REM
if not exist %OUTPUT_BIOS_FOLDER% mkdir %OUTPUT_BIOS_FOLDER%
REM [-end-170214-IB07400840-modify]REM
REM [-end-160505-IB07400722-add]REM

REM
REM OEM platform (default using Oxbow Hill)
REM
REM [-start-170216-IB07400841-modify]REM
echo Gen Full BIOS for OEM platform ..
REM [-end-170216-IB07400841-modify]REM

REM [-start-160713-IB07400757-add]REM
if not defined INPUT_FIT_XML_FOR_SPI (
  if defined DEFAULT_INPUT_FIT_XML_FOR_SPI set INPUT_FIT_XML_FOR_SPI=%DEFAULT_INPUT_FIT_XML_FOR_SPI%
)
if not defined INPUT_FIT_XML_FOR_EMMC (
  if defined DEFAULT_INPUT_FIT_XML_FOR_EMMC set INPUT_FIT_XML_FOR_EMMC=%DEFAULT_INPUT_FIT_XML_FOR_EMMC%
)
REM [-end-160713-IB07400757-add]REM

REM [-start-160601-IB07400738-modify]REM
@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_FULL_BINARY_AX" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==NO goto GenAxDone
  )
)
set SOC_STEPPING=AX

REM [-start-160713-IB07400757-modify]REM
if %AUTO_GEN_SPI_BINARY%==YES (
  if defined INPUT_FIT_XML_FOR_SPI (
    set FIT_INPUT_XML=%INPUT_FIT_XML_FOR_SPI%
  
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

if %AUTO_GEN_EMMC_BINARY%==YES (
  if defined INPUT_FIT_XML_FOR_EMMC (
    set FIT_INPUT_XML=%INPUT_FIT_XML_FOR_EMMC%
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)
REM [-end-160713-IB07400757-modify]REM

:GenAxDone

@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_FULL_BINARY_BX" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==NO goto GenBxDone
  )
)
set SOC_STEPPING=BX

REM [-start-160713-IB07400757-modify]REM
if %AUTO_GEN_SPI_BINARY%==YES (
  if defined INPUT_FIT_XML_FOR_SPI (
    set FIT_INPUT_XML=%INPUT_FIT_XML_FOR_SPI%
  
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

if %AUTO_GEN_EMMC_BINARY%==YES (
  if defined INPUT_FIT_XML_FOR_EMMC (
    set FIT_INPUT_XML=%INPUT_FIT_XML_FOR_EMMC%
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)
REM [-end-160713-IB07400757-modify]REM

:GenBxDone
REM [-end-160601-IB07400738-modify]REM

goto ExitGen8M

:CRBGen8M

REM [-start-160505-IB07400722-add]REM
REM [-start-170214-IB07400840-modify]REM
if exist %OUTPUT_BIOS_FOLDER% rmdir %OUTPUT_BIOS_FOLDER% /s /q
if exist %OUTPUT_BIOS_FOLDER%.7z (
  if exist "C:\Program Files\7-Zip\7z.exe" (
    echo UnZip BIOS bianry...
    "C:\Program Files\7-Zip\7z.exe" x %OUTPUT_BIOS_FOLDER%.7z > NUL
  )
)
if not exist %OUTPUT_BIOS_FOLDER% mkdir %OUTPUT_BIOS_FOLDER%
REM [-end-170214-IB07400840-modify]REM
REM [-end-160505-IB07400722-add]REM

REM [-start-160713-IB07400757-modify]REM
if not defined BUILD_PLATFORM_TYPE (
  goto CheckBuildPlatformTypeDone
)

set CRB_GEN_APL_RVP1_FULL_BIOS=NO
set CRB_GEN_APL_RVP2_FULL_BIOS=NO
set CRB_GEN_APLI_OXH_FULL_BIOS=NO
if %BUILD_PLATFORM_TYPE%==ApolloLake_RVP1 set CRB_GEN_APL_RVP1_FULL_BIOS=YES
if %BUILD_PLATFORM_TYPE%==ApolloLake_RVP2 set CRB_GEN_APL_RVP2_FULL_BIOS=YES
if %BUILD_PLATFORM_TYPE%==ApolloLakeI_OxbowHill set CRB_GEN_APLI_OXH_FULL_BIOS=YES
if %BUILD_PLATFORM_TYPE%==ApolloLakeI_LeafHill set CRB_GEN_APLI_OXH_FULL_BIOS=YES
if %BUILD_PLATFORM_TYPE%==ApolloLakeI_JuniperHill set CRB_GEN_APLI_OXH_FULL_BIOS=YES

:CheckBuildPlatformTypeDone

@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_FULL_BINARY_AX" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==NO goto CrbGenAxDone
  )
)
set SOC_STEPPING=AX

REM
REM Apollo Lake RVP1
REM
if %CRB_GEN_APL_RVP1_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake RVP1..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP1_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP1_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

REM
REM Apollo Lake RVP2
REM
if %CRB_GEN_APL_RVP2_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake RVP2..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP2_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP2_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

REM
REM Apollo Lake-I OxbowHill
REM
if %CRB_GEN_APLI_OXH_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake-I Oxbow Hill..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APLI_OXH_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APLI_OXH_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

:CrbGenAxDone

@FOR /f "tokens=1,4" %%a in ('find "AUTO_GEN_FULL_BINARY_BX" ..\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==NO goto CrbGenBxDone
  )
)
set SOC_STEPPING=BX

REM
REM Apollo Lake RVP1
REM
if %CRB_GEN_APL_RVP1_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake RVP1..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP1_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP1_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

REM
REM Apollo Lake RVP2
REM
if %CRB_GEN_APL_RVP2_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake RVP2..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP2_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APL_RVP2_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

REM
REM Apollo Lake-I OxbowHill
REM
if %CRB_GEN_APLI_OXH_FULL_BIOS%==YES (
REM [-start-170214-IB07400840-modify]REM
  echo Gen %CRB_GEN_BIOS_SIZE% BIOS for Apollo Lake-I Oxbow Hill..
REM [-end-170214-IB07400840-modify]REM
  if %AUTO_GEN_SPI_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APLI_OXH_XML_FOR_SPI%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
  if %AUTO_GEN_EMMC_BINARY%==YES (
REM [-start-170214-IB07400840-modify]REM
    set FIT_INPUT_XML=%CRB_APLI_OXH_XML_FOR_EMMC%
REM [-end-170214-IB07400840-modify]REM
REM [-start-160726-IB07400761-modify]REM
    @FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
      IF %%d==bin (
        set BIOS_FILE=%%c
        set BIOS_FILE=!BIOS_FILE:~5!
        if exist bios.bin del bios.bin
        copy /y %%c.bin bios.bin > NUL
        set OUTPUT_BIOS_NAME=!BIOS_FILE!
        pushd ..\stitch
        call StitchBiosOnly.bat
        popd
REM [-start-170612-IB07400873-add]REM
        call GenDnxFiles.bat
REM [-end-170612-IB07400873-add]REM
REM [-start-160912-IB07400783-add]REM
        if NOT "!errorlevel!" == "0" goto Gen8mError
REM [-end-160912-IB07400783-add]REM
      )
    )
REM [-end-160726-IB07400761-modify]REM
  )
)

:CrbGenBxDone
REM [-end-160713-IB07400757-modify]REM
REM [-start-170214-IB07400840-modify]REM

REM
REM Zip BIOS binary
REM
if exist "C:\Program Files\7-Zip\7z.exe" (
  echo Zip BIOS bianry...
  if exist %OUTPUT_BIOS_FOLDER%.7z del %OUTPUT_BIOS_FOLDER%.7z
  "C:\Program Files\7-Zip\7z.exe" a %OUTPUT_BIOS_FOLDER%.7z %OUTPUT_BIOS_FOLDER% > NUL
  rmdir %OUTPUT_BIOS_FOLDER% /s /q
)
REM [-end-170214-IB07400840-modify]REM
REM [-start-160912-IB07400783-add]REM
goto ExitGen8M

:Gen8mError
exit /b 1

REM [-end-160912-IB07400783-add]REM
:ExitGen8M

REM [-start-160713-IB07400757-add]REM
if exist BiosNameList.txt del BiosNameList.txt > NUL
REM [-end-160713-IB07400757-add]REM

EndLocal
