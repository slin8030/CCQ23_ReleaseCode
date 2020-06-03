@REM
@REM Chipset PreBuild batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************
@REM     Add Chispet specific pre-build process here
@echo off

if not exist %WORKSPACE%\Build mkdir %WORKSPACE%\Build
if not exist %WORKSPACE%\Build\%PROJECT_PKG% mkdir %WORKSPACE%\Build\%PROJECT_PKG%

@if "%1%" == "" (
  set BuildTarget=uefi32
  goto ChipsetUefi32
) else (
  set BuildTarget=%1
)

REM [-start-190227-IB07401087-add]REM
REM
REM Set default build flag
REM 
set MRC_EV_RMT_BUILD=NO
set FIRMWARE_PERFORMANCE=NO
set NO_TIME_STAMP=NO
set BUILD_SECURE_FLASH=NO
set BUILD_H2O_CBT=NO
set BUILD_H2O_RVS=NO
REM [-end-190227-IB07401087-add]REM

@if /I "%1"=="uefi64"           goto ChipsetUefi64
@if /I "%1"=="uefi32"           goto ChipsetUefi32
@if /I "%1"=="uefi64ddt"        goto ChipsetUefi64ddt
@if /I "%1"=="uefi32ddt"        goto ChipsetUefi32ddt
@if /I "%1"=="efidebug"         goto ChipsetEfidebug
@if /I "%1"=="efidebugddt"      goto ChipsetEfidebugddt
@if /I "%1"=="efidebugddt64"    goto ChipsetEfidebugddt64
@if /I "%1"=="ddtdebug"         goto ChipsetDdtdebug
@if /I "%1"=="uefi64perf"       goto ChipsetUefi64perf
@if /I "%1"=="uefi32perf"       goto ChipsetUefi32perf
@if /I "%1"=="uefi64debug"      goto ChipsetUefi64debug
@if /I "%1"=="uefi32rmt"        goto Chipsetuefi32rmt
@if /I "%1"=="uefi64rmt"        goto Chipsetuefi64rmt
REM [-start-160328-IB07400711-add]REM
@if /I "%1"=="uefi32edbg"      	goto ChipsetUefi32edbg
@if /I "%1"=="uefi64edbg"       goto ChipsetUefi64edbg
@if /I "%1"=="uefi32ddtedbg"    goto ChipsetUefi32ddtedbg
@if /I "%1"=="uefi64ddtedbg"    goto ChipsetUefi64ddtedbg
@if /I "%1"=="uefi32nots"       goto ChipsetUefi32nots
@if /I "%1"=="uefi64nots"       goto ChipsetUefi64nots
REM [-end-160328-IB07400711-add]REM
REM [-start-160822-IB07400773-add]REM
@if /I "%1"=="uefi32sf"       	goto ChipsetUefi32sf
@if /I "%1"=="uefi64sf"       	goto ChipsetUefi64sf
REM [-start-170407-IB07400856-remove]REM
REM @if /I "%1"=="uefi32sfesp"     	goto ChipsetUefi32sfesp
REM @if /I "%1"=="uefi64sfesp"     	goto ChipsetUefi64sfesp
REM [-end-170407-IB07400856-remove]REM
REM [-end-160822-IB07400773-add]REM
REM [-start-170331-IB07400855-add]REM
@if /I "%1"=="uefi32cbt"     	goto ChipsetUefi32cbt
@if /I "%1"=="uefi64cbt"     	goto ChipsetUefi64cbt
REM [-end-170331-IB07400855-add]REM
REM [-start-181120-IB07401039-add]REM
@if /I "%1"=="uefi32rvs"     	goto ChipsetUefi32rvs
@if /I "%1"=="uefi64rvs"     	goto ChipsetUefi64rvs
REM [-end-181120-IB07401039-add]REM
goto ChipsetUefi32


:ChipsetUefi64
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
goto ChipsetPreBuildFinal

:ChipsetUefi32
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
goto ChipsetPreBuildFinal

:ChipsetUefi64ddt
set X64_CONFIG=YES
set INSYDE_DEBUG=YES
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=DDT
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi32ddt
set X64_CONFIG=NO
set INSYDE_DEBUG=YES
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=DDT
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetEfidebug
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetEfidebugddt
set X64_CONFIG=NO
set INSYDE_DEBUG=YES
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG_TO_DDT
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetEfidebugddt64
set X64_CONFIG=YES
set INSYDE_DEBUG=YES
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG_TO_DDT
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetDdtdebug
set X64_CONFIG=NO
set INSYDE_DEBUG=YES
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=YES
set FD_TARGET=DDT_EFIDEBUG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi32perf
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=PERF
REM [-start-160328-IB07400711-add]REM
set FIRMWARE_PERFORMANCE=YES
REM [-end-160328-IB07400711-add]REM
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi64perf
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=PERF
REM [-start-160328-IB07400711-add]REM
set FIRMWARE_PERFORMANCE=YES
REM [-end-160328-IB07400711-add]REM
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi64debug
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:Chipsetuefi32rmt
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
set MRC_EV_RMT_BUILD=YES
goto ChipsetPreBuildFinal

