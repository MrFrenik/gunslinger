#!/bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=Gunslinger
proj_root_dir=$(pwd)/../

flags=(
	-std=gnu99
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

	# Gfx plugin ( Again, remove from engine build )
	../source/graphics/opengl/*.c

	# Alsa plugin
	../source/audio/alsa/*.c
)

# Build library
gcc -c -O0 ${flags[*]} ${inc[*]} ${src[*]} ${libs[*]}
ar -rcs lib${proj_name}.a *o 
ranlib lib${proj_name}.a
rm *.o

cd ..



