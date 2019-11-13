---
title: Getting started with 3dfier
keywords: 3dfier homepage
summary: These instructions will help you to get started with 3dfier. Other sections will provide additional information on the use of the software.
sidebar: 3dfier_sidebar
permalink: index.html
---
# 3dfier: The open-source tool for creation of 3D models
* * *

3dfier tries to fill the gap for simply creating 3D models. It takes 2D GIS datasets (e.g. topographical datasets) and "3dfies" them (as in "making them three-dimensional"). The elevation is obtained from a point cloud (we support LAS/LAZ at the moment), and the semantics of every polygon is used to perform the lifting. After lifting, elevation gaps between the polygons are removed by "stitching" together the polygons based on rules so that a watertight digital surface model (DSM) fused with 3D objects is constructed. A rule based stucture is used to extrude water as horizontal polygons, create LOD1 blocks for buildings, smooth road surfaces and construct bridges (3D polygonal surface). This software is developped by the [3D Geoinformation group](https://3d.bk.tudelft.nl) at **Delft University of Technology**.

Our aim is to obtain one model that is error-free, so no intersecting objects, no holes (the surface is watertight) and buildings are integrated in the surface.

{% include imagezoom.html file="Delft_3dfier-3.png" alt="3dfier result of Delft" %}

## Your first time running
3dfier is a command-line utility that does not have a graphical user interface (GUI). This means you need to run it from [Command Prompt](https://www.lifewire.com/how-to-open-command-prompt-2618089).

### Verify installation
First thing to do is to test if the [Installation]({{site.baseurl}}/installation) was successful.
You can do this by opening the Command Prompt (press windows button+R, type cmd and press enter) and dragging the 3dfier program into the Command Prompt window. If you now press enter the 3dfier program should output:

```
3dfier Copyright (C) 2015-2019  3D geoinformation research group, TU Delft
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; for details run 3dfier with the '--license' option.

ERROR: one YAML config file must be specified.

Allowed options:
  --help                        View all options
  --version                     View version
  --license                     View license
  --OBJ arg                     Output
  --OBJ-NoID arg                Output
  --CityGML arg                 Output
  --CityGML-Multifile arg       Output
  --CityGML-IMGeo arg           Output
  --CityGML-IMGeo-Multifile arg Output
  --CityJSON arg                Output
  --CSV-BUILDINGS arg           Output
  --CSV-BUILDINGS-MULTIPLE arg  Output
  --CSV-BUILDINGS-ALL-Z arg     Output
  --Shapefile arg               Output
  --Shapefile-Multifile arg     Output
  --PostGIS arg                 Output
  --PostGIS-PDOK arg            Output
  --PostGIS-PDOK-CityGML arg    Output
  --GDAL arg                    Output
```

### Prepare example data
For this example we use [BGT_Delft_Example.zip]({{site.github.repository_url}}/raw/master/resources/Example_data/BGT_Delft_Example.zip) from the GitHub repository located in `3dfier/resources/Example_data/`. Create a folder with 3dfier and the depencency dll's and add the `example_data folder`.

{% include imagezoom.html file="example_folder.png" caption="Folder layout" alt="Folder layout" %}

### Running with the example data
Opening the Command Prompt (press windows button+R, type cmd and press enter) and navigate to the folder where 3dfier is located.

Now go into the example data folder using `cd example_data` and run `..\3dfier.exe testarea_config.yml --OBJ output\myfirstmodel.obj`. Now 3dfier will start processing and when finished it produced its first 3D model!

The output file can be found here `example_data\output\testarea.obj` and console output should be as follows.

```
3dfier Copyright (C) 2015-2019  3D geoinformation research group, TU Delft
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; for details run 3dfier with the '--license' option.

Config file is valid.
Reading input dataset: bgt\bgt_waterdeel.sqlite
        Layer: waterdeel
        (3 features --> Water)
Reading input dataset: bgt\bgt_ondersteunendwaterdeel.sqlite
        Layer: ondersteunendwaterdeel
        (0 features --> Water)
Reading input dataset: bgt\bgt_onbegroeidterreindeel.sqlite
        Layer: onbegroeidterreindeel
        (81 features --> Terrain)
Reading input dataset: bgt\bgt_wegdeel.sqlite
        Layer: trafficarea
        (151 features --> Road)
Reading input dataset: bgt\bgt_ondersteunendwegdeel.sqlite
        Layer: auxiliarytrafficarea
        (0 features --> Road)
Reading input dataset: bgt\bgt_pand.sqlite
        Layer: buildingpart
        (160 features --> Building)
Reading input dataset: bgt\bgt_begroeidterreindeel.sqlite
        Layer: plantcover
        (126 features --> Forest)
Reading input dataset: bgt\bgt_scheiding.sqlite
        Layer: scheiding
        (57 features --> Separation)
Reading input dataset: bgt\bgt_kunstwerkdeel.sqlite
        Layer: kunstwerkdeel
        (1 features --> Separation)
Reading input dataset: bgt\bgt_overigbouwwerk.sqlite
        Layer: overigbouwwerk
        (0 features --> Separation)
Reading input dataset: bgt\bgt_overbruggingsdeel.sqlite
        Layer: bridgeconstructionelement
        (3 features --> Bridge/Overpass)

Total # of polygons: 570
Constructing the R-tree... done.
Spatial extent: (84,616.468, 447,422.999) (85,140.839, 447,750.636)
Reading LAS/LAZ file: ahn3\ahn3_cropped_1.laz
        (640,510 points in the file)
        (all points used, no skipping)
        (omitting LAS classes: 0 1 )
[==================================================] 100%
Reading LAS/LAZ file: ahn3\ahn3_cropped_2.laz
        (208,432 points in the file)
        (all points used, no skipping)
        (omitting LAS classes: 0 1 )
[==================================================] 100%
All points read in 1 seconds || 00:00:01
3dfying all input polygons...
===== /LIFTING =====
===== LIFTING/ =====
=====  /ADJACENT FEATURES =====
=====  ADJACENT FEATURES/ =====
=====  /STITCHING =====
=====  STITCHING/ =====
=====  /BOWTIES =====
=====  BOWTIES/ =====
=====  /VERTICAL WALLS =====
=====  VERTICAL WALLS/ =====
Lifting, stitching and vertical walls done in 0 seconds || 00:00:00
=====  /CDT =====
=====  CDT/ =====
CDT created in 0 seconds || 00:00:00
...3dfying done.
OBJ output: output\myfirstmodel.obj
Features written in 0 seconds || 00:00:00
Successfully terminated in 2 seconds || 00:00:02
```

## Data that is generated
Only 3D data is generated for 2D polygons supplied in the input. If no polygon is input for terrain it is not (automagically) generated. There is no option (yet) for the creation of terrain within the polygons extent or point file extent. A tutorial is written to create a terrain polygon with building footprints cut out. Please follow the [Generate terrain polygon tutorial]({{site.baseurl}}/generate_terrain)

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


## Known issues
### Missing DLL's
If you see an error message about missing MSCVRxxx.dll instead of this text you may need to do one of the following:

* MSCVR100.dll is missing: Install the [Microsoft Visual C++ 2010 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=14632)
* MSCVR120.dll is missing: Install the [Microsoft Visual C++ 2013 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=40784)
* MSCVR140.dll is missing: Install the [Microsoft Visual C++ 2015 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=48145)