:Chipsetuefi64rmt
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
set MRC_EV_RMT_BUILD=YES
goto ChipsetPreBuildFinal

REM [-start-160328-IB07400711-add]REM
:ChipsetUefi32edbg
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi64edbg
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=EFIDEBUG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi32ddtedbg
set X64_CONFIG=NO
set INSYDE_DEBUG=YES
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=DDTEDBG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi64ddtedbg
set X64_CONFIG=YES
set INSYDE_DEBUG=YES
set EFI_DEBUG=YES
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=DDTEDBG
set PROJECT_TARGET=DEBUG
goto ChipsetPreBuildFinal

:ChipsetUefi64nots
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
set NO_TIME_STAMP=YES
goto ChipsetPreBuildFinal

:ChipsetUefi32nots
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
set NO_TIME_STAMP=YES
goto ChipsetPreBuildFinal
REM [-end-160328-IB07400711-add]REM
REM [-start-160822-IB07400773-add]REM
:ChipsetUefi64sf
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=SF
set PROJECT_TARGET=RELEASE
set BUILD_SECURE_FLASH=YES
goto ChipsetPreBuildFinal

:ChipsetUefi32sf
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=SF
REM [-start-160831-IB07400776-modify]REM
set PROJECT_TARGET=RELEASE
REM [-end-160831-IB07400776-modify]REM
set BUILD_SECURE_FLASH=YES
goto ChipsetPreBuildFinal

REM [-start-170407-IB07400856-remove]REM
REM :ChipsetUefi64sfesp
REM set X64_CONFIG=YES
REM set INSYDE_DEBUG=NO
REM set EFI_DEBUG=NO
REM set USB_DEBUG_SUPPORT=NO
REM set FD_TARGET=SFESP
REM set PROJECT_TARGET=RELEASE
REM set BUILD_SECURE_FLASH=YES
REM set BUILD_SECURE_FLASH_ESP=YES
REM goto ChipsetPreBuildFinal
REM 
REM :ChipsetUefi32sfesp
REM set X64_CONFIG=NO
REM set INSYDE_DEBUG=NO
REM set EFI_DEBUG=NO
REM set USB_DEBUG_SUPPORT=NO
REM set FD_TARGET=SFESP
REM REM [-start-160831-IB07400776-modify]REM
REM set PROJECT_TARGET=RELEASE
REM REM [-end-160831-IB07400776-modify]REM
REM set BUILD_SECURE_FLASH=YES
REM set BUILD_SECURE_FLASH_ESP=YES
REM goto ChipsetPreBuildFinal
REM [-end-170407-IB07400856-remove]REM
REM [-end-160822-IB07400773-add]REM

REM [-start-170331-IB07400855-add]REM
:ChipsetUefi64cbt
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=CBT
set PROJECT_TARGET=DEBUG
set BUILD_H2O_CBT=YES
goto ChipsetPreBuildFinal

:ChipsetUefi32cbt
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=CBT
set PROJECT_TARGET=DEBUG
set BUILD_H2O_CBT=YES
goto ChipsetPreBuildFinal
REM [-end-170331-IB07400855-add]REM
REM [-start-181120-IB07401039-add]REM
REM [-start-181128-IB07401044-modify]REM
:ChipsetUefi64rvs
set X64_CONFIG=YES
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
set BUILD_H2O_RVS=YES
goto ChipsetPreBuildFinal

:ChipsetUefi32rvs
set X64_CONFIG=NO
set INSYDE_DEBUG=NO
set EFI_DEBUG=NO
set USB_DEBUG_SUPPORT=NO
set FD_TARGET=RELEASE
set PROJECT_TARGET=RELEASE
set BUILD_H2O_RVS=YES
goto ChipsetPreBuildFinal
REM [-end-181128-IB07401044-modify]REM
REM [-end-181120-IB07401039-add]REM

:ChipsetPreBuildFinal

