#!/bin/bash

# Install script for 3dfier on Ubuntu 16.04 LTS
# Parameters:
#   $1 – path to Source Directory where LASzip, libLAS, 3dfier will be installed, e.g. /opt
# Usage:
#   ./build_ubuntu.sh /opt

# You need to have write permission on the Source Directory
# The script downloads the source files of LASzip, libLAS and 3dfier. You might
# need to update the link to the latest 3dfier below.

# Install CGAL, GDAL, yaml-cpp, boost on your own.
# If you have the ubuntugis-(un)stable PPA added to your repositories, you can easily
# install these with sudo apt install ...
# Or you can use e.g. Synaptic Package Manager to the same.
# you'll need libgdal, libcgal, libboost, libyaml-cpp0.5

# CGAL
cd $1
wget https://github.com/CGAL/cgal/archive/releases/CGAL-4.10.tar.gz
wget https://github.com/CGAL/cgal/releases/download/releases%2FCGAL-4.10/md5sum.txt
md5sum -c md5sum.txt
rm md5sum.txt
tar -xf CGAL-4.10.tar.gz
cd cgal-releases-CGAL-4.10

mkdir build
cd build
cmake ..
make -j `nproc`

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1/cgal-releases-CGAL-4.10/build/lib
