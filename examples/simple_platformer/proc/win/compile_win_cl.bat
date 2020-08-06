@echo off
rmdir /Q /S bin
mkdir bin
pushd bin

rem Name
set name=SimplePlatformer

rem Include directories 
set inc=/I ..\..\..\include

rem Source files
set src_main=..\source\*.c

rem All source together
set src_all=%src_main%

rem Library directories
set lib_d=/LIBPATH:"..\..\..\bin\"

rem OS Libraries
set os_libs= opengl32.lib kernel32.lib user32.lib ^
shell32.lib vcruntime.lib msvcrt.lib gdi32.lib

rem User Libraries
set libs=Gunslinger.lib

rem Link options
set l_options=/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib

rem Compile
cl /MP /FS /Ox /W1 /Fe%name%.exe %src_all% %inc% ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:LIBCMT ^
%lib_d% %libs% %os_libs%

rem Compile Debug
rem cl /w /MP -Zi /DEBUG:FULL /Fe%name%.exe %src_all% %inc% ^
rem /EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:LIBCMT ^
rem %lib_d% %libs% %os_libs%

popd
