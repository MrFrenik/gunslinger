#!bin/sh

rm -rf bin
mkdir bin
cd bin

name=hello_cpp
proj_root_dir=$(pwd)/../

flags=(
	-std=c++11 -w
)

# Include directories
inc=(
	-I ../../../include/		# Gunslinger includes
)

# Source files
src=(
	../source/main.cpp
)

lib_dirs=(
	-L ../../../bin/
)

fworks=(
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
)

# Build
g++ -O3 ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o ${name}

cd ..



