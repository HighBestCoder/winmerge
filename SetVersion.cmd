@echo off

set MAJOR=2
set MINOR=16
set REVISION=45
set PATCHLEVEL=1
set STRPRIVATEBUILD=""

set RCVER=%MAJOR%.%MINOR%.%REVISION%.%PATCHLEVEL%
set STRVER=%MAJOR%.%MINOR%.%REVISION%.%PATCHLEVEL%

SET PRIVATEBUILD=%STRPRIVATEBUILD:"=%
if %STRPRIVATEBUILD% == "" (
  set APPVER=%STRVER%
  set SAFEAPPVER=%STRVER%
) else (
  set APPVER=%STRVER%+-%PRIVATEBUILD%
  set SAFEAPPVER=%STRVER%-%PRIVATEBUILD%
)

setlocal enabledelayedexpansion
(
for /f "usebackq delims=" %%a in ("Version.in") do (
    call echo %%a
)
) > Version.h
endlocal