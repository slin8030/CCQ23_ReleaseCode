Echo off
REM Code structure preparation
REM D:\BIOS\SKyLake\AAX05
REM                \DEVTLS  
REM Project dependent parameters
REM subst directory : %1
REM project name    : %2
REM virtual drive   : %3
cls
If "%1" == "" goto HELP_Msg
If "%2" == "" goto HELP_Msg
If "%3" == "" goto HELP_Msg
goto START

:HELP_Msg
echo Usage: Seten %1 %2 %3
echo    %%1: Source code directory
echo    %%2: Project code (Project directory name - ex: AAX05)
echo    %%3: Virtual Drive to map - ex: Q
pause
goto END

:START
echo subst directory : %1
echo project name    : %2
echo virtual drive   : %3
echo.

subst %3: /D
subst %3: %1

Set OemPath=%2
Set tagfile=%3:

REM Check Parent directory exist the tool or not.
REM If not exist, use default D: drive or C: drive root directory.
REM Base on new define for easy get code/compiler, we define each compiler tool set with code.
REM D:\BIOS\SKyLake\AAX05
REM                \DEVTLS  
cd ..
If EXIST DEVTLS goto SetTOOL1
cd ..
If EXIST ..\DEVTLS goto SetTOOL1
cd ..
If EXIST ..\..\DEVTLS goto SetTOOL1
If EXIST D:\DEVTLS goto SetTOOL2
If EXIST C:\DEVTLS goto SetTOOL3
color 0c
echo !!! Can't found DEVTLS at parent directory and C or D drive.
echo !!! Please check Compiler tool ready or not.
echo.
echo.
echo.
goto SetTOOL2

:SetTOOL1
set DEVTLS_DIR=%cd:~3%\DEVTLS
set TOOL_DIR=%cd:~0,2%
goto SetTOOLDone

:SetTOOL2
set DEVTLS_DIR=DEVTLS
set TOOL_DIR=D:
goto SetTOOLDone

:SetTOOL3
set DEVTLS_DIR=DEVTLS
set TOOL_DIR=C:
goto SetTOOLDone

:SetTOOLDone
set TOOL_DRV=%TOOL_DIR%
%3:
set WORKSPACE=%cd:~0,3%
set PROJECT_PKG=Project\Compal\T66\IPCprj
cd %PROJECT_PKG%

echo WORKSPACE=%WORKSPACE%
echo TOOL_DRV=%TOOL_DRV%
echo TOOL_DIR=%TOOL_DIR%
echo DEVTLS_DIR=%DEVTLS_DIR%
echo PROJECT_PKG=%PROJECT_PKG%

rem Set BIOS ia32/x64 build and string file.
Set BIOSBuild=IA32
Set BIOSVerFile=i32
copy Q:\%PROJECT_PKG%\Project.uni.%BIOSVerFile% Q:\%PROJECT_PKG%\Project.uni /y

ProjectBuild.bat

:END