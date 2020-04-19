#!/bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=Gunslinger
proj_root_dir=$(pwd)/../../..

flags=(
	-std=c99
	# -x objective-c
)

# Include directories
inc=(
	-I $proj_root_dir 							# Gunslinger root
	-I $proj_root_dir/include/					# Gunslinger includes
	-I $proj_root_dir/third_party/include 		# Third Party includes
)

# Source files
src=(
	$proj_root_dir/source/main.c 
	$proj_root_dir/source/base/*.c
	$proj_root_dir/source/platform/*.c
	$proj_root_dir/source/graphics/*.c
	$proj_root_dir/source/serialize/*.c

	# Todo(John): Remove this into a plugin
	# ../source/platform/sdl/*.c
	$proj_root_dir/source/platform/glfw/*.c

	# Gfx plugin ( Again, remove from engine build )
	$proj_root_dir/source/graphics/opengl/*.c
)

# Build library
gcc -c -O0 ${flags[@]} ${inc[@]} ${src[@]} ${libs[@]}
ar -rvs lib${proj_name}.a *o 
ranlib lib${proj_name}.a
rm *.o

cd ..



