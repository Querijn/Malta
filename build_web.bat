@echo off

REM You can change these settings if you'd like.
SET PROD_FOLDER=build\web_prod
SET DEBUG_FOLDER=build\web_debug
SET BASE_FOLDER=build

REM Setting default config to "prod", otherwise, it's the first argument
if "%1"=="" (SET CONFIG=prod) else (SET CONFIG=%1)
SET ORIG_DIR=%cd%

call emsdk.bat activate latest --permanent

mkdir %BASE_FOLDER% 2>nul

if not exist %DEBUG_FOLDER% (
	echo Configuring Debug...
	cmake -S %ORIG_DIR% -DCMAKE_TOOLCHAIN_FILE="%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake" -B %DEBUG_FOLDER% -G Ninja .
)
if not exist %PROD_FOLDER% (
	echo Configuring Release...
	cmake -S %ORIG_DIR% -DCMAKE_TOOLCHAIN_FILE="%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake" -B %PROD_FOLDER% -G Ninja .
)

:BUILD
if "%CONFIG%" == "debug" (cmake --build %DEBUG_FOLDER% --config Debug)
if "%CONFIG%" == "prod" (cmake --build %PROD_FOLDER% --config Release)
cd /d %ORIG_DIR%
goto COMMON_EXIT

:INSTRUCTIONS
echo Usage: web_build debug/prod (rebuild)

:COMMON_EXIT
echo Done.
REM pause