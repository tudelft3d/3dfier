# 3dfier

dependencies:

  1. LIBLAS *with* LASzip support (`brew install liblas --with-laszip`)
  2. GDAL (`brew install gdal`)
  3. Boost (`brew install boost`)

The plan is to:

  - triangulate each polygon with [Shewchuk's Triangle](http://www.cs.cmu.edu/%7Equake/triangle.html) 
  - use [ANN](http://www.cs.umd.edu/~mount/ANN/) to index *locally* the points when we're assigning a height to the ones on the boundary for instance.

  