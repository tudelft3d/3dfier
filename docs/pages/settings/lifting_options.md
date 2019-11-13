---
title: Lifting options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: lifting_options.html
---

### Building
~~~ yaml
roof:
 height: percentile-90     # Percentile of points within radius of building vertices for roof height lifting, building_radius_vertex_elevation is defined in options
 use_LAS_classes:          # LAS classes to be used for this class. If empty all classes are used
   - 6
ground:
 height: percentile-10     # Percentile of points within radius of building vertices for floor height lifting
 use_LAS_classes:
   - 2
   - 9
lod: 1                      # Define the Level Of Detail for Buildings (0 and 1 possible)
floor: true                 # Set if floors should be created
inner_walls: true           # Set if the walls between to adjacent buildings within a block should be created
triangulate: false          # Set if the output should be triangulated, only works for non-triangular output formats like CityGML/IMGeo
~~~
### Water
~~~ yaml
height: percentile-10      # Percentile of points within radius of water vertices for lifting, radius_vertex_elevation is defined in options
use_LAS_classes_within:    # LAS classes to be used for this class, but only if points fall within the polygon and the range of the vertex.
  - 2
  - 9
~~~
### Road
~~~ yaml
height: percentile-50
filter_outliers: true      # Filter outliers by iterative Least Squares fitting of 3D quadric suface. Only replace heights of detected outliers
flatten: true              # Filter outliers by iterative Least Squares fitting of 3D quadric suface. Replace all heights of polygon with the fitted plane. Results in smoother roads
use_LAS_classes:           # LAS classes to be used for this class. If empty all classes are used
  - 3
  - 6
~~~
### Separation
~~~ yaml
height: percentile-80
~~~
### Bridge/Overpass
~~~ yaml
height: percentile-50
flatten: true              # Filter outliers by iterative Least Squares fitting of 3D quadric suface. Replace all heights of polygon with the fitted plane. Results in smoother bridges
~~~
### Terrain
~~~ yaml
simplification: 100          # Simplification factor for points added within terrain polygons, points are added random
simplification_tinsimp: 0.1  # Simplification threshold for points added within terrain polygons, points are removed from triangulation until specified error threshold value is reached
innerbuffer: 1.0             # Inner buffer in meters where no additional points will be added within boundary of the terrain polygon
~~~
### Forest
~~~ yaml
simplification: 100          # Simplification factor for points added within forest polygons, points are added random
simplification_tinsimp: 0.1  # Simplification threshold for points added within forest polygons, points are removed from triangulation until specified error threshold value is reached
innerbuffer: 1.0             # Inner buffer in meters where no additional points will be added within boundary of the forest polygon
~~~