#!bin/sh

rm -rf bin
mkdir bin
cd bin

name=sand_sim
proj_root_dir=$(pwd)/../

flags=(
	-std=c99
)

# Include directories
inc=(
	-I ../../../include/				# Gunslinger includes
	-I ../include/						# SandSim includes
)

# Source files
src=(
	../source/main.c
	../source/render_passes/*.c
)

lib_dirs=(
	-L ../../../bin/
)

libs=(
	-lgunslinger
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



