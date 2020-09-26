@echo off

set EXE_NAME=sim.exe

set SRC_DIR=..\src\

set SDL_DIR=C:\SDL2-2.0.10\

:: Debug messages etc
set ADDITIONAL_FLAGS=/DSTDOUT_DEBUG /DFIXED_GAME_MEMORY /DPLATFORM_GL_MAJOR_VERSION=3 /DPLATFORM_GL_MINOR_VERSION=3

:: Disabled warnings
:: /wd4100  unreferenced formal parameter
:: /wd4189  local variable is initialized but not referenced
:: /wd4996  strncat warning TODO put this back when we have our own string functions
set DISABLED_WARNINGS=/wd4100 /wd4189 /wd4996

:: Compiler flags
:: /Oi		compiler intrinsics
:: /GR- 	no runtime type info
:: /EHa- 	turn off exception handling
:: /nologo 	turn off compiler banner thing
:: /W4		warning level 4
:: /MT		statically link C runtime into executable for portability
:: /Gm-		Turn off incremental build
:: /Z7		Old style debugging info
:: /Fm		Generate map file
:: /I       Additional include directory (SDL headers)
:: not set
:: /P       Preprocessor output to file
set COMMON_COMPILER_FLAGS=/Oi /GR- /EHa- /nologo /W4 /MT /Gm- /Z7 /Fm %DISABLED_WARNINGS% /I ..\src\include /I %SDL_DIR%\include

:: Linker flags
:: /opt:ref         remove unneeded stuff from .map file
:: /LIBPATH:        library directories
:: .libs            libraries to include
:: /SUBSYSTEM:      type of exe - CONSOLE is a win32 character-mode application, WINDOWS is a windowed app that doesn't have a console...etc
:: /INCREMENTAL:NO  perform a full link
set COMMON_LINKER_FLAGS=/INCREMENTAL:NO /SUBSYSTEM:CONSOLE /LIBPATH:%SDL_DIR%\lib\x64 SDL2.lib SDL2main.lib

:: Create build directory and copy SDL2.dll in case it isn't there
IF NOT EXIST build mkdir build
cd build
:: Get SDL runtime libraries
IF NOT EXIST SDL2.dll copy %SDL_DIR%\lib\x64\SDL2.dll .

IF EXIST %EXE_NAME% del %EXE_NAME%

:: Build executable
cl %SRC_DIR%\sdl_main.cpp %SRC_DIR%\game.cpp %SRC_DIR%\gl_rendering.cpp %SRC_DIR%\math.cpp %SRC_DIR%\glad.c %COMMON_COMPILER_FLAGS% %ADDITIONAL_FLAGS% /link %COMMON_LINKER_FLAGS%

cd ..