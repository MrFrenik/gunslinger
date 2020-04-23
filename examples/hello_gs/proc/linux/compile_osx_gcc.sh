#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -Wl,--no-as-needed -ldl -lGL -lGLU -lX11 -pthread -lXi
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

fworks=(
)

libs=(
	-lGunslinger
)

# Build
gcc -O3 ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o HelloGS

cd ..



