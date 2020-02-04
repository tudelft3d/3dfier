## Continuous integration
| Build server | Platform | Build status |
| :---- | :------ | :---- |
| AppVeyor | Windows | [![AppVeyor build status][1]][2]
| Travis | Ubuntu | [![Travis build status][3]][4]

[1]: https://ci.appveyor.com/api/projects/status/github/tudelft3d/3dfier?branch=master&svg=true
[2]: https://ci.appveyor.com/project/tudelft3d/3dfier/branch/master
[3]: https://api.travis-ci.org/tudelft3d/3dfier.svg?branch=master
[4]: https://travis-ci.org/tudelft3d/3dfier

# Wiki

The [3dfier wiki](https://github.com/tudelft3d/3dfier/wiki) has extensive information on the installation, data preparation, usage and output of 3dfier.

**SETTINGS**

For all available setting read the documentation and for default values look at the [config files folder](https://github.com/tudelft3d/3dfier/tree/master/resources/config_files)

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
  1. Separation (used for concrete slabs used along canals for instance, surely very "Dutch")

It is possible to define new classes, although that would require a bit of programming.

Output is in the following formats: OBJ, CityGML, CityJSON, CSV (for buildings only, i.e. their ID and height (ground+roof) are output in a tabular format), and PostGIS.
The ID of each polygon is preserved, and there is a 1-to-1 mapping between the input and the output. 

If you use it, feedback is very much appreciated.

## LAS/LAZ Pointcloud

We expect the LAS/LAZ to be classified according to the ASPRS Standard LIDAR Point Classes v1.4 (Table 4.9 of this [PDF](http://www.asprs.org/wp-content/uploads/2010/12/LAS_1-4_R6.pdf)), and at a minimum these should be defined:

  - 0-1: Created, never classified and/or unclassified
  - 2: Ground
  - 3-5: Vegetation

If the vegetation is not classified or not filtered out, then buildings might be taller and there might be artefacts in the terrain.

## Test data
In the folder `example_data` (download [example dataset](https://github.com/tudelft3d/3dfier/tree/master/example_data)) there is a small part of the [BGT datasets](http://www.kadaster.nl/web/Themas/Registraties/BGT.htm) (2D 1:1k topographic datasets of the Netherlands), and a part of the [AHN3 LIDAR dataset](https://www.pdok.nl/nl/ahn3-downloads) that can be used for testing. 
The resulting model (in OBJ) can be found in `example_data/output/test_area.obj`

Further, there is an [open data website](https://3d.bk.tudelft.nl/opendata/3dfier/) that contains 3D models of a few Dutch cities, generated with 3dfier.

## Validate config file
The configuration is stored in [YAML format](http://docs.ansible.com/ansible/latest/YAMLSyntax.html) and needs to be valid for the parser to read the file. 
Config files can be schema validated using [YAML Lint](http://www.yamllint.com)

A configuration file with description of all possible settings is [myconfig_README.yml](https://github.com/tudelft3d/3dfier/blob/master/resources/config_files/myconfig_README.yml)

## Run 3dfier:
**Windows** 
Open a command line (click start and type `command` or `cmd`). Using the command line browse to the folder where you extracted the example files and run:
`3dfier myconfig.yml -o output.ext`

**Mac OS X and Linux**
Open a console. Using the console browse to the folder where you extracted the example files and run:
`$ ./3dfier myconfig.yml -o output.ext`

## Prepare BGT data
For preparing BGT data as input for 3dfier look at [Large scale topography to 3D example](http://tudelft3d.github.io/3dfier/bgt_example.html) in the documentation.
