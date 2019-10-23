---
title: Contents of a release
keywords: release zip source code
sidebar: 3dfier_sidebar
permalink: release_contents.html
---

## Release binaries content
Description of files in the zip file

All dll files distributed with 3dfier belong to GDAL or other packages used in the GDAL drivers.

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

## Resources
Description of the contents of the resources repository

Name | Folder | Description
-----|--------|-------------
3dfier.mtl | | Material description file for coloring an OBJ file
3dfier2sim | x | Source code for program to convert 3dfier OBJ output to OFF file with a volume, see README.md
BGT_prepare | x | Script and GFS files to stroke arcs and convert BGT GML to GeoPackage, see ReadMe.md
BGT_prepare\BGT_gfs_files | x | GFS files describing the content of different BGT GML files
build_ubuntu | x | Scripts to build 3dfier program and dependencies for Ubuntu
build_ubuntu1604.sh | | Script to build 3dfier program and dependencies for Ubuntu 16.04
config_files | x | Example config files
config_files\myconfig.yml | | Example config file
config_files\myconfig_DEFAULTS.yml | | Config file describing programmed defaults
config_files\myconfig_README.yml | | Config file with complete description of all settings options
create_vegetation_off.py | | **TODO: ASK HUGO**
Example_data | x | Zip file with example input data and configuration for running 3dfier
flowdiagrams | x | Flow diagrams of 3dfier used within the documentation
obj2off.py | | Script to convert a OBJ to OFF file format
splitobj | x | Script for splitting a single OBJ file into several different OBJ files by class
splitobj_cpp | x | Program for splitting a single OBJ file into several different OBJ files by class
translate2min-obj.py | | Script for shifting origin of OBJ to 0,0