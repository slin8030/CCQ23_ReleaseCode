@REM
@REM Kernel pre-build batch file
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

@REM PkgPreBuild Project.dsc %WORKSPACE%\Build\%PROJECT_PKG%\Project.dsc %1 
@REM @if not errorlevel 0 goto error
@REM PkgPreBuild Project.fdf %WORKSPACE%\Build\%PROJECT_PKG%\Project.fdf %1
@REM @if not errorlevel 0 goto error
@copy Project.dsc %WORKSPACE%\Build\%PROJECT_PKG%\Project.dsc /y
@copy Project.fdf %WORKSPACE%\Build\%PROJECT_PKG%\Project.fdf /y

@for /f "tokens=3" %%a in ('findstr /R "\<TARGET\>" %WORKSPACE%\Conf\target.txt') do (
    GenFlashLayout Build\%PROJECT_PKG%\%%a_%TOOL_CHAIN% Build\%PROJECT_PKG%\AutoGen\FlashLayout
    @if not errorlevel 0 goto error
    GenFlashSetting Build\%PROJECT_PKG%\AutoGen
    @if not errorlevel 0 goto error
)
:error
