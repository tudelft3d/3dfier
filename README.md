## Continuous integration
| Build server | Platform | Build status |
| :---- | :------ | :---- |
| AppVeyor | Windows | [![AppVeyor build status][1]][2]
| Travis | Ubuntu | [![Travis build status][3]][4]
[1]: https://ci.appveyor.com/api/projects/status/github/tudelft3d/3dfier?branch=master&svg=true
[2]: https://ci.appveyor.com/project/tudelft3d/3dfier/branch/master
[3]: https://api.travis-ci.org/tudelft3d/3dfier.svg?branch=master
[4]: https://travis-ci.org/tudelft3d/3dfier


# 3dfier
![](https://dl.dropboxusercontent.com/s/tojiay8cmomu2v5/Delft_3dfier-3.png)


Takes 2D GIS datasets (e.g. topographical datasets) and "3dfies" them (as in "making them three-dimensional") by lifting every polygon to 3D.
The elevation is obtained from a point cloud (we support LAS/LAZ at this moment), and the semantics of every polygon is used to perform the lifting.
That is, water polygons are extruded to horizontal polygons, buildings to LOD1 blocks, roads as smooth surfaces, etc.
Every polygon is triangulated (constrained Delaunay triangulation) and the lifted polygons are "stitched" together so that one digital surface model (DSM) is constructed.
Our aim is to obtain one DSM that is error-free, i.e. no intersecting triangles, no holes (the surface is watertight), where buildings are integrated in the surface, etc.
This surface will then be used as input in simulation software for instance.

<a href="https://vimeo.com/181421237">This video</a> illustrates the process and what 3dfier is about.

The lifting options can be configured in the YAML file (`myconfig.yml`) provided.
Any 2D input (which should be a planar partition) can be used as input, and each class must be mapped to one of the following:

  1. Building
  1. Terrain
  1. Road
  1. Water
  1. Forest
  1. Bridge
  1. Separation (used for concrete slabs used along canals for instance, surely very "Dutch")

It is possible to define new classes, although that would require a bit of programming.

Output is at this moment in either OBJ or CityGML (and CSV for buildings only, i.e. their ID and height (ground+roof) are output in a tabular format).
The ID of each polygon is preserved, and there is a 1-to-1 mapping between the input and the output. 

Notice that this version is very much a beta version, although it is in our opinion usable. 
If you use it, feedback is very much appreciated.

## LAS/LAZ Pointcloud

We expect the LAS/LAZ to be classified according to the ASPRS Standard LIDAR Point Classes v1.4 (Table 4.9 of this [PDF](http://www.asprs.org/wp-content/uploads/2010/12/LAS_1-4_R6.pdf)), and at a minimum these should be defined:

  - 0-1: Created, never classified and/or unclassified
  - 2: Ground
  - 3-5: Vegetation

If the vegetation is not classified or not filtered out, then buildings might be taller and there might be artefacts in the terrain.

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
  4. yaml-cpp (`brew install yaml-cpp`)

## Compiling Windows 10 using Visual Studio

1. Download and install Boost precompiled binaries https://sourceforge.net/projects/boost/files/boost-binaries (`Visual Studio 2015, 64-bit; boost_1_62_0-msvc-14.0-64.exe`)
2. Download and install OSGeo4W64 https://trac.osgeo.org/osgeo4w/
3. Compile your own copies of Yaml-cpp and CGAL
4. Set the path to the include folders in `Project->C/C++->General->Additional Include Directories`
5. Set the path to the boost binaries in `Project->Linker->General->Additional Library Directories`
6. Set the path to other libraries in `Project->Linker->Input->Additional Dependencies`
7. Build solution

## To run:

`$ ./3dfier myconfig.yml -o output.ext`

There is also a [tutorial](https://github.com/tudelft3d/3dfier/wiki/General-3dfier-tutorial-to-generate-LOD1-models) on how to generate a 3D model with 3dfier.

## Test data

In the folder `example_data` there is a small part of the [BGT datasets](http://www.kadaster.nl/web/Themas/Registraties/BGT.htm) (2D 1:1k topographic datasets of the Netherlands), and a part of the [AHN3 LIDAR dataset](https://www.pdok.nl/nl/ahn3-downloads) that can be used for testing. 
The resulting model (in OBJ) can be found in `example_data/output/test_area.obj`

Further, there is an [open data website](https://3d.bk.tudelft.nl/opendata/3dfier/) that contains 3D models of a few Dutch cities, generated with 3dfier.

## Prepare BGT data
For preparing BGT data as input for 3dfier look at [ressources/BGT_prepare/ReadMe.md](https://github.com/tudelft3d/3dfier/blob/master/ressources/BGT_prepare/ReadMe.md)
