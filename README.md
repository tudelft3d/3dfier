## Continuous integration
| Build server | Platform | Build status |
| :---- | :------ | :---- |
| AppVeyor | Windows | [![AppVeyor build status][1]][2]
| Travis | Ubuntu | [![Travis build status][3]][4]

[1]: https://ci.appveyor.com/api/projects/status/github/tudelft3d/3dfier?branch=master&svg=true
[2]: https://ci.appveyor.com/project/tudelft3d/3dfier/branch/master
[3]: https://api.travis-ci.org/tudelft3d/3dfier.svg?branch=master
[4]: https://travis-ci.org/tudelft3d/3dfier

# Documentation
The [3dfier documentation](http://tudelft3d.github.io/3dfier) has extensive information on the installation, usage and how the 3dfier algorithm works.

# 3dfier
<img src="https://dl.dropboxusercontent.com/s/05eo2r5yc2kke5g/3dfierNoBridge.png" width="300">

Takes 2D GIS datasets (e.g. topographical datasets) and "3dfies" them (as in "making them three-dimensional") by lifting every polygon to 3D.
The elevation is obtained from a point cloud (we support LAS/LAZ at this moment), and the semantics of every polygon is used to perform the lifting.
That is, water polygons are extruded to horizontal polygons, buildings to LOD1 blocks, roads as smooth surfaces, etc.
Every polygon is triangulated (constrained Delaunay triangulation) and the lifted polygons are "stitched" together so that one digital surface model (DSM) is constructed.
Our aim is to obtain one DSM that is error-free, i.e. no intersecting triangles, no holes (the surface is watertight), where buildings are integrated in the surface, etc.
This surface will then be used as input in simulation software for instance.

![](https://dl.dropboxusercontent.com/s/tojiay8cmomu2v5/Delft_3dfier-3.png)

<a href="https://vimeo.com/181421237">This video</a> illustrates the process and what 3dfier is about.

The lifting options can be configured in the YAML file provided, an examples are provided in `/resources/config_files/myconfig.yml`.
Any 2D input (which should be a planar partition) can be used as input, and each class must be mapped to one of the following:

  1. Building
  1. Terrain
  1. Road
  1. Water
  1. Forest
  1. Bridge
  1. Separation (used for walls and fences)

It is possible to define new classes, although that would require a bit of programming.

Output is in the following formats: OBJ, CityGML, CityJSON, CSV (for buildings only, i.e. their ID and height (ground+roof) are output in a tabular format), PostGIS, and STL.
The ID of each polygon is preserved, and there is a 1-to-1 mapping between the input and the output. 

If you use it, feedback is very much appreciated.

## LAS/LAZ Pointcloud

We expect the LAS/LAZ to be classified according to the ASPRS Standard LIDAR Point Classes v1.4 (Table 4.9 of this [PDF](http://www.asprs.org/wp-content/uploads/2010/12/LAS_1-4_R6.pdf)), and at a minimum these should be defined:

  - 0-1: Created, never classified and/or unclassified
  - 2: Ground
  - 3-5: Vegetation

If the vegetation is not classified or not filtered out, then buildings might be taller and there might be artefacts in the terrain.

## Binary releases for Windows and Mac OS X

In order to make easy use of 3dfier we created pre-build binaries which can be downloaded from the [releases page](https://github.com/tudelft3d/3dfier/releases). 

Download the latest release and unzip the archive in a easy to find location, not in the download folder of your browser. 

To be able to quickly test 3dfier one can download the [example dataset](https://github.com/tudelft3d/3dfier/releases/tag/example_data) and unzip the archive in the folder of 3dfier. 

## Test data

In the folder `example_data` (download [example dataset](https://github.com/tudelft3d/3dfier/releases/tag/example_data)) there is a small part of the [BGT datasets](http://www.kadaster.nl/web/Themas/Registraties/BGT.htm) (2D 1:1k topographic datasets of the Netherlands), and a part of the [AHN3 LIDAR dataset](https://www.pdok.nl/nl/ahn3-downloads) that can be used for testing. 
The resulting model (in OBJ) can be found in `example_data/output/test_area.obj`

Further, there is an [open data website](https://3d.bk.tudelft.nl/opendata/3dfier/) that contains 3D models of a few Dutch cities, generated with 3dfier.

## Validate config file
The configuration is stored in [YAML format](http://docs.ansible.com/ansible/latest/YAMLSyntax.html) and needs to be valid for the parser to read the file. 
Config files can be schema validated using [YAML Lint](http://www.yamllint.com)

## Run 3dfier:
**Windows** 
Open a command line (click start and type `command` or `cmd`). Using the command line browse to the folder where you extracted the example files and run:
`3dfier myconfig.yml -o output.ext`

**Mac OS X and Linux**
Open a console. Using the console browse to the folder where you extracted the example files and run:
`$ ./3dfier myconfig.yml -o output.ext`

**Docker**

3dfier offers a alpine base image which tries to give you as much freedom for your vector data source as possible. Vector data is read by GDAL/OGR.

To run 3dfier over Docker simply execute:

    $ docker run --rm --name 3dfier -v <local path where your files are>:/data tudelft3d/3dfier:<tag> 3dfier <name of config file> <... 3dfier parameters>

All your input data needs to be in `<local path where your files are>` and in the config file you need to reference your input data relative to `<local path where your files are>`. To achieve this either move your data and config into `<local path where your files are>` (and subdirectories), or set `<local path where your files are>` to the lowest common ancestor that contains all the data and config files you need.

**Keep in mind that `<local path where your files are>` need to be writable by any user, otherwise your output won't be saved.**

For instance to run it on the example data set (on Linux):

    $ cd 3dfier/example_data
    $ docker run --rm -it -v 3dfier/example_data:/data tudelft3d/3dfier:latest 3dfier testarea_config_unix.yml --OBJ test.obj

There is also a [tutorial](https://github.com/tudelft3d/3dfier/wiki/General-3dfier-tutorial-to-generate-LOD1-models) on how to generate a 3D model with 3dfier.

## Prepare BGT data
For preparing BGT data as input for 3dfier look at [resources/BGT_prepare/ReadMe.md](https://github.com/tudelft3d/3dfier/blob/master/resources/BGT_prepare/ReadMe.md)

## Compiling Mac OS X/Linux

To build you'll normally do (from 3dfier root directory):

```
mkdir build && cd build
cmake .. 
cmake ..
make
```

Notice that cmake is indeed called *twice*, we have noticed that on some machines the compiler optimisation is activated only when you cmake twice.
Why that is we are not sure, but to be sure do it twice.
With the optimisation, the test dataset should take around 20s to produce; if more (>5min) then the optimisation is not activated properly.

The dependencies that are necessary (under Mac we suggest using [Homebrew](http://brew.sh)):

  1. LIBLAS *with* LASzip support (`brew install liblas --with-laszip`)
  1. GDAL (`brew install gdal`)
  1. Boost (`brew install boost`)
  1. CGAL (`brew install cgal`)
  1. yaml-cpp (`brew install yaml-cpp`)

For Linux we suggest taking a look at the travis build scripts for Ubuntu.

## Compiling Windows using Visual Studio
Detailed instructions can be found on our [wiki](https://github.com/tudelft3d/3dfier/wiki/Building-on-Windows-10). Short version:

1. Download and install [Boost precompiled binaries](https://sourceforge.net/projects/boost/files/boost-binaries)
1. Download and install [OSGeo4W](https://trac.osgeo.org/osgeo4w)
1. Compile your own copies of [Yaml-cpp](https://github.com/jbeder/yaml-cpp) and [CGAL](https://www.cgal.org)
1. Add environment variables for:
1. - `OSGEO4W_ROOT` (set by OSGeo4W installer)
    - `BOOST_ROOT` (root of the boost directory)
    - `BOOST_LIBRARYDIR` (dir of the boost lib files)
    - `LIBLAS_ROOT` (same as OSGEO4W_ROOT if that liblas is used)
    - `LASZIP_ROOT` (same as OSGEO4W_ROOT if that liblas is used)
    - `YAML-CPP_DIR` (root of the Yaml-cpp directory)
    - `CGAL_DIR` (root of the CGAL directory)
1. Build solution using Visual Studio
