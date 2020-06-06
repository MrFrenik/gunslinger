#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99 -Wl,--no-as-needed -ldl -lGL -lX11 -pthread -lXi
)

# Include directories
inc=(
	-I ../../../include/		# Gunslinger includes
	-I ../../../third_party/include/
	-I ../include/
	-I ../source/
)

# Source files
src=(
	../source/main.c
	../source/font/*.c
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
gcc -O3 ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o Serialization

cd ..



