#!bin/sh

rm -rf bin
mkdir bin
cd bin

name=simple_texture
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -w
)

# Include directories
inc=(
	-I ../../../include/		# Gunslinger includes
)

# Source files
src=(
	../source/main.c
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
)

# Build
gcc -O3 ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o ${name}

cd ..



