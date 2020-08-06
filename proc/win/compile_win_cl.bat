@echo off
rmdir /Q /S bin
mkdir bin
pushd bin

rem Include directories 
set inc=/I ..\include\ /I ..\third_party\include\ /I ..\

rem Source files
set src_base=..\source\base\*.c
set src_serialize=..\source\serialize\*.c
set src_graphics=..\source\graphics\*.c
set src_platform=..\source\platform\*.c
set src_audio=..\source\audio\*.c

rem Graphics specific plugin
set src_graphics_ogl=..\source\graphics\opengl\*.c

rem Audio specific plugin
rem set src_audio_wasapi=..\source\audio\wasapi\*.c
set src_audio_plugin=..\source\audio\miniaudio\*.c

rem Platform specific plugin
set src_platform_glfw=..\source\platform\glfw\*.c

rem TP Source

rem All source together
set src_all=%src_base% %src_graphics% %src_serialize% ^
%src_platform% %src_platform_glfw% %src_graphics_ogl% ^
%src_audio% %src_audio_plugin%

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
rem cl /c /Zi /MP /FS /Od /DEBUG:FULL /W1 %src_all% %inc% /EHsc

rem Compile library
lib *.obj /out:Gunslinger.lib

popd
