# 3dfier

To run:

`$ ./3dfier myconfig.yml > output.gml`

To test it with a very small area around the TUD campus (8 polygons + AHN2 cropped + config file):

  - https://www.dropbox.com/sh/wdzhixnqqucuc6p/AAAN1ebHZGP374VAr_Zv3BwBa?dl=0

With a (pre-processed part of top10nl in GML):

  - https://www.dropbox.com/s/9b49oamhyujlqlk/TOP10NL_37W_00.gml?dl=0
  - http://geodata.nationaalgeoregister.nl/ahn2/extract/ahn2_uitgefilterd/u37bn2.laz.zip
  - https://www.dropbox.com/s/695nyjhsjl2acgm/top10.yml?dl=0


Dependencies:

  1. LIBLAS *with* LASzip support (`brew install liblas --with-laszip`)
  2. GDAL (`brew install gdal`)
  3. Boost (`brew install boost`)
  4. yaml-cpp (`brew install yaml-cpp`)

The plan is to:

  - triangulate each polygon with [Shewchuk's Triangle](http://www.cs.cmu.edu/%7Equake/triangle.html) 
  - use [ANN](http://www.cs.umd.edu/~mount/ANN/) to index *locally* the points when we're assigning a height to the ones on the boundary for instance.

  