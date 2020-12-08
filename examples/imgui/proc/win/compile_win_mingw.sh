#!bin/sh

rm -rf bin
mkdir bin
cd bin

name=imgui
proj_root_dir=$(pwd)/../

flags=(
	-std=c++11 -w
)

# Include directories
inc=(
	-I ../../../include/		# Gunslinger includes
	-I ../source/
	-I ../../../third_party/include/
)

# Source files
src=(
	../source/main.cpp
	../source/imgui/*.cpp
)

lib_dirs=(
	-L ../../../bin/
)

libs=(
	-lGunslinger
	-lopengl32
	-lopengl32 
	-lkernel32 
	-luser32 
	-lshell32 
	-lgdi32 
	-lAdvapi32
	-lstdc++
)

# Build
g++ -O0 ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o ${name}

cd ..



