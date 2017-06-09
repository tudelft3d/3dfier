#!/bin/bash

# Install script for 3dfier on Ubuntu 16.04 LTS
# Parameters:
#   $1 – path to Source Directory where 3dfier will be installed, and source files
# of LASzip and libLAS downloaded, e.g. /opt
# Usage:
#   sudo ./build_ubuntu.sh /opt

# You need to have write permission on the Source Directory
# You need to run the sript as root, because:
# LASzip and libLAS installed to /usr/local
# 3dfier is installed to Source Directory

# The script downloads the source files of LASzip, libLAS and 3dfier. You might
# need to update the link to the latest 3dfier below.

# Install CGAL, GDAL, yaml-cpp, boost on your own.
# If you have the ubuntugis-(un)stable PPA added to your repositories, you can easily
# install these with sudo apt install ...
# Or you can use e.g. Synaptic Package Manager to do the same.
# you'll need libgdal, libcgal, libboost, libyaml-cpp0.

# GRASS GIS
# If you already have GRASS installed from the ubuntugis-(un)stable PPA, you
# also have libLAS (liblas-c3, liblas3) installed, as GRASS depends on them.
# However, this libLAS install is without LASzip support as LASzip is not part
# of the ubuntugis PPA. Therefore, you will need to remove GRASS and the libLAS
# libraries first, then compile libLAS with LASzip support (with this script). 
# Then you can install GRASS again from the ubuntugis PPA.


# -------
# LASzip
# note that LASzip need to be compiled before libLAS

cd $1
wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz
wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz.md5
md5sum -c laszip-src-2.2.0.tar.gz.md5
rm laszip-src-2.2.0.tar.gz.md5
tar -xvf laszip-src-2.2.0.tar.gz
cd laszip-src-2.2.0

# The Makefile need to be modified in order to be compliant to what libLAS
# is looking for. https://github.com/libLAS/libLAS/issues/9
sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.am
sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.in

# Store the compiled sofware within its directory instead of distributing
# the files in the filesystem. It is easier for libLAS to locate and link
# the executables this way.
./configure
make
make install
make clean

#--------
# libLAS

cd $1
wget http://download.osgeo.org/liblas/libLAS-1.8.1.tar.bz2
# for some reason the md5 file was not accessible on osgeo when I checked it
# wget http://download2.osgeo.org/liblas/libLAS-1.8.1.tar.bz2.md5
# md5sum -c libLAS-1.8.1.tar.bz2.md5
# rm libLAS-1.8.1.tar.bz2.md5
tar -xvf libLAS-1.8.1.tar.bz2
cd libLAS-1.8.1
mkdir cmake_build
cd cmake_build
# Compile with GDAL and LASzip. GDAL should be found automatically, thus no
# need to provide the link here.
cmake .. \
-DWITH_GDAL=ON \
-DWITH_LASZIP=ON \
-DWITH_STATIC_LASZIP=ON
make
make install
make clean

#-------
# 3dfier

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
-DLIBLAS_INCLUDE_DIR=/usr/local/include/liblas \
-DLIBLAS_LIBRARY=/usr/local/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=/usr/local/include/laszip \
-DLASZIP_LIBRARY=/usr/local/lib/liblaszip.so
make

#------
# clean up
rm $1/laszip-src-2.2.0.tar.gz $1/libLAS-1.8.1.tar.bz2 $1/v0.9.6.tar.gz

# run 3dfier
# $1/build/3dfier


