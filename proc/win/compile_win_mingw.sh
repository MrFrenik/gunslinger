#!/bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=gunslinger
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99
	-w
)

# Include directories
inc=(
	-I ../ 							# Gunslinger root
	-I ../include/					# Gunslinger includes
	-I ../third_party/include 		# Third Party includes
)

# Source files
src=(
	../source/base/*.c
	../source/platform/*.c
	../source/graphics/*.c
	../source/serialize/*.c
	../source/audio/*.c

	# Todo(John): Remove this into a plugin
	# ../source/platform/sdl/*.c
	../source/platform/glfw/*.c

	# Gfx plugin (Again, remove from engine build)
	../source/graphics/opengl/*.c

	# Audio plugin
	../source/audio/miniaudio/*.c
)

# Build library
gcc -c -O3 ${flags[*]} ${inc[*]} ${src[*]}
ar -rcs ${proj_name}.lib *o 
ranlib ${proj_name}.lib
rm *.o

cd ..



