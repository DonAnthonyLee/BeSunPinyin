if "x%1" == "x" (set SRCPATH=..) else (set SRCPATH=%1)

@set SOURCES=SunPinyinHandler.cpp SunPinyinModule.cpp

@set OUTPUT_PATH=%SRCPATH%\build\Win32\projects

@set CONVERT_TOOL=%SRCPATH%\utils\utf8_hex.exe

@md "%OUTPUT_PATH%"
@for %%i in (%SOURCES%) do "%CONVERT_TOOL%" "%SRCPATH%\%%i" -r -oa "%OUTPUT_PATH%\%%~ni-VC.cpp"

