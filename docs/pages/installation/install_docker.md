---
title: Install 3dfier using a Docker image
keywords: install docker
sidebar: 3dfier_sidebar
permalink: install_docker.html
---
The Dockerfile for building 3dfier (`Dockerfile_builder`) contains the complete instruction for setting up a build environment. If you don't want to install the required dependencies on your local machine, you can use this docker image to build and run 3dfier. If you want to build 3dfier locally, then please look into the Dockerfile for the libraries that you'll need.

## Using Docker to compile and run 3dfier

The builder image (`Dockerfile_builder`) does not contain the source code of 3dfier, neither the executable. We use this image for compiling 3dfier from different branches during development and testing. Nevertheless, it is suitable for 3dfying any data set.

As an example, the complete process of building the docker image, compiling 3dfier and running looks like the following.


**1. Build the docker image**

```
$ docker build -t 3dfier:builder -f Dockerfile_builder .
```

**2. Prepare a compiler script for 3dfier**

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

**3. Compile 3dfier in a container**

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

**4. Running 3dfier in a container**

Then the previously built executable is mounted to the container and executed there. Note that the data and config files are also mounted in a similar fashion.

```
$ docker run --rm -it -v "$(pwd)":/opt/3dfier_src 3dfier:builder /opt/3dfier_src/build/3dfier /opt/3dfier_src/3dfier/example_data/testarea_config_docker.yml --CSV-BUILDINGS /opt/3dfier_src/test.csv
```