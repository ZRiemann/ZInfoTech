@echo off

set VERSION_MAJOR=1
set VERSION_MINOR=1
set VERSION_REVISION=0
set VERSION_DATE=%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%
set VERSION_ALPHABET=alpha
set AUTO_VERSION=..\..\..\src\auto_version.h
FOR /F "usebackq delims=" %%i IN (`git rev-parse HEAD`) DO set GIT_REV=%%i

if "%1"=="rm-auto-version" (
	del %AUTO_VERSION%
) else ( 
	echo #define VER_AUTO 1 > %AUTO_VERSION%
	echo const int major_version=%VERSION_MAJOR%; >> %AUTO_VERSION%
	echo const int minor_version=%VERSION_MINOR%; >> %AUTO_VERSION%
	echo const int revision_version=%VERSION_REVISION%; >> %AUTO_VERSION%
	echo const char *version="V%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_REVISION%.%VERSION_DATE%_%VERSION_ALPHABET%"; >> %AUTO_VERSION%
	echo const char *build_date="%VERSION_DATE% %TIME%"; >> %AUTO_VERSION%
	echo const char *git_rev="GIT_REV: %GIT_REV%"; >> %AUTO_VERSION%
)