@if "%X64_CONFIG%" == "" (
  set X64_CONFIG=NO
)
@if "%INSYDE_DEBUG%" == "" (
  set INSYDE_DEBUG=NO
)
@if "%EFI_DEBUG%" == "" (
  set EFI_DEBUG=NO
)
@if "%USB_DEBUG_SUPPORT%" == "" (
  set USB_DEBUG_SUPPORT=NO
)
REM [-start-160419-IB07400718-add]REM
@if "%MRC_EV_RMT_BUILD%" == "" (
  set MRC_EV_RMT_BUILD=NO
)
REM [-end-160419-IB07400718-add]REM

REM [-start-151122-IB07220012-add]
REM [-start-160223-IB07400702-remove]REM
REM @if "%RVP2%" == "" (
REM   set RVP2=NO
REM )
REM [-end-160223-IB07400702-remove]REM
REM [-end-151122-IB07220012-add]

@if "%X64_CONFIG%" == "YES" (
  set TARGET_ARCH=X64
  set ARCH=X64
) else (
  set TARGET_ARCH=Ia32
  set ARCH=IA32  
)

REM [-start-160328-IB07400711-add]REM
@if "%FIRMWARE_PERFORMANCE%" == "" (
  set FIRMWARE_PERFORMANCE=NO
)
@if "%NO_TIME_STAMP%" == "" (
  set NO_TIME_STAMP=NO
)

REM [-start-160822-IB07400773-add]REM
@if "%BUILD_SECURE_FLASH%" == "" (
  set BUILD_SECURE_FLASH=NO
)
REM [-start-170407-IB07400856-remove]REM
REM @if "%BUILD_SECURE_FLASH_ESP%" == "" (
REM   set BUILD_SECURE_FLASH_ESP=NO
REM )
REM [-end-170407-IB07400856-remove]REM
REM [-end-160822-IB07400773-add]REM
REM [-start-170331-IB07400855-add]REM
@if "%BUILD_H2O_CBT%" == "" (
  set BUILD_H2O_CBT=NO
)
REM [-end-170331-IB07400855-add]REM
REM [-start-181120-IB07401039-add]REM
@if "%BUILD_H2O_RVS%" == "" (
  set BUILD_H2O_RVS=NO
)
REM [-end-181120-IB07401039-add]REM
REM [-start-160421-IB07400719-modify]REM
set BUILD_BIOS_NAME=%BIOS_NAME%_%TARGET_ARCH%_%FD_TARGET%
REM [-end-160421-IB07400719-modify]REM
REM [-end-160328-IB07400711-add]REM

echo DEFINE X64_CONFIG            = %X64_CONFIG%                          >  %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE INSYDE_DEBUGGER       = %INSYDE_DEBUG%                        >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE EFI_DEBUG             = %EFI_DEBUG%                           >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE USB_DEBUG_SUPPORT     = %USB_DEBUG_SUPPORT%                   >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE FD_TARGET             = %FD_TARGET%                           >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE MRC_EV_RMT_BUILD      = %MRC_EV_RMT_BUILD%                    >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-start-151122-IB07220012-add]
REM [-start-160223-IB07400702-remove]REM
REM echo DEFINE RVP2                  = %RVP2%                                >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-160223-IB07400702-remove]REM
REM [-end-151122-IB07220012-add]
REM [-start-160328-IB07400711-add]REM
echo DEFINE FIRMWARE_PERFORMANCE  = %FIRMWARE_PERFORMANCE%                >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE BUILD_PLATFORM_TYPE   = %BUILD_PLATFORM_TYPE%                 >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE BUILD_BIOS_NAME       = %BUILD_BIOS_NAME%                     >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
echo DEFINE NO_TIME_STAMP         = %NO_TIME_STAMP%                       >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-160328-IB07400711-add]REM
REM [-start-160822-IB07400773-add]REM
echo DEFINE BUILD_SECURE_FLASH     = %BUILD_SECURE_FLASH%                 >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-start-170407-IB07400856-remove]REM
REM echo DEFINE BUILD_SECURE_FLASH_ESP = %BUILD_SECURE_FLASH_ESP%             >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-170407-IB07400856-remove]REM
REM [-end-160822-IB07400773-add]REM
REM [-start-170331-IB07400855-add]REM
echo DEFINE BUILD_H2O_CBT          = %BUILD_H2O_CBT%                      >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-170331-IB07400855-add]REM
REM [-start-180424-IB07400961-add]REM
echo DEFINE BUILD_TOOL_CHAIN       = %BUILD_TOOL_CHAIN%                   >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-180424-IB07400961-add]REM
REM [-start-181120-IB07401039-add]REM
echo DEFINE BUILD_H2O_RVS          = %BUILD_H2O_RVS%                      >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env
REM [-end-181120-IB07401039-add]REM

