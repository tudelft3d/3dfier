#!/bin/bash

cd $1
# download the current release
# not that probably you'll need to update the link to the newest release
# TODO: Fetch master and build it
wget https://github.com/tudelft3d/3dfier/archive/v0.9.7.tar.gz
tar -xf v0.9.7.tar.gz
rm v0.9.7.tar.gz
cd 3dfier-0.9.7
mkdir build
cd build

# note that cmake might need to be run twice
cmake .. \
-DLIBLASC_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas_c.so \
-DLIBLAS_INCLUDE_DIR=$1/libLAS-1.8.1/build/include \
-DLIBLAS_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=$1/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=$1/laszip-src-2.2.0/build/lib/liblaszip.so \
-DCGAL_DIR=$1/cgal-releases-CGAL-4.10/build


cmake .. \
-DLIBLASC_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas_c.so \
-DLIBLAS_INCLUDE_DIR=$1/libLAS-1.8.1/build/include \
-DLIBLAS_LIBRARY=$1/libLAS-1.8.1/build/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=$1/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=$1/laszip-src-2.2.0/build/lib/liblaszip.so \
-DCGAL_DIR=$1/cgal-releases-CGAL-4.10/build
make

cd ../example_data
../build/3dfier testarea_config.yml -o output/testarea.obj | grep -iq "Successfully terminated"
