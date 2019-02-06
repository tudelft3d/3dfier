---
layout: default
title: Windows compilation
group: getting-started
---

This guide will talk you through the compilation of 3dfier on Windows 10 64-bit using Visual Studio 2017 (steps are identical for Visual Studio 2015).

There are some steps to be taken to prepare the build environment for 3dfier. Most important is installing software to compile and downloading the libraries 3dfier is dependent of.

## 1. Running installers
First you will need to download and install in their default directorties:
1. [Visual Studio Community 2017](https://www.visualstudio.com/downloads/). Install at least the C++ part.
1. [CMake](https://cmake.org/download/), download and install `Windows win64-x64 Installer`. Add variable to the PATH during installation.
1. [Boost precompiled binaries](https://sourceforge.net/projects/boost/files/boost-binaries). Pick the latest version (`boost_1_69_0-msvc-14.0-64.exe` at the time of writing). If you build on VS2015 get *mscv-14.0*, for VS2017 get *mscv-14.1*. Install boost.
1. [OSGeo4W](https://trac.osgeo.org/osgeo4w), download the [64-bit installer](http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe). From this package you will need to install at least GDAL.
1. [CGAL](https://github.com/CGAL/cgal/releases), download `CGAL-4.12-Setup.exe` and install. Select *GMP and MPFR precompiled libs*, *Environment variables to set CGAL_DIR* and *Add CGAL/auxilary/gmp/lib to the PATH* during setup.

## 2. Compilation of dependencies
Next, we need to download and compile Yaml-cpp and LAStools. 

### Yaml-cpp
Download [yaml-cpp 0.5.3](https://github.com/jbeder/yaml-cpp/releases) and extract to e.g. `C:\deps\yaml-cpp`. There are two options of getting the Visual Studio project files using CMake:

1. using CMake GUI ([tutorial here](https://cmake.org/runningcmake/)).

1. using command line. 
Open a Command prompt (press windows button+R, type cmd and press enter) and navigate to the yaml-cpp directory:
```
cd C:\deps\yaml-cpp
```
Generate the Visual Studio build files with
```
mkdir vs_build
cd vs_build
cmake .. -G "Visual Studio 15 2017 Win64"
```

After generation open the Visual Studio file `YAML_CPP.sln`. Set the solution configuration to `Release` in the main toolbar. From the menu bar select Build and click `Build Solution`.

### LAStools
Download [LAStools](https://rapidlasso.com/lastools/) and extract to e.g. `C:\deps\lastools`. There are two options of getting the Visual Studio project files:

1. Go to the folder where you extacted LAStools using file explorer (C:\deps\lastools). Enter subfolder LASlib and open `LASlib.dsp`. Now Visual Studio will automatically open and asks for a one time upgrade. Choose Yes to proceed with the upgrade. After the upgrade save and close the solution.
1. Use CMake as explained previous for [Yaml-cpp](#yaml-cpp) to generate the Visual Studio solution files.

After generation open the Visual Studio file `LASlib.sln`. Set the solution configuration to `Release` in the main toolbar. From the menu bar select Build and click `Build Solution`.

## 3. Set environment variables
Go to `Control Panel > System > Advanced system settings > Environment Variables` and add the following user variables. Note that the version numbers may be different!
* `BOOST_ROOT`=`C:\boost_1_69_0`
* `BOOST_LIBRARYDIR`=`C:\boost_1_69_0\lib64-msvc-14.0`
* `CGAL_DIR`=`C:\dev\CGAL-4.12`
* `GDAL_ROOT`=`C:\OSGeo4W64`
* `LALIB_ROOT`=`C:\deps\lastools\LASlib`
* `LASZIP_ROOT`=`C:\deps\lastools\LASzip`
* `OSGEO4W_ROOT`=`C:\OSGeo4W64`
* `YAML-CPP_DIR`=`C:\deps\yaml-cpp`

Go to `Control Panel > System > Advanced system settings > Environment Variables` and add the following directory to Path.
* `C:\OSGeo4W64\bin`

## 4. Compile 3dfier
Download and extract the source code from the menu on the left or fork directly from GitHub. Browse to the vs_build folder and open the Visual Studio file `3dfier.sln`.

If in any case the Visual Studio solution is not working its possible to generate them from the CMake files directly as explained previous for [Yaml-cpp](#yaml-cpp).

## 5. Run 3dfier!
If all is good you should now be able to run 3dfier! Go to [Examples]({{site.baseurl}}/pages/examples) and start producing models.

* * * 
### Help, Visual Studio complains that some file can not be found!
Check whether the directories and files specified in the environment variables are correct. Also check these places in Visual Studio:
* the include folders in `Project > Properties > C/C++ > General > Additional Include Directories`
* the library folders in `Project > Properties > Linker > General > Additional Library Directories`
* the libraries files in `Project > Properties > Linker > Input > Additional Dependencies`

Make sure each directory or file exists on your drive. For example: you may need to change a version number somewhere.