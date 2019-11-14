---
title: Input options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: input_options.html
---

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
This is the name of the lifting options defined in [Lifting options]({{site.baseurl}}/lifting_options)
### height_field
Geometry relationship in height

### handle_multiple_heights

## input_elevation
### datasets
1. Single file
~~~ yaml
- datasets:                          # List of data set with specific settings
  - D:\data\top10nl\schie\ahn3.laz   # Definition for one or multiple LAS/LAZ files using the same parameters
~~~
2. Directory
~~~ yaml
- datasets:                          # List of data set with specific settings
  - D:\data\top10nl\schie\*          # Definition for reading a full directory with LAS/LAZ files using the same parameters
~~~

### omit_LAS_classes
~~~ yaml
omit_LAS_classes:                    # Option to omit classes defined in the LAS/LAZ files
  - 1 # unclassified                  # ASPRS Standard Lidar Point Classes classification value
  - 3 # vegation
  - 4 # vegation
  - 5 # vegation 
~~~

### thinning
~~~ yaml
thinning: 10                         # Thinning factor for points, this is the amount of points skipped during read, a value of 10 would result in points 1, 11, 21, 31 beeing used
~~~