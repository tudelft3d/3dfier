#!/bin/bash

cd $1
# download the current release
# not that probably you'll need to update the link to the newest release
wget https://github.com/tudelft3d/3dfier/archive/v0.9.6.tar.gz
tar -xvf v0.9.6.tar.gz
cd 3dfier-0.9.6
mkdir build
cd build

# note that cmake might need to be run twice
cmake .. \
-DLIBLASC_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas_c.so \
-DLIBLAS_INCLUDE_DIR=$1/libLAS-1.8.1/build/include \
-DLIBLAS_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=$1/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=$1/laszip-src-2.2.0/build/lib/liblaszip.so

cmake .. \
-DLIBLASC_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas_c.so \
-DLIBLAS_INCLUDE_DIR=$1/libLAS-1.8.1/build/include \
-DLIBLAS_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=$1/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=$1/laszip-src-2.2.0/build/lib/liblaszip.so
make

#------
# clean up
rm $1/laszip-src-2.2.0.tar.gz $1/libLAS-1.8.1.tar.bz2 $1/v0.9.6.tar.gz
