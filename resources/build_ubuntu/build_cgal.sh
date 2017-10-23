#!/bin/bash

# CGAL
cd $1
wget https://github.com/CGAL/cgal/archive/releases/CGAL-4.10.tar.gz
wget https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.10/md5sum.txt
md5sum -c md5sum.txt
rm md5sum.txt
tar -xf CGAL-4.10.tar.gz
rm CGAL-4.10.tar.gz
cd cgal-releases-CGAL-4.10

mkdir build
cd build
cmake ..
make

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1/cgal-releases-CGAL-4.10/build/lib
