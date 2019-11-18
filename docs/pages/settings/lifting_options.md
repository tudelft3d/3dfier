---
title: Lifting options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: lifting_options.html
---

In the lifting_options we define the settings used for lifting polygons of the specified class. In total there are 7 different possibilities to choose from;
- Building
- Terrain
- Forest
- Water
- Road
- Separation
- Bridge/Overpass

If a class is not defined the default values will be used. Default values can be found in [resources/config_files/myconfig_DEFAULTS.yml](https://github.com/{{site.repository}}/raw/master/resources/config_files/myconfig_DEFAULTS.yml).

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

#### roof and ground settings
Settings within the roof section only apply to the calculation of the roof height where settings in the ground section only applies to the calculation of the floor height
.
#### height: percentile-xx

#### use_LAS_classes

#### use_LAS_classes_within

#### lod: 0
*Download [YAML]({{site.baseurl}}/assets/configs/lod0.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_lod0.png" alt="" %}
{% include imagezoom.html file="/settings/lod0.png" alt="" %}

#### lod: 1
*Download [YAML]({{site.baseurl}}/assets/configs/lod1.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_lod1.png" alt="" %}
{% include imagezoom.html file="/settings/lod1.png" alt="" %}

#### floor: true
*Download [YAML]({{site.baseurl}}/assets/configs/floor_true.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_floor_true.png" alt="" %}
{% include imagezoom.html file="/settings/floor_true.png" alt="" %}

#### floor: false
*Download [YAML]({{site.baseurl}}/assets/configs/floor_false.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_floor_false.png" alt="" %}
{% include imagezoom.html file="/settings/floor_false.png" alt="" %}

#### inner_walls: true
*Download [YAML]({{site.baseurl}}/assets/configs/inner_walls_true.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_inner_walls_true.png" alt="" %}
{% include imagezoom.html file="/settings/inner_walls_true.png" alt="" %}

#### inner_walls: false
*Download [YAML]({{site.baseurl}}/assets/configs/inner_walls_false.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_inner_walls_false.png" alt="" %}
{% include imagezoom.html file="/settings/inner_walls_false.png" alt="" %}


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
filter_outliers: true      # Filter outliers by iterative Least Squares fitting of 3D quadric surface. Only replace heights of detected outliers
flatten: true              # Filter outliers by iterative Least Squares fitting of 3D quadric surface. Replace all heights of polygon with the fitted plane. Results in smoother roads
use_LAS_classes:           # LAS classes to be used for this class. If empty all classes are used
  - 3
  - 6
~~~

#### filter_outliers: true
#### filter_outliers: false
#### flatten: true
#### flatten: false

### Separation
This class describes objects inpired by objects contained in the Dutch BGT. These objects generally describe walls or wall-like objects with a flat top.

~~~ yaml
height: percentile-80
~~~

### Bridge/Overpass
~~~ yaml
height: percentile-50
flatten: true              # Filter outliers by iterative Least Squares fitting of 3D quadric surface. Replace all heights of polygon with the fitted plane. Results in smoother bridges
~~~

### Terrain
~~~ yaml
simplification: 100          # Simplification factor for points added within terrain polygons, points are added random
simplification_tinsimp: 0.1  # Simplification threshold for points added within terrain polygons, points are removed from triangulation until specified error threshold value is reached
innerbuffer: 1.0             # Inner buffer in meters where no additional points will be added within boundary of the terrain polygon
~~~

#### simplification
Random filtering

**Don't use simplification and simplification_tinsimp at the same time!**

#### simplification_tinsimp
*Download [YAML]({{site.baseurl}}/assets/configs/tinsimp_0.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_tinsimp_0.png" alt="" %}
{% include imagezoom.html file="/settings/tinsimp_0.png" alt="" %}

*Download [YAML]({{site.baseurl}}/assets/configs/tinsimp_01.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_tinsimp_01.png" alt="" %}
{% include imagezoom.html file="/settings/tinsimp_01.png" alt="" %}

*Download [YAML]({{site.baseurl}}/assets/configs/tinsimp_05.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_tinsimp_05.png" alt="" %}
{% include imagezoom.html file="/settings/tinsimp_05.png" alt="" %}

#### innerbuffer
*Download [YAML]({{site.baseurl}}/assets/configs/innerbuffer_05.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_innerbuffer_05.png" alt="" %}
{% include imagezoom.html file="/settings/innerbuffer_05.png" alt="" %}

*Download [YAML]({{site.baseurl}}/assets/configs/innerbuffer_1.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_innerbuffer_1.png" alt="" %}
{% include imagezoom.html file="/settings/innerbuffer_1.png" alt="" %}

*Download [YAML]({{site.baseurl}}/assets/configs/innerbuffer_3.yml) and [OBJ]()*
{% include imagezoom.html file="/settings/settings_innerbuffer_3.png" alt="" %}
{% include imagezoom.html file="/settings/innerbuffer_3.png" alt="" %}

### Forest
~~~ yaml
simplification: 100          # Simplification factor for points added within forest polygons, points are added random
simplification_tinsimp: 0.1  # Simplification threshold for points added within forest polygons, points are removed from triangulation until specified error threshold value is reached
innerbuffer: 1.0             # Inner buffer in meters where no additional points will be added within boundary of the forest polygon
~~~
