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
  1. Separation (used for concrete slabs used along canals for instance, surely very "Dutch")

It is possible to define new classes, although that would require a bit of programming.

Output is in the following formats: OBJ, CityGML, CityJSON, CSV (for buildings only, i.e. their ID and height (ground+roof) are output in a tabular format), and PostGIS.
The ID of each polygon is preserved, and there is a 1-to-1 mapping between the input and the output. 

If you use it, feedback is very much appreciated.
