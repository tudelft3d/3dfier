---
title: Installation
keywords: 3dfier installation ubuntu docker windows compile
summary: These instructions will help you to install 3dfier on various operating systems. For Windows please use the binary files and do not compile from
sidebar: 3dfier_sidebar
permalink: installation.html
---

**TODO: CHANGE LIBLAS TO LASLIB for Ubuntu 16.04!!**

## Install on Windows using binaries
Binary releases exist only for Windows users. Others will have to follow one of the other installation guides for [Linux](#ubuntu-1604) or [Docker](#docker)
There exists a ready-to-use version of [3dfier for Windows 64-bit](https://github.com/{{site.repository}}/releases/latest). Download and extract the files to any given folder and follow the instructions in the [Get started guide]({{site.baseurl}}/index).

### Release binaries content
Description of files in the zip file

All dll files distributed with 3dfier belong to GDAL or other packages used in the GDAL drivers. Other depencencies used are statically built within the executable. 

Filename | Package
---------|--------
3dfier.exe | 3dfier
expat.dll | GDAL
freexl.dll | GDAL
gdal204.dll | GDAL
geos.dll | GDAL
geos_c.dll | GDAL
hdf5.dll | GDAL
hdf5_hl.dll | GDAL
iconv-2.dll | GDAL
iconv.dll | GDAL
jpeg.dll | GDAL
libcurl.dll | GDAL
libeay32.dll | GDAL
libgmp-10.dll | GDAL
liblzma.dll | GDAL
libmpfr-4.dll | GDAL
libmysql.dll | GDAL
libpng16.dll | GDAL
libpq.dll | GDAL
libxml2.dll | GDAL
lwgeom.dll | GDAL
netcdf.dll | GDAL
ogdi.dll | GDAL
openjp2.dllv | GDAL
proj_5_2.dll | GDAL
spatialite.dll | GDAL
sqlite3.dll | GDAL
ssleay32.dll | GDAL
szip.dll | GDAL
xerces-c_3_2.dll | GDAL
zlib1.dll | GDAL
zstd.dll | GDAL


## Docker
The Dockerfile for building 3dfier (`Dockerfile_builder`) contains the complete instruction for setting up a build environment. If you don't want to install the required dependencies on your local machine, you can use this docker image to build and run 3dfier. If you want to build 3dfier locally, then please look into the Dockerfile for the libraries that you'll need.

The builder image (`Dockerfile_builder`) does not contain the source code of 3dfier, neither the executable. We use this image for compiling 3dfier from different branches during development and testing. Nevertheless, it is suitable for compiling from source and 3dfying any data set.

As an example, the complete process of building the docker image, compiling 3dfier and running looks like the following.


### 1. Build the docker image
```
$ docker build -t 3dfier:builder -f Dockerfile_builder
```

### 2. Prepare a compiler script for 3dfier
The contents of `build_3dfier.sh`

```
#! /bin/sh

rm -rf /opt/3dfier_src/build; \
cd /opt/3dfier_src && mkdir build && cd ./build; \
cmake \
-DLIBLASC_LIBRARY=/opt/libLAS-1.8.1/build/lib/liblas_c.so \
-DLIBLAS_INCLUDE_DIR=/opt/libLAS-1.8.1/build/include \
-DLIBLAS_LIBRARY=/opt/libLAS-1.8.1/build/lib/liblas.so \
-DLASZIP_INCLUDE_DIR=/opt/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=/opt/laszip-src-2.2.0/build/lib/liblaszip.so \
-DCGAL_DIR=/opt/cgal-releases-CGAL-4.12/build \
-DCMAKE_BUILD_TYPE=Release ../3dfier \
; \
make
```

### 3. Compile 3dfier in a container
If we set up an out of source build system in a root directory `3dfier_src` with the following sturcture,

```
/3dfier_src
├── 3dfier
├── build

```
then we execute the commands from the root dir. The `3dfier_src` dir is *bind-mount*-ed to the container where 3dfier is complied by running `build_3dfier.sh`. The script will place the executable into the `build` directory in `3dfier_src`.

```
$ docker run --rm -it -v "$(pwd)":/opt/3dfier_src 3dfier:builder /opt/3dfier_src/build_3dfier.sh
```

### 4. Running 3dfier in a container
Then the previously built executable is mounted to the container and executed there. Note that the data and config files are also mounted in a similar fashion.

```
$ docker run --rm -it -v "$(pwd)":/opt/3dfier_src 3dfier:builder /opt/3dfier_src/build/3dfier /opt/3dfier_src/3dfier/example_data/testarea_config_docker.yml --CSV-BUILDINGS /opt/3dfier_src/test.csv
```

## Ubuntu 16.04
### 1. Adding *ubuntugis* repositories
To install *GDAL* on Ubuntu 16.04 LTS it is probably the easiest to add one of the *ubuntugis* repositories, either [*ubuntugis-stable*](https://launchpad.net/~ubuntugis/+archive/ubuntu/ppa?field.series_filter=xenial) or [*ubuntugis-unstable*](https://launchpad.net/~ubuntugis/+archive/ubuntu/ubuntugis-unstable?field.series_filter=xenial). Both contains *GDAL* >= 2.1.

E.g. add the *ubuntugis-stable* repository by running:
```
sudo add-apt-repository ppa:ubuntugis/ppa
sudo apt-get update
```

### 2. Install dependencies
*CGAL* (`libcgal-dev`), *Boost* (`libboost-all-dev`) and *yaml-cpp* (`libyaml-cpp0.5v5`) are part of the *Ubuntu Universe* repository.

Once you have all the repos added, you can use a package manager, e.g. `apt` or *Synaptic* to install them. 

E.g. using apt-get:
```
sudo apt-get install libcgal-dev libboost-all-dev libyaml-cpp0.5v5
```

### 3. Run the build script 
*LASzip*, *libLAS* and *3dfier* need to be built manually in this order, and this is what the [build script](https://github.com/tudelft3d/3dfier/blob/build_ubuntu_2/ressources/build_ubuntu1604.sh) can do for you. It downloads and compiles these three packages, and takes care that *libLAS* is compiled with *LASzip* support and that *3dfier* can find the executables of both. We try to take care that the download links are up to date with the upcoming releases. However, if you notice that the links are outdated, just update the version number in the script.

To download and run the [build script](https://github.com/tudelft3d/3dfier/blob/build_ubuntu_2/ressources/build_ubuntu1604.sh) do:
```
wget https://raw.githubusercontent.com/tudelft3d/3dfier/build_ubuntu_2/ressources/build_ubuntu1604.sh
sudo build_ubuntu1604.sh /opt
```

Where `/opt` is the directory where *3dfier* will be installed and *LASzip*, *libLAS* downloaded and compiled. *LASzip*, *libLAS* is installed into `/usr/local`, hence you'll probably need root privilage to run the script.

**A note on GRASS GIS and libLAS**\
If you already have GRASS installed from the *ubuntugis-(un)stable* PPA, you probably also have libLAS (`liblas-c3, liblas3`) installed, as GRASS depends on them. However, this *libLAS* install is without *LASzip* support as *LASzip* is not part of the *ubuntugis* PPA. Therefore, you will need to remove the GRASS and *libLAS* libraries first, then compile *libLAS* with *LASzip* support (with this script). Then you can install GRASS again from the *ubuntugis* PPA, it will be compiled with *libLAS* that now supports `.laz`.

## Windows
This guide will talk you through the compilation of 3dfier on Windows 10 64-bit using Visual Studio 2017 (steps are identical for Visual Studio 2015).

There are some steps to be taken to prepare the build environment for 3dfier. Most important is installing software to compile and downloading the libraries 3dfier is dependent of.

### 1. Running installers
First you will need to download and install in their default directorties:
1. [Visual Studio Community (2017 or later)](https://www.visualstudio.com/downloads/). Install at least the C++ part.
1. [CMake (3.15 or later)](https://cmake.org/download/), download and install `Windows win64-x64 Installer`. Add variable to the PATH during installation.
1. [Boost precompiled binaries (1.71 or later)](https://sourceforge.net/projects/boost/files/boost-binaries). Pick the latest version. If you build on VS2015 get *mscv-14.0*, for VS2017 get *mscv-14.1*. Install boost using the installer.
1. [OSGeo4W (with GDAL 2.3.0 or later)](https://trac.osgeo.org/osgeo4w), download the [64-bit installer](http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe). From this package you will need to install at least the GDAL package.
1. [CGAL (4.12 or later)](https://github.com/CGAL/cgal/releases), download `CGAL-4.12-Setup.exe` (or newer) and install. Select *GMP and MPFR precompiled libs*, *Environment variables to set CGAL_DIR* and *Add CGAL/auxilary/gmp/lib to the PATH* during setup.

### 2. Compilation of dependencies
Next, we need to download and compile Yaml-cpp and LAStools. 

#### Yaml-cpp
Download [yaml-cpp (0.5.3 or later)](https://github.com/jbeder/yaml-cpp/releases) and extract to e.g. `C:\dev\yaml-cpp`. There are two options of getting the Visual Studio project files using CMake:

1. using CMake GUI ([tutorial here](https://cmake.org/runningcmake/)).

1. using command line. 
Open a Command prompt (press windows button+R, type cmd and press enter) and navigate to the yaml-cpp directory:
```
cd C:\dev\yaml-cpp
```
Generate the Visual Studio build files with
```
mkdir vs_build
cd vs_build
cmake .. -G "Visual Studio 15 2017 Win64"
```

After generation open the Visual Studio file `YAML_CPP.sln`. Set the solution configuration to `Release` in the main toolbar. From the menu bar select Build and click `Build Solution`.

#### LAStools
Download [LAStools](https://rapidlasso.com/lastools/) and extract to e.g. `C:\dev\lastools`. There are two options of getting the Visual Studio project files:

1. Go to the folder where you extacted LAStools using file explorer (C:\dev\lastools). Enter subfolder LASlib and open `LASlib.dsp`. Now Visual Studio will automatically open and asks for a one time upgrade. Choose Yes to proceed with the upgrade. After the upgrade save and close the solution.
1. Use CMake as explained previous for [Yaml-cpp](#yaml-cpp) to generate the Visual Studio solution files.

After generation open the Visual Studio file `LASlib.sln`. Set the solution configuration to `Release` in the main toolbar. From the menu bar select Build and click `Build Solution`.

### 3. Set environment variables
Go to `Control Panel > System > Advanced system settings > Environment Variables` and add the following user variables. Note that the version numbers may be different!
* `BOOST_ROOT`=`C:\boost_1_71_0`
* `BOOST_LIBRARYDIR`=`C:\boost_1_71_0\lib64-msvc-14.0`
* `CGAL_DIR`=`C:\dev\CGAL-4.12`
* `GDAL_ROOT`=`C:\OSGeo4W64`
* `LASLIB_ROOT`=`C:\dev\lastools\LASlib`
* `LASZIP_ROOT`=`C:\dev\lastools\LASzip`
* `OSGEO4W_ROOT`=`C:\OSGeo4W64`
* `YAML-CPP_DIR`=`C:\dev\yaml-cpp`

Go to `Control Panel > System > Advanced system settings > Environment Variables` and add the following directory to Path.
* `C:\OSGeo4W64\bin`

### 4. Compile 3dfier
Download and extract the source code from the menu on the left or fork directly from GitHub. Browse to the vs_build folder and open the Visual Studio file `3dfier.sln`.

If in any case the Visual Studio solution is not working its possible to generate them from the CMake files directly as explained previous for [Yaml-cpp](#yaml-cpp).

### 5. Run 3dfier!
If all is good you should now be able to run 3dfier! Go to the [First run]({{site.baseurl}}/first_run) and start producing models.

* * * 
#### Help, Visual Studio complains that some file can not be found!
Check whether the directories and files specified in the environment variables are correct. Also check these places in Visual Studio:
* the include folders in `Project > Properties > C/C++ > General > Additional Include Directories`
* the library folders in `Project > Properties > Linker > General > Additional Library Directories`
* the libraries files in `Project > Properties > Linker > Input > Additional Dependencies`

Make sure each directory or file exists on your drive. For example: you may need to change a version number somewhere.
