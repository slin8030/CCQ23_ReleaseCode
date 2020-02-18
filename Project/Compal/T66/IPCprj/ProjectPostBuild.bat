@REM
@REM Project post-build batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off

@REM  Read target configuration to environment variables  
@find /v "#" %WORKSPACE%\Conf\target.txt > string
@for /f "tokens=1,2,3" %%i in (string) do set %%i=%%k
@del string


@for /f "tokens=1,2,3,4" %%i in (%WORKSPACE%\Build\%PROJECT_PKG%\Project.env) do set %%j=%%l

if "%X64_CONFIG%" == "NO" (
  set TARGET_ARCH=Ia32
  SET TARGET_BIN=%BOARD_NAME%Ia32
) else (
  set TARGET_ARCH=X64
  SET TARGET_BIN=%BOARD_NAME%X64
)

REM [-start-170921-IB07400911-add]REM
REM
REM For AutoBuild Compatible, Restore CHV.fd and NVCOMMON.fd
REM
for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV_.fd  (
	copy /y %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV_.fd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV.fd > NUL
	del %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV_.fd > NUL
  )
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON_.fd (
    copy /y %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON_.fd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON.fd > NUL
    del %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON_.fd > NUL
  )
)
REM [-end-170921-IB07400911-add]REM

@REM [-start-151229-IB07220030-add]
@REM Patch $BME of Bvdt region of Fd according .FDM after region resize
@REM PatchBvdt Broadwell.fd BROADWELL.fdm
@REM [-end-151229-IB07220030-add]

@REM Run Chipset specific post-build process
@if exist %WORKSPACE%\%CHIPSET_PKG%\ChipsetPostBuild.bat call %WORKSPACE%\%CHIPSET_PKG%\ChipsetPostBuild.bat %1
@if not errorlevel 0 goto error

@REM Run kernel post-build process
@if exist %WORKSPACE%\BaseTools\KernelPostBuild.bat call %WORKSPACE%\BaseTools\KernelPostBuild.bat %1
@if not errorlevel 0 goto error

@REM [-start-160330-IB07220054-add]
@REM Update hash vale of FDM entries at last step of building
for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
  PatchFdmHash %WORKSPACE%/Build/%PROJECT_PKG%\%%a_%TOOL_CHAIN%/FV/NVCOMMON.fd %WORKSPACE%/Build/%PROJECT_PKG%\%%a_%TOOL_CHAIN%/FV/NVCOMMON.fdm
)
@REM [-end-160330-IB07220054-add]

@REM Run IBB override post-build process
@if exist %WORKSPACE%\%CHIPSET_PKG%\Override\IBBOverridePostBuild.bat call %WORKSPACE%\%CHIPSET_PKG%\Override\IBBOverridePostBuild.bat %1
@if not errorlevel 0 goto error

@REM FvAlignChecker.exe for 4K Align Check
FvAlignChecker.exe --PCD-report-file %WORKSPACE%/Build/%PROJECT_PKG%/BuildReport.txt

REM [-start-190220-IB07401083-add]REM
@REM Copy ObbLayout to Tools
for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
  @if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\IA32\ObbLayout.exe (
     copy /y %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\IA32\ObbLayout.exe %WORKSPACE%\%PROJECT_PKG%\Bios\ObbLayout.exe
  )
)
REM [-end-190220-IB07401083-add]REM

:FdDone
 
@REM Copy BIOS binary Fd file to Bios directory
@if not exist %WORKSPACE%\%PROJECT_PKG%\Bios mkdir %WORKSPACE%\%PROJECT_PKG%\Bios

REM [-start-160223-IB07400702-add]REM
@find /v "#" %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt > string
@for /f "tokens=1,2,3" %%i in (string) do set %%i=%%k
@del string
REM [-end-160223-IB07400702-add]REM

REM [-start-160420-IB07400719-modify]REM
echo IFWI Stitiching.....
pushd Stitch\
call Stitch.bat > NUL
popd
if NOT "%errorlevel%" == "0" goto error 
REM [-start-160713-IB07400757-modify]REM
pushd Bios\
REM [-start-170214-IB07400840-modify]REM
REM Gen Integrated Firmware Image (IFWI)
call GenIfwiBios.bat
REM [-end-170214-IB07400840-modify]REM
popd
REM [-start-160913-IB07400783-add]REM
if NOT "%errorlevel%" == "0" goto error 
REM [-end-160913-IB07400783-add]REM
REM call SetupGenFullBiosPackage.bat
@echo.
REM [-end-160713-IB07400757-modify]REM
REM [-end-160420-IB07400719-modify]REM
REM[-start-161123-IB07250310-add]REM
REM ------------Restore ApolloLakeBoardPkg\Project.env file -------------#
REM Restore the Project.env that must in the end of build.
@if exist %WORKSPACE%\%PROJECT_PKG%\Project_.env (
    copy %WORKSPACE%\%PROJECT_PKG%\Project_.env %WORKSPACE%\%PROJECT_PKG%\Project.env /y
    del  %WORKSPACE%\%PROJECT_PKG%\Project_.env
)
REM[-end-161123-IB07250310-add]REM

@echo Makefile Board:       %BOARD_NAME%
@echo Makefile TARGET:      %1
@echo TOOL_CHAIN_TAG:       %TOOL_CHAIN%
REM [-start-151122-IB07220012-add]
REM [-start-160223-IB07400702-remove]REM
REM @echo RVP2:                 %RVP2%
REM [-end-160223-IB07400702-remove]REM
REM [-end-151122-IB07220012-add]
REM [-start-160420-IB07400719-modify]REM
REM [-start-170214-IB07400840-modify]REM
@echo BIOS location:        %PROJECT_PKG%\Bios\
REM [-end-170214-IB07400840-modify]REM
REM [-end-160420-IB07400719-modify]REM
REM [-start-170920-IB07400911-add]REM
REM
REM For AutoBuild Compatible, backup BIOS.7z and GenFullBiosPackage.7z
REM
for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV.fd move /y %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV.fd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV_.fd > NUL
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON.fd move /y %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON.fd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON_.fd > NUL
  if exist %WORKSPACE%\%PROJECT_PKG%\Bios\BIOS_8M.7z copy %WORKSPACE%\%PROJECT_PKG%\Bios\BIOS_8M.7z %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV.fd /y > NUL
  if exist %WORKSPACE%\%PROJECT_PKG%\Bios\GenFullBiosPackage.7z copy %WORKSPACE%\%PROJECT_PKG%\Bios\GenFullBiosPackage.7z %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON.fd /y > NUL
)
REM [-end-170920-IB07400911-add]REM
@echo.
@echo The EDKII BIOS build has successfully completed!
@time /t
@echo.

:error
