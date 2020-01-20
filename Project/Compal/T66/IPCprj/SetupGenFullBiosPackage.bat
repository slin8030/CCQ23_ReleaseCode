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

if not exist "C:\Program Files\7-Zip\7z.exe" (
  echo Gen Full BIOS package not support.
  goto GenFullBiosPackageDone
)

echo Gen Full BIOS Package...

if exist GenFullBiosPackage rmdir GenFullBiosPackage /s /q
mkdir GenFullBiosPackage > NUL

echo Copy Bios folder...
xcopy Bios GenFullBiosPackage\Bios /I /S > NUL
if exist GenFullBiosPackage\Bios\BIOS_8M.7z del GenFullBiosPackage\Bios\BIOS_8M.7z > NUL
REM [-start-170215-IB07400840-add]REM
if exist GenFullBiosPackage\Bios\BIOS_16M.7z del GenFullBiosPackage\Bios\BIOS_16M.7z > NUL
REM [-end-170215-IB07400840-add]REM
if exist GenFullBiosPackage\Bios\GenFullBiosPackage.7z del GenFullBiosPackage\Bios\GenFullBiosPackage.7z > NUL
REM [-start-160713-IB07400758-add]REM
if exist GenFullBiosPackage\Bios\GenFullBiosPackage rmdir GenFullBiosPackage\Bios\GenFullBiosPackage /s /q > NUL
REM [-end-160713-IB07400758-add]REM

echo Copy Stitch folder...
xcopy Stitch GenFullBiosPackage\Stitch /I /S > NUL

echo Export Project.env Setting...
echo ##################################################################  > GenFullBiosPackage\Project.env
echo #                                                                #  >> GenFullBiosPackage\Project.env
echo #               Auto Gen Project.env for FIT tool                #  >> GenFullBiosPackage\Project.env
echo #                                                                #  >> GenFullBiosPackage\Project.env
echo ##################################################################  >> GenFullBiosPackage\Project.env
echo.                                                                    >> GenFullBiosPackage\Project.env

@FOR /f "tokens=1,2,3,4" %%a in ('find "APOLLOLAKE_CRB" Project.env') DO (
  IF %%a==DEFINE (
REM [-start-160713-IB07400758-modify]REM
	IF %%b==APOLLOLAKE_CRB (
  	  echo. %%a %%b %%c %%d >> GenFullBiosPackage\Project.env
	)
REM [-end-160713-IB07400758-modify]REM
  )
)

@FOR /f "tokens=1,2,3,4" %%a in ('find "AUTO_" Project.env') DO (
  IF %%a==DEFINE (
  	echo. %%a %%b %%c %%d >> GenFullBiosPackage\Project.env
  )
)

@FOR /f "tokens=1,2,3,4" %%a in ('find "INTEL_" Project.env') DO (
  IF %%a==DEFINE (
  	echo. %%a %%b %%c %%d >> GenFullBiosPackage\Project.env
  )
)

echo Zip Gen Full BIOS Package...
"C:\Program Files\7-Zip\7z.exe" a GenFullBiosPackage.7z GenFullBiosPackage > NUL
copy /y GenFullBiosPackage.7z Bios\GenFullBiosPackage.7z > NUL
rmdir GenFullBiosPackage /s /q > NUL
del GenFullBiosPackage.7z > NUL

echo Gen Full BIOS Package Done...
:GenFullBiosPackageDone
EndLocal