REM [-start-190227-IB07401087-add]REM
echo ============================= Chipset PreBuild Flags ==========================
echo X64_CONFIG             :%X64_CONFIG%
echo INSYDE_DEBUGGER        :%INSYDE_DEBUG%
echo EFI_DEBUG              :%EFI_DEBUG%
echo USB_DEBUG_SUPPORT      :%USB_DEBUG_SUPPORT%
echo FD_TARGET              :%FD_TARGET%
echo MRC_EV_RMT_BUILD       :%MRC_EV_RMT_BUILD%
echo FIRMWARE_PERFORMANCE   :%FIRMWARE_PERFORMANCE%
echo BUILD_PLATFORM_TYPE    :%BUILD_PLATFORM_TYPE%
echo BUILD_BIOS_NAME        :%BUILD_BIOS_NAME%
echo NO_TIME_STAMP          :%NO_TIME_STAMP%
echo BUILD_SECURE_FLASH     :%BUILD_SECURE_FLASH%
echo BUILD_H2O_CBT          :%BUILD_H2O_CBT%
echo BUILD_TOOL_CHAIN       :%BUILD_TOOL_CHAIN%
echo BUILD_H2O_RVS          :%BUILD_H2O_RVS%
echo ===============================================================================
REM [-end-190227-IB07401087-add]REM

echo ACTIVE_PLATFORM              = Build/%PROJECT_PKG%/Project.dsc       >  %WORKSPACE%\Conf\target.txt
echo BUILD_RULE_CONF              = Conf/build_rule.txt                   >> %WORKSPACE%\Conf\target.txt
echo TARGET                       = %PROJECT_TARGET%                      >> %WORKSPACE%\Conf\target.txt
echo TOOL_CHAIN_CONF              = Conf/tools_def.txt                    >> %WORKSPACE%\Conf\target.txt
echo TOOL_CHAIN_TAG               = %TOOL_CHAIN%                          >> %WORKSPACE%\Conf\target.txt
echo MAX_CONCURRENT_THREAD_NUMBER = %NUMBER_OF_PROCESSORS%                >> %WORKSPACE%\Conf\target.txt


set CHIPSET_PKG=BroxtonChipsetPkg
find "DEFINE" %WORKSPACE%\Build\%PROJECT_PKG%\Project.dsc > %WORKSPACE%\Build\%PROJECT_PKG%\Define.txt
for /f "tokens=4" %%a in ('find "CHIPSET_PKG" %WORKSPACE%\Build\%PROJECT_PKG%\Define.txt') do set CHIPSET_PKG=%%a

@if "%X64_CONFIG%" == "NO" (
  echo TARGET_ARCH                  = IA32                                  >> %WORKSPACE%\Conf\target.txt
) else (
  echo TARGET_ARCH                  = IA32 X64                              >> %WORKSPACE%\Conf\target.txt
) 

