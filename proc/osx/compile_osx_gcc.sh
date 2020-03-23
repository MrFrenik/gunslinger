#!bin/sh

rm -rf bin
mkdir bin
cd bin

proj_name=Gunslinger
proj_root_dir=$(pwd)/../

flags=(
	-std=c99
	-x objective-c
)

# Include directories
inc=(
	-I ../ 							# Gunslinger root
	-I ../include/					# Gunslinger includes
	-I ../third_party/include 		# Third Party includes
)

# Source files
src=(
	../source/main.c 
	../source/base/*.c
	../source/platform/*.c
	../source/graphics/*.c
	../source/serialize/*.c

	# Todo(John): Remove this into a plugin
	# ../source/platform/sdl/*.c
	../source/platform/glfw/*.c

	# Gfx plugin ( Again, remove from engine build )
	../source/graphics/opengl/*.c
)

# ext_src=(
# 	../third_party/source/GLFW/*.c
# 	../third_party/source/GLFW/*.m
# )

# lib_dirs=(
# 	-L ../third_party/lib/release/osx/
# )

# Need to compile ALL 3rd party depedency code into Gunslinger
# By default, will include: 
	# Platform -> GLFW
	# Video -> OpenGL (via glad)
	# Audio
libs=(
	# -lSDL2
	# -framework OpenGL
	# -lm -lGLEW
	# `sdl2-config --libs`
	# -lglfw3
	# -framework CoreFoundation 
	# -framework CoreVideo 
	# -framework IOKit 
	# -framework Cocoa 
	# -framework Carbon
)

# Build
# gcc -O0 ${lib_dirs[*]} ${libs[*]} ${inc[*]} ${src[*]} ${flags[*]} -o Gunslinger

# Build library
gcc -c -O3 ${flags[*]} ${inc[*]} ${src[*]} ${libs[*]}
ar -rvs lib${proj_name}.a *o 
ranlib lib${proj_name}.a
rm *.o

cd ..



