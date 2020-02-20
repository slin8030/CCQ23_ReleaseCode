	@REM
@REM Project setup batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off

REM ---------------------------------------------------------------------------------------------
REM Auto setting of WORKSPACE environment variable
REM ---------------------------------------------------------------------------------------------
pushd \ && set ROOT_DIR=%cd% && popd && pushd .
:SetWorkSpace
if "%cd%" == "%ROOT_DIR%" goto Next
cd ..
if not exist %cd%\BaseTools goto SetWorkSpace
set WORKSPACE=%cd%
:Next
set ROOT_DIR= && popd

REM[-start-161123-IB07250310-add]REM
@if not defined NASM_PREFIX (
    @set "NASM_PREFIX=%WORKSPACE%/BroxtonChipsetPkg/Override/BaseTools/Bin/Win32/"
)
REM[-end-161123-IB07250310-add]REM

REM ---------------------------------------------------------------------------------------------
REM Auto setting of PROJECT_PKG and CHIPSET_PKG environment variable
REM ---------------------------------------------------------------------------------------------
%WORKSPACE%\BaseTools\Bin\Win32\GetProjectEnv PROJECT_PKG > NUL && for /f %%i in ('%WORKSPACE%\BaseTools\Bin\Win32\GetProjectEnv PROJECT_PKG') do set %%i
%WORKSPACE%\BaseTools\Bin\Win32\GetProjectEnv CHIPSET_PKG > NUL && for /f %%i in ('%WORKSPACE%\BaseTools\Bin\Win32\GetProjectEnv CHIPSET_PKG') do set %%i
@set PATH=%WORKSPACE%\%CHIPSET_PKG%;%PATH%

REM [-start-161206-IB07400822-add]REM
if not DEFINED SOC_NAME set SOC_NAME=ApolloLakeI
if not DEFINED CRB_BOARD_NAME set CRB_BOARD_NAME=OxbowHill
REM [-end-161206-IB07400822-add]REM

REM [-start-160328-IB07400711-add]REM
set BUILD_PLATFORM_TYPE=%SOC_NAME%_%CRB_BOARD_NAME%
if not DEFINED BIOS_NAME set BIOS_NAME=%SOC_NAME%_%CRB_BOARD_NAME%
@FOR /f "tokens=1,4" %%a in ('find "APOLLOLAKE_CRB" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==YES set BIOS_NAME=APL
  )
)
REM [-end-160328-IB07400711-add]REM

REM ---------------------------------------------------------------------------------------------
REM Project dependent parameters
REM ---------------------------------------------------------------------------------------------
REM [-start-170323-IB07400848-modify]REM
if not DEFINED TOOL_CHAIN set TOOL_CHAIN=DEVTLS_VC10
REM [-end-170323-IB07400848-modify]REM
set BOARD_NAME=APL

REM [-start-170327-IB07400850-add]REM
REM 
REM Early Override to support VS2015 Tool Chain
REM
if exist %WORKSPACE%\%CHIPSET_PKG%\Override2\BaseTools (
  xcopy /S /Y %WORKSPACE%\%CHIPSET_PKG%\Override2\BaseTools %WORKSPACE%\BaseTools > NUL
REM [-start-170519-IB07400868-add]REM
  xcopy /S /Y %WORKSPACE%\%CHIPSET_PKG%\Override2\Conf %WORKSPACE%\Conf > NUL
REM [-end-170519-IB07400868-add]REM
  xcopy /S /Y %WORKSPACE%\%T66_COMMON_PATH%\Override\Conf %WORKSPACE%\Conf > NUL
)
REM [-end-170327-IB07400850-add]REM
REM[-start-161123-IB07250310-add]REM
if "%FSP_ENABLE_BUILD%" == "YES" (
    REM ------------Fixup ApolloLakeBoardPkg\Project.env file -------------#
    @if not exist %WORKSPACE%\%PROJECT_PKG%\Project_.env (
        copy %WORKSPACE%\%PROJECT_PKG%\Project.env %WORKSPACE%\%PROJECT_PKG%\Project_.env /y
    )
    call SetFspFlag.js
) else (
    REM ------------Restore ApolloLakeBoardPkg\Project.env file -------------#
    @if exist %WORKSPACE%\%PROJECT_PKG%\Project_.env (
        copy %WORKSPACE%\%PROJECT_PKG%\Project_.env %WORKSPACE%\%PROJECT_PKG%\Project.env /y
        del  %WORKSPACE%\%PROJECT_PKG%\Project_.env
    )
)

@for /f "tokens=4" %%a in ('find "FSP_ENABLE" %WORKSPACE%\%PROJECT_PKG%\Project.env') do @set FSP_ENABLE=%%a

REM [-start-170915-IB07400910-modify]REM
if "%FSP_ENABLE%" == "YES" (
    copy %WORKSPACE%\%CHIPSET_PKG%\Override\BaseTools\Conf\MakefileFsp %WORKSPACE%\BaseTools\Conf\Makefile /y
) else (
    copy %WORKSPACE%\%CHIPSET_PKG%\Override\BaseTools\Conf\MakefileOrg %WORKSPACE%\BaseTools\Conf\Makefile /y
)
REM [-end-170915-IB07400910-modify]REM
REM[-end-161123-IB07250310-add]REM

REM [-start-180425-IB07400961-add]REM
set BUILD_TOOL_CHAIN=%TOOL_CHAIN%
REM [-end-180425-IB07400961-add]REM

call %WORKSPACE%\BaseTools\H2ORev50.bat

REM ++ >>>> Modify for build server process.
if '%AutoBuildEn%'=='1'  (
  nmake clean
  nm %AutoBuildCmd%
  C:\Projects\Script\WinSCP.com /script=C:\Projects\Script\%OemPath%.txt
)
REM ++ <<<< Modify for build server process.