REM [-start-160223-IB07400702-add]REM
@FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_KIT_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
  IF %%a==DEFINE (
  	echo CSE_Version                = %%b                              	    > %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
  )
)
REM [-start-160601-IB07400738-remove]REM
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_ISH_FW_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo ISH_Version                = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_PMC_FW_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo PMC_Version                = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_MICROCODE_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo Patch_Version              = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_PMC_SMIP_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo Smip_PMC_Version           = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM [-end-160601-IB07400738-remove]REM
REM [-start-160421-IB07400719-remove]REM
REM @FOR /f "tokens=1,4" %%a in ('find "GEN_SPI_BIOS_BINARY" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo GEN_SPI_BIOS_BINARY        = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM @FOR /f "tokens=1,4" %%a in ('find "GEN_EMMC_BIOS_BINARY" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo GEN_EMMC_BIOS_BINARY       = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM REM [-start-160304-IB07400707-add]REM
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_FIT_XML_SPI_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo FIT_XML_SPI                = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM @FOR /f "tokens=1,4" %%a in ('find "INTEL_FIT_XML_EMMC_VERSION" %WORKSPACE%\%PROJECT_PKG%\Project.env') DO (
REM   IF %%a==DEFINE (
REM   	echo FIT_XML_EMMC               = %%b                              	    >> %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt
REM   )
REM )
REM REM [-end-160304-IB07400707-add]REM
REM [-end-160421-IB07400719-remove]REM
REM [-end-160223-IB07400702-add]REM

REM [-start-160504-IB07400722-add]REM
REM Gen Project UNI
@FOR /f "tokens=4" %%a in ('find "GEN_PROJECT_UNI" Project.env') DO IF %%a==NO GOTO Default_Project_Info
REM
REM Gen System Time
REM
for /f "delims=" %%a in ('wmic OS Get localdatetime  ^| find "."') do set "dt=%%a"
set "YYYY=%dt:~0,4%"
set "MM=%dt:~4,2%"
set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%"
set "Min=%dt:~10,2%"
set "Sec=%dt:~12,2%"

set BuildDate=%MM%/%DD%/%YYYY%
set BuildTime=%HH%:%Min%:%Sec%

echo //;############################ BuildTime Project Info ############################# >  %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
REM [-start-170313-IB07400847-modify]REM
@FOR /f "tokens=2 delims=[=],#" %%a in ('find "STR_MISC_BIOS_VERSION" Project.env') DO (
set BIOSVERSTR=%%a
)
set "BIOSVERSTR=%BIOSVERSTR:~1%"
echo #string STR_MISC_BIOS_VERSION         #language en-US  "%BIOSVERSTR%"                >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
echo DEFINE GEN_SMBIOS_TYPE00     = "FR1AA V;%BIOSVERSTR%;%BuildDate%"               >> %WORKSPACE%\Build\%PROJECT_PKG%\Project.env

@FOR /f "tokens=2 delims=[=],#" %%a in ('find "STR_MISC_SYSTEM_PRODUCT_NAME" Project.env') DO (
set PRODSTR=%%a
)
set "PRODSTR=%PRODSTR:~1%"
echo #string STR_MISC_SYSTEM_PRODUCT_NAME  #language en-US  "%PRODSTR% %TARGET_ARCH%"     >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
REM [-end-170313-IB07400847-modify]REM

@FOR /f "tokens=4" %%a in ('find "STR_CCB_VERSION" Project.env') DO (
echo #string STR_CCB_VERSION               #language en-US  "%%a"                         >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
)

REM
REM Gen Build Time
REM
@FOR /f "tokens=4" %%a in ('find "RELEASE_DATE_IS_BUILD_DATE" Project.env') DO IF %%a==YES GOTO ReleaseDateIsBuildDate

@FOR /f "tokens=4" %%a in ('find "STR_MISC_BIOS_RELEASE_DATE" Project.env') DO (
echo #string STR_MISC_BIOS_RELEASE_DATE    #language en-US  "%%a"                         >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
)
@FOR /f "tokens=4" %%a in ('find "STR_MISC_BIOS_RELEASE_TIME" Project.env') DO (
echo #string STR_MISC_BIOS_RELEASE_TIME    #language en-US  "%%a"                         >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
)
goto GenOtherInfo

:ReleaseDateIsBuildDate

@if "%NO_TIME_STAMP%" == "YES" ( 
  @FOR /f "tokens=4" %%a in ('find "STR_MISC_BIOS_RELEASE_DATE" Project.env') DO (
  echo #string STR_MISC_BIOS_RELEASE_DATE    #language en-US  "%%a"                       >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
  )
  @FOR /f "tokens=4" %%a in ('find "STR_MISC_BIOS_RELEASE_TIME" Project.env') DO (
  echo #string STR_MISC_BIOS_RELEASE_TIME    #language en-US  "%%a"                       >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
  )
) else (
  echo #string STR_MISC_BIOS_RELEASE_DATE    #language en-US  "%BuildDate%"               >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
  echo #string STR_MISC_BIOS_RELEASE_TIME    #language en-US  "%BuildTime%"               >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
)

:GenOtherInfo

@FOR /f "tokens=4" %%a in ('find "STR_ESRT_VERSION" Project.env') DO (
echo #string STR_ESRT_VERSION              #language en-US  "%%a"                         >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
)  
@FOR /f "tokens=4" %%a in ('find "STR_ESRT_FIRMWARE_GUID" Project.env') DO (
echo #string STR_ESRT_FIRMWARE_GUID        #language en-US  "%%a"                         >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt
) 
echo //;################################################################################# >> %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt

REM
REM Covert ASCII to Unicode
REM
CMD /U /C TYPE %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.txt > %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.uni
copy /b %WORKSPACE%\%PROJECT_PKG%\CopyRight.uni + %WORKSPACE%\Build\%PROJECT_PKG%\ProjectInfo.uni %WORKSPACE%\%PROJECT_PKG%\Project.uni >> NUL
goto End_Gen_Project_Info

:Default_Project_Info
copy /y %WORKSPACE%\%PROJECT_PKG%\ProjectUniDefault.uni %WORKSPACE%\%PROJECT_PKG%\Project.uni > NUL

:End_Gen_Project_Info
REM [-end-160504-IB07400722-add]REM
