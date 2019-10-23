---
title: Input options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: input_options.html
---

## input_polygons
### datasets
1. flat config
~~~ yaml
- datasets:
  - D:\data\campus\partof.shp
	uniqueid: FACE_ID
	lifting: Building
~~~

2. nested config
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
### handle_multiple_heights

## input_elevation
### datasets
### omit_LAS_classes
### thinning