---
title: Input options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: input_options.html
---

In the input_options we define the settings used for reading the input files for both polygons and point clouds.

If a class is not defined the default values will be used. Default values can be found in [resources/config_files/myconfig_DEFAULTS.yml](https://github.com/{{site.repository}}/raw/master/resources/config_files/myconfig_DEFAULTS.yml).

## input_polygons
### datasets
1. Flat config
~~~ yaml
- datasets:
  - D:\data\campus\partof.shp
	uniqueid: FACE_ID
	lifting: Building
~~~

2. Nested config
~~~ yaml
- datasets:
  - D:\data\campus\polygons.gml
	uniqueid: fid
	lifting_per_layer:
		Buildingfootprint: Building
		Terrainobjects: Terrain
		Waterways: Water
~~~

The gml file consists of three layers:
- Buildingfootprint
- Terrainobjects
- Waterways

### uniqueid
This is the name of the attribute in the input file that contains a unique identifier per feature. In case nothing is put here it uses fid by default. **CHECK THIS**
When not using a unique identifier many of the output formats will be invalid according to their standard.

### lifting
This is a class name corresponding to the list of the lifting options defined in [Lifting options]({{site.baseurl}}/lifting_options). In total there are 7 different possibilities to choose from;
- Building
- Terrain
- Forest
- Water
- Road
- Separation
- Bridge/Overpass

{% include imagezoom.html file="/settings/settings_input_polygon_class_lifting_class.png" alt="" %}

### height_field
This is the attribute name of an integer attribute. This integer attribute describes the different relative height levels of polygons overlapping in 3D. This describes the vertical relationship between the objects. In Dutch BGT all objects with height_field = 0 describe the ground level. All objects above are counted in positive direction (a bridge would have level 1) and objects below ground level are counted in negative direction (a tunnel would have level -1). [More information on how this is applied in BGT](http://imgeo.geostandaarden.nl/def/imgeo-object/overbruggingsdeel/inwinningsregel-imgeo/toelichting-relatieve-hoogte)

### handle_multiple_heights
When the input polygons use the [height_field](#height_field) this boolean setting describes if these multiple vertical overlapping objects need to be used (height_field attribute with any value) or to just use the ground level polygons (height_field attribute with value of 0).

## input_elevation
### datasets
1. Single file
~~~ yaml
- datasets:                         # List of data set with specific settings
  - D:\data\top10nl\schie\ahn3.laz  # Definition for one or multiple LAS/LAZ files using the same parameters
~~~
2. Directory
~~~ yaml
- datasets:                         # List of data set with specific settings
  - D:\data\top10nl\schie\*         # Definition for reading a full directory with LAS/LAZ files using the same parameters
~~~

### omit_LAS_classes
~~~ yaml
omit_LAS_classes:     # Option to omit classes defined in the LAS/LAZ files
  - 1 # unclassified  # ASPRS Standard Lidar Point Classes classification value
  - 3 # vegetation
  - 4 # vegetation
  - 5 # vegetation
~~~

### thinning
~~~ yaml
thinning: 10          # Thinning factor for points, this is the amount of points skipped during read, a value of 10 would result in points 1, 11, 21, 31 being used
~~~