#!bin/sh

cd bin

proj_root_dir=$(pwd)/../

flags=(
	-std=c99
)

# Include directories
inc=(
	-I ../include/
)

# Source files
src=(
	../source/main.c 
	../source/base/*.c

	# Todo(John): Remove this into a plugin
	../source/platform/sdl/*.c
)

# Not sure if I want to take this route or not...or if I want to bite the bullet and write my own platform layers...
libs=(
	-lSDL2
	-framework OpenGL
	-lm -lGLEW
	-framework CoreFoundation 
	`sdl2-config --libs`
)

# Build
gcc -O0 ${libs[*]} ${inc[*]} ${src[*]} ${flags[*]} -o Gunslinger

cd ..



