# 3dfier

Takes 2D GIS datasets (eg topographical datasets) and "3dfy" them by lifting every polygon to 3D.
The elevation is obtained from a point cloud (we support LAS/LAZ at this moment), and the semantics of every polygon is used to perform the lifting.
That is, water polygons are extruded to horizontal polygons, buildings to LOD1, roads are smoothed, etc.
Every polygon is triangulated (constrained Delaunay triangulation) and all the lifted polygons are "stitched" so that one DSM is constructed.

The lifting can be configured in the YAML file (`myconfig.yml`) provided.

Output is at this moment in either OBJ or CityGML (and CVS for buildings only, ie their ID and height (ground+roof) are output).
The ID of each polygon is preserved, and there is a 1-to-1 mapping between the input and the output. 

Observe that this version is *early alpha*, and shouldn't really be used for anything except for testing.

## Compiling

To build you'll normally do (from 3dfier root directory):

```
mkdir build && cd build
cmake ..
make
```

And on ubuntu systems that come with GDAL 2, make sure the `libgdal1-dev` package is installed and replace `cmake ..` with 
```
cmake .. -DGDAL_CONFIG=/bin/gdal-config -DGDAL_LIBRARY=/usr/lib/libgdal.so -DGDAL_INCLUDE_DIR=/usr/include/gdal
```

To run:

`$ ./3dfier myconfig.yml > output.obj`

## Dependencies:

Under Mac OSX they can all be installed with [Homebrew](http://brew.sh).

  1. LIBLAS *with* LASzip support (`brew install liblas --with-laszip`)
  2. GDAL (`brew install gdal`)
  3. Boost (`brew install boost`)
  4. yaml-cpp (`brew install yaml-cpp`)
  5. [Shewchuk's Triangle](http://www.cs.cmu.edu/%7Equake/triangle.html) (`brew install homebrew/science/triangle`)

## Data

TOP10NL download

  - https://www.pdok.nl/nl/producten/pdok-downloads/basis-registratie-topografie/topnl/topnl-actueel/top10nl

OGR will not read it properly, it needs to be preprocessed with [nlextract](https://github.com/opengeogroep/NLExtract/tree/top10_v1_2/top10nl/etl) (Below the part was pre-processed with it).





  
