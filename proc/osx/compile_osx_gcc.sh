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

lib_dirs=(
	-L ../third_party/lib/release/osx/
)

# Not sure if I want to take this route or not...or if I want to bite the bullet and write my own platform layers...
libs=(
	-lSDL2
	-framework OpenGL
	-lm -lGLEW
	`sdl2-config --libs`
	-lglfw3
	-framework CoreFoundation 
	-framework CoreVideo 
	-framework IOKit 
	-framework Cocoa 
	-framework Carbon
)

# Build
gcc -O0 ${lib_dirs[*]} ${libs[*]} ${inc[*]} ${src[*]} ${flags[*]} -o Gunslinger

cd ..



