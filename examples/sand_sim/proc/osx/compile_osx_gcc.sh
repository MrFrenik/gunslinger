#!bin/sh

rm -rf bin
mkdir bin
cd bin

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

fworks=(
	-framework OpenGL
	-framework CoreFoundation 
	-framework CoreVideo 
	-framework IOKit 
	-framework Cocoa 
	-framework Carbon
)

libs=(
	-lGunslinger
)

# Build
gcc -O3 ${lib_dirs[*]} ${libs[*]} ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} -o SandSim

cd ..



