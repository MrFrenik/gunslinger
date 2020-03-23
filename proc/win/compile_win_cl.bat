@echo off
rmdir /Q /S bin
mkdir bin
pushd bin

rem Include directories 
set inc=/I ..\include\ /I ..\third_party\include\ /I ..\

rem Source files
set src_main=..\source\*.c
set src_base=..\source\base\*.c
set src_serialize=..\source\serialize\*.c
set src_graphics=..\source\graphics\*.c
set src_platform=..\source\platform\*.c

rem Graphics specific plugin
set src_graphics_ogl=..\source\graphics\opengl\*.c

rem Platform specific plugin
set src_platform_glfw=..\source\platform\glfw\*.c

rem TP Source

rem All source together
set src_all=%src_main% %src_base% %src_graphics% %src_serialize% ^
%src_platform% %src_platform_glfw% %src_graphics_ogl%

rem Library directories
set lib_d=/LIBPATH:"..\third_party\lib\release\win\"

rem OS Libraries
set os_libs= opengl32.lib kernel32.lib user32.lib ^
shell32.lib vcruntime.lib msvcrt.lib gdi32.lib

rem Name
set name=Gunslinger

rem Compile options
set c_options=cl /MP /FS /Ox /W1 /Fe%name%.exe

rem Link options
set l_options=/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib

rem Compile library objects
cl /c /MP /FS /Ox /W1 %src_all% %inc% /EHsc

rem Compile library
lib *.obj /out:Gunslinger.lib

popd
