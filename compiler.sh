#!/bin/sh

# @file compiler.sh 
# @breif compiler c/c++(.c/.cpp/.cc/.cxx) files to object files(.o)
# @useage compiler <src_dir> <obj_dir> <CC> <FLAGS...>
# @note
#  comiple all c/c++ source file from <src> to <dest> obj file.
#  src_dir:src; obj_dir:make/obj; out_dir:make/bin;
#  sample: ./compiler src make/obj gcc -g -Wall -Werror -I./src
#   .C, .cc, .cpp, .CPP, .c++, .cp, or .cxx
# @optimize
#  . paralle compile
#  . check head file modify 

SRC_DIR=$1
OBJ_DIR=$2
CC=$3
CFLAGS=$4

compile(){
    if [ "$CC" = "g++" ]; then
	echo "g++ -c $CFLAGS -o $OBJ_DIR/${NAME%.*}.o $FILE"
	g++ -c $CFLAGS -o $OBJ $FILE
    else
	echo "gcc -c $CFLAGS -o $OBJ_DIR/${NAME%.*}.o $FILE"
	gcc -c $CFLAGS -o $OBJ $FILE	    
    fi    
}

for FILE in $(find $SRC_DIR -type f -name \*.c -o -name \*.C -o -name \*.cc -o -name \*.cpp -o -name \*.CPP -o -name \*.c++ -o -name \*.cp -o -name \*.cxx)
do
    NAME=${FILE##*/}
    OBJ=$OBJ_DIR/${NAME%.*}.o
    if [ -f $OBJ ]; then
	if [ $FILE -nt $OBJ ]; then
	    compile
	fi
    else
	compile
    fi
done

# check if headfile newer to source file(task delay)

#for FILE in $(find $SRC_DIR -type f -name *.h -o -name *.hh -o -name *.H -o -name *.hp -o -name *.hxx -o -name *.hpp -o -name *.HPP -o -name *.h++ -o -name *.tcc)
#do
#    echo $FILE
#done

exit 0
