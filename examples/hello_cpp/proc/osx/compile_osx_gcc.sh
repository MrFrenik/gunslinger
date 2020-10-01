#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_root_dir=$(pwd)/../

flags=(
	-std=c++11
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
g++ -O3 ${lib_dirs[*]} ${libs[*]} ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} -o HelloCPP

cd ..



