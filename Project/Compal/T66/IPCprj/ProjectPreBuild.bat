@REM
@REM Project pre-build batch file
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

REM [-start-161207-IB07400824-add]REM
REM Build BIOS NAME Check
@FOR /f "tokens=1,4" %%a in ('find "APOLLOLAKE_CRB" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
  IF %%a==DEFINE (
  	IF %%b==YES set BIOS_NAME=APL
  	IF %%b==NO (
      IF %BIOS_NAME%==APL (
        echo.
        echo.
        echo Incorrect BIOS Name, please reopen build batch file!!
        echo.
        echo.
        pause
        exit /b 1
      )
  	)
  )
)
REM [-end-161207-IB07400824-add]REM

@REM Run IBB override pre-build process
@if exist %WORKSPACE%\%CHIPSET_PKG%\Override\IBBOverridePreBuild.bat call %WORKSPACE%\%CHIPSET_PKG%\Override\IBBOverridePreBuild.bat %1
@if exist %WORKSPACE%\Conf\tools_def_.txt (
        copy %WORKSPACE%\%T66_COMMON_PATH%\Override\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def.txt /y
    ) else (
        copy %WORKSPACE%\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def_.txt /y
        copy %WORKSPACE%\%T66_COMMON_PATH%\Override\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def.txt /y
    )
@if not errorlevel 0 goto error

@REM Run kernel pre-build process
@if exist %WORKSPACE%\BaseTools\KernelPreBuild.bat call %WORKSPACE%\BaseTools\KernelPreBuild.bat %1
@if not errorlevel 0 goto error

REM [-start-160725-IB07400761-add]REM
@FOR /f "tokens=1,4" %%a in ('find "VBT_HOOK_SUPPORT" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
  IF %%a==DEFINE (
    IF %%b==YES (
      REM 
      REM Gen VBT Header
      REM 
      @FOR /f "tokens=1,4" %%c in ('find "GOP_VER_X64" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
        IF %%c==DEFINE ( REM 64-bit GOP
          IF exist %WORKSPACE%\%PROJECT_PKG%\Binary\GopDriver\%%d\X64\vbt.bsf (
            %WORKSPACE%\%CHIPSET_PKG%\Tools\GenVbtHeader.exe %WORKSPACE%\%PROJECT_PKG%\Binary\GopDriver\%%d\X64\vbt.bsf %WORKSPACE%\%CHIPSET_PKG%\Include\GenGopVbtHeaderX64.h > NUL 
          )
        )
      )
      @FOR /f "tokens=1,4" %%c in ('find "GOP_VER_IA32" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
        IF %%c==DEFINE ( REM 32-bit GOP
          IF exist %WORKSPACE%\%PROJECT_PKG%\Binary\GopDriver\%%d\IA32\vbt.bsf (
            %WORKSPACE%\%CHIPSET_PKG%\Tools\GenVbtHeader.exe %WORKSPACE%\%PROJECT_PKG%\Binary\GopDriver\%%d\IA32\vbt.bsf %WORKSPACE%\%CHIPSET_PKG%\Include\GenGopVbtHeaderIa32.h > NUL
          )
        )
      )
      @FOR /f "tokens=1,4" %%c in ('find "VIDEO_ROM_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
        IF %%c==DEFINE ( REM VBIOS
REM [-start-160818-IB07400772-modify]REM
          IF exist %WORKSPACE%\%PROJECT_PKG%\Binary\LegacyVideoRom\%%d\bxt.bsf (
            %WORKSPACE%\%CHIPSET_PKG%\Tools\GenVbtHeader.exe %WORKSPACE%\%PROJECT_PKG%\Binary\LegacyVideoRom\%%d\bxt.bsf %WORKSPACE%\%CHIPSET_PKG%\Include\GenVbiosVbtHeader.h > NUL 
          )
REM [-end-160818-IB07400772-modify]REM
        )
      )  
    )
  )
)
REM [-end-160725-IB07400761-add]REM
REM [-start-170512-IB07400866-add]REM
REM
REM BaseTools Pre-Build Override
REM
if exist %WORKSPACE%\%CHIPSET_PKG%\Override2\BaseTools (
  xcopy /S /Y %WORKSPACE%\%CHIPSET_PKG%\Override2\BaseTools %WORKSPACE%\BaseTools > NUL
REM [-start-170519-IB07400868-add]REM
  xcopy /S /Y %WORKSPACE%\%CHIPSET_PKG%\Override2\Conf %WORKSPACE%\Conf > NUL
REM [-end-170519-IB07400868-add]REM
  xcopy /S /Y %WORKSPACE%\%T66_COMMON_PATH%\Override\Conf %WORKSPACE%\Conf > NUL
)
REM [-end-170512-IB07400866-add]REM
REM [-start-190102-IB07401061-add]REM
REM
REM SIC code override
REM
@FOR /f "tokens=1,4" %%a in ('find "IOTG_SIC_RC_CODE_SUPPORTED" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
  IF %%a==DEFINE (
    IF %%b==YES (
      xcopy /S /Y %WORKSPACE%\BroxtonSiIotgPkg %WORKSPACE%\BroxtonSiPkg > NUL
    )
    IF %%b==NO (
      xcopy /S /Y %WORKSPACE%\BroxtonSiCcgPkg %WORKSPACE%\BroxtonSiPkg > NUL
    )
  )
)
REM [-end-190102-IB07401061-add]REM
REM [-start-190321-IB07401093-add]REM
REM
REM Kernel Include Override
REM
if exist %WORKSPACE%\%CHIPSET_PKG%\Override2\InsydeModulePkg\Include (
  xcopy /S /Y %WORKSPACE%\%CHIPSET_PKG%\Override2\InsydeModulePkg\Include %WORKSPACE%\InsydeModulePkg\Include > NUL
)
REM [-end-190321-IB07401093-add]REM
@REM Run Chipset specific pre-build process
@if exist %WORKSPACE%\%CHIPSET_PKG%\ChipsetPreBuild.bat call %WORKSPACE%\%CHIPSET_PKG%\ChipsetPreBuild.bat %1
@if not errorlevel 0 goto error

REM[-start-161123-IB07250310-add]REM
@for /f "tokens=4" %%a in ('find "FSP_ENABLE" %WORKSPACE%\%PROJECT_PKG%\Project.env') do @set FSP_ENABLE=%%a
@REM Run Reset Vector build process
@if exist %WORKSPACE%\%CHIPSET_PKG%\Override\UefiCpuPkg\ResetVector\nasm\ResetVectorBuild.bat call %WORKSPACE%\%CHIPSET_PKG%\Override\UefiCpuPkg\ResetVector\nasm\ResetVectorBuild.bat %1
@if not errorlevel 0 goto error
REM[-end-161123-IB07250310-add]REM

REM [-start-170921-IB07400911-add]REM
REM [-start-181130-IB07401046-modify]REM
REM
REM For AutoBuild Compatible, Restore CHV.fd and NVCOMMON.fd
REM
for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\CHV_.fd  (
    pushd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\
    del CHV.fd > NUL
    del CHV_.fd > NUL
    popd
  )
  if exist %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\NVCOMMON_.fd (
    pushd %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\FV\
    del NVCOMMON.fd > NUL
    del NVCOMMON_.fd > NUL
    popd
  )
)
REM [-end-181130-IB07401046-modify]REM
REM [-end-170921-IB07400911-add]REM

:error
