---
title: Installation
keywords: 3dfier installation ubuntu docker windows compile
summary: These instructions will help you to install 3dfier on various operating systems. For Windows please use the binary files and do not compile from source.
sidebar: 3dfier_sidebar
permalink: installation.html
---

## Install on Windows using binaries
Ready-to-use binary releases exist for [Windows, macOS and Linux](https://github.com/{{site.repository}}/releases/latest). Download and extract the files to any given folder and follow the instructions in the [Get started guide]({{site.baseurl}}/index).

## Compiling from source

3dfier is a C++17 project built with CMake. Its dependencies are:

  1. [Boost](https://www.boost.org) (`program_options`, `filesystem`, `locale`, `chrono`)
  1. [CGAL](https://www.cgal.org)
  1. [GDAL](https://gdal.org/)
  1. [PDAL](https://pdal.io) (used to read LAS/LAZ point clouds)
  1. [yaml-cpp](https://github.com/jbeder/yaml-cpp)

### macOS

We suggest using [Homebrew](http://brew.sh/) to install all dependencies:

    $ brew install boost cgal gdal pdal yaml-cpp cmake

To compile 3dfier:

    $ mkdir build
    $ cd build
    $ cmake .. -DCMAKE_BUILD_TYPE=Release
    $ make
    $ make install

### Ubuntu / Debian

On Ubuntu 22.04 (jammy) all dependencies are available from the standard repositories:

```
sudo apt-get update
sudo apt-get install -y cmake g++ make \
  libboost-program-options-dev libboost-filesystem-dev \
  libboost-locale-dev libboost-chrono-dev \
  libcgal-dev libgdal-dev libpdal-dev libyaml-cpp-dev
```

Then compile 3dfier:

```
git clone https://github.com/tudelft3d/3dfier.git
cd 3dfier; mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

Note: newer Ubuntu (24.04) and Debian (12+) releases no longer ship the `pdal` package. On those systems install PDAL from [another source](https://pdal.io/en/latest/development/building.html) (e.g. a build from source, `vcpkg`, or `conda`) or use the [Docker image](#docker).

### Windows

The recommended way to compile 3dfier on Windows is with [vcpkg](https://vcpkg.io). The dependencies are described in the `vcpkg.json` manifest file in the repository root.

```
git clone https://github.com/tudelft3d/3dfier.git
cd 3dfier
vcpkg install
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Nix

A [Nix flake](https://nixos.wiki/wiki/Flakes) is provided in the repository root. It builds 3dfier on Linux and macOS and provides a development shell:

```
nix build
nix develop
```

## Docker
We offer built docker images from the `master`, `development` branches and each release. You'll find the images and instructions on using them at [Docker Hub](https://hub.docker.com/r/tudelft3d/3dfier).
