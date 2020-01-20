@REM @file
@REM   Windows batch file to find the Visual Studio set up script
@REM
@REM Copyright (c) 2013-2014, ARM Limited. All rights reserved.

@REM This program and the accompanying materials
@REM are licensed and made available under the terms and conditions of the BSD License
@REM which accompanies this distribution.  The full text of the license may be found at
@REM http://opensource.org/licenses/bsd-license.php
@REM
@REM THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM


@echo off
if "%1" == "" goto  :main
goto :select

:read_vsvars
@rem Do nothing if already found, otherwise call vsvars32.bat if there
if defined VCINSTALLDIR goto :EOF
  set GET_VSVARS_BAT_CHECK_DIR=%1
  set GET_VSVARS_BAT_CHECK_DIR=%GET_VSVARS_BAT_CHECK_DIR:"=%
  set TOOL_CHAIN=%2
  if exist "%GET_VSVARS_BAT_CHECK_DIR%\vsvars32.bat"  call "%GET_VSVARS_BAT_CHECK_DIR%\vsvars32.bat"
  if defined VCINSTALLDIR (
    if not "%VCINSTALLDIR:(x86)=%"=="%VCINSTALLDIR%" (set TOOL_CHAIN=%TOOL_CHAIN%x86)
  )
:vsvars_done
goto :EOF


REM NOTE: This file will find the most recent Visual Studio installation
REM       apparent from the environment.
REM       To use an older version, modify your environment set up.
REM       (Or invoke the relevant vsvars32 file beforehand).

:main
if defined VCINSTALLDIR goto :done
  if defined VS140COMNTOOLS  call :read_vsvars  "%VS140COMNTOOLS%" VS2015
  if defined VS130COMNTOOLS  call :read_vsvars  "%VS130COMNTOOLS%" VS2014CTP
  if defined VS120COMNTOOLS  call :read_vsvars  "%VS120COMNTOOLS%" VS2013
  if defined VS110COMNTOOLS  call :read_vsvars  "%VS110COMNTOOLS%" VS2012
  if defined VS100COMNTOOLS  call :read_vsvars  "%VS100COMNTOOLS%" VS2010
  if defined VS90COMNTOOLS   call :read_vsvars  "%VS90COMNTOOLS%" VS2008
  if defined VS80COMNTOOLS   call :read_vsvars  "%VS80COMNTOOLS%" VS2005
  if defined VS71COMNTOOLS   call :read_vsvars  "%VS71COMNTOOLS%" VS2003
goto :done

:select
  if "%1"=="VS2015" goto :VS2015Setup
  if "%1"=="VS2014" goto :VS2014Setup
  if "%1"=="VS2013" goto :VS2013Setup
  if "%1"=="VS2012" goto :VS2012Setup
  if "%1"=="VS2010" goto :VS2010Setup
  if "%1"=="VS2008" goto :VS2008Setup
  if "%1"=="VS2005" goto :VS2005Setup
  if "%1"=="VS2003" goto :VS2003Setup
:error
  goto :EOF
  
:VS2015Setup
  if defined VS140COMNTOOLS  call :read_vsvars  "%VS140COMNTOOLS%" VS2015
  goto :done
:VS2014Setup
  if defined VS130COMNTOOLS  call :read_vsvars  "%VS130COMNTOOLS%" VS2014CTP
  goto :done
:VS2013Setup
  if defined VS120COMNTOOLS  call :read_vsvars  "%VS120COMNTOOLS%" VS2013
  goto :done
:VS2012Setup
  if defined VS110COMNTOOLS  call :read_vsvars  "%VS110COMNTOOLS%" VS2012
  goto :done
:VS2010Setup
  if defined VS100COMNTOOLS  call :read_vsvars  "%VS100COMNTOOLS%" VS2010
  goto :done
:VS2008Setup
  if defined VS90COMNTOOLS   call :read_vsvars  "%VS90COMNTOOLS%" VS2008
  goto :done
:VS2005Setup
  if defined VS80COMNTOOLS   call :read_vsvars  "%VS80COMNTOOLS%" VS2005
  goto :done
:VS2003Setup
  if defined VS71COMNTOOLS   call :read_vsvars  "%VS71COMNTOOLS%" VS2003
  goto :done

:done
if not defined VCINSTALLDIR goto :error
set GET_VSVARS_BAT_CHECK_DIR=
