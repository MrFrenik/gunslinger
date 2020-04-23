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
	-I ../source/noise/src/     # Perlin noise libary includes
)

# Source files
src=(
	../source/main.c
	../source/noise/src/sdnoise1234.c
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
gcc -O3 ${fworks[*]} ${inc[*]} ${src[*]} ${flags[*]} ${lib_dirs[*]} ${libs[*]} -lm -o Terrain
<<<<<<< HEAD

=======
>>>>>>> fec05c4df52a8c83e2366740543fb6fc00868ae4
cd ..



