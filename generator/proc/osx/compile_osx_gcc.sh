#!bin/sh

cd bin

flags=(
	-std=c99
)

# Include directories
inc=(
	-I ../../include/	# Gunslinger includes
)

# Source files
src=(
	../source/*.c 
)

# Build
gcc -std=c99 -O3 ${inc[*]} ${src[*]} ${flags[*]} -o Generator

