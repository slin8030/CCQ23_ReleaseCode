@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

if "%MAKEDIR%"=="" (
  for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
    set BUILDTARGET=%%a
  )
  set MAKEDIR=%WORKSPACE%\Build\!BUILDTARGET!_%TOOL_CHAIN%
)

set BASEDIR=!MAKEDIR!
set LANGCODES=

REM
REM find langcodes by PcdList.txt if --langcodes isn't exist in commanbd line parameter
REM
((echo "%*"|find "--langcodes")>nul) || (

  for %%i in (..\IA32 ..\X64 ..\ARM ..\AARCH64 IA32 X64 ARM AARCH64) do (

    set PCD_LIST_PATH=!MAKEDIR!\%%i\PcdList.txt

    if "!LANGCODES!" == "" if exist !PCD_LIST_PATH! (
      for /f "tokens=2* delims=: " %%j  in ('findstr PcdUefiVariableDefaultPlatformLangCodes !PCD_LIST_PATH!') do (
        set "LANGCODES='%%j'"
      )
    )

  )

  if "!LANGCODES!" == "" (
    echo.
    echo ---- Can not found langcodes in PcdList.txt, it will not generate font.hpk ----
    echo.
    @exit 0
  )

  set "LANGCODES=--langcodes !LANGCODES!"
)

set Param=!LANGCODES! --hpkdir !MAKEDIR! %*

if exist %WORKSPACE%\%PROJECT_PKG%\Project.fif (
  set Param=!Param! --fif %WORKSPACE%\%PROJECT_PKG%\Project.fif
)

GenFont.exe !Param!

@exit %ERRORLEVEL%

