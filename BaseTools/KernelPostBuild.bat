@REM
@REM Kernel post-build batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@if exist %WORKSPACE%\Build\%PROJECT_PKG%\BuildProject.dsc copy %WORKSPACE%\Build\%PROJECT_PKG%\BuildProject.dsc %WORKSPACE%\Build\%PROJECT_PKG%\Project.dsc /y
@if exist %WORKSPACE%\Build\%PROJECT_PKG%\BuildProject.fdf copy %WORKSPACE%\Build\%PROJECT_PKG%\BuildProject.fdf %WORKSPACE%\Build\%PROJECT_PKG%\Project.fdf /y
@if exist %WORKSPACE%\InsydeModulePkg\Tools\GenUnsignedFv\GenUnsignedFv.exe (
  @FOR /f "tokens=3" %%a IN ('findstr /R "\<TARGET\>" ..\Conf\target.txt') DO (
    @copy %WORKSPACE%\InsydeModulePkg\Tools\GenUnsignedFv\GenUnsignedFv.exe %WORKSPACE%\Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN%\IA32\GenUnsignedFv.exe /y
  )
)