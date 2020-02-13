---
title: Output options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: output_options.html
---

## options
~~~ yaml
building_radius_vertex_elevation: 3.0  # Radius in meters used for point-vertex distance between 3D points and vertices of building polygons, radius_vertex_elevation used when not specified
radius_vertex_elevation: 1.0           # Radius in meters used for point-vertex distance between 3D points and vertices of polygons
threshold_jump_edges: 0.5              # Threshold in meters for stitching adjacent objects, when the height difference is larger then the threshold a vertical wall is created 
threshold_bridge_jump_edges: 0.5       # Threshold in meters for stitching bridges to adjacent objects, if not specified it falls back to threshold_jump_edges
max_angle_curvepolygon: 0.0            # The largest allowed angle along the stroked arc of a curved polygon. Use zero for the default setting. (https://gdal.org/doxygen/ogr__api_8h.html#a87f8bce40c82b3513e36109ea051dff2)
extent: xmin, ymin, xmax, ymax         # Filter the input polygons to this extent
~~~

### radius_vertex_elevation
*Default value: 1.0m.* 
Describes the maximum distance between a vertex and a height points. If the point is within radius of the vertex it will be stored in the vertex height list.
{% include imagezoom.html file="/settings/radius_vertex_elevation.png" alt="" %}

### building_radius_vertex_elevation
*Default value: 3.0m.*
Same as [radius_vertex_elevation](#radius_vertex_elevation) but specific to vertices of buildings. Radius for buildings is larger for finding appropriate ground points due to shading effects of the building itself. In locations where streets are narrow and buildings close to each other it can be hard to find ground points. By setting a larger radius this can be solved. In cases with lots of relief, a larger radius can introduce an incorrect ground height since too many points are taken into account.

### threshold_jump_edges
*Default value: 0.5m.*
When stitching objects their height difference is taken into account. This threshold sets the minimum height difference between to objects to result in a vertical wall to be created. Two objects with a height difference smaller then the threshold will be stitched by adjusting their heights based on set rules.

### threshold_bridge_jump_edges
*Default value: 0.5m.*
Same as [threshold_jump_edges](#threshold_jump_edges) but specific for bridges. In the case of bridges the threshold must sometimes be set to a larger value due to a lower accuracy in height data of the terrain around a bridge. Mostly the case when using Dense Matching point clouds.

### max_angle_curvepolygon
*Default value: 4 degrees.*
Use this setting when using curved polygons as input. Creation of a 3D object from an arc is not possible. Therefore the arcs need to be stroked to lines. The OGR algorithm strokes the arcs identical in both directions of the arc. Because of this the topology is maintained and assured for the stroked lines. Please refer to the OGR API documentation for the function [OGR_G_ApproximateArcAngles()](https://gdal.org/doxygen/ogr__api_8h.html#a87f8bce40c82b3513e36109ea051dff2) that is used to stroke an arc to a line string.

### extent
*Download [YAML]({{site.baseurl}}/assets/configs/extent_green.yml) and [OBJ]({{site.baseurl}}/assets/configs/extent_green.obj)*
As one can see from the examples below, all objects of which its bounding box intersects with the extent is added to the output. This can be used to clip an area without the need to change input configuration.

{% include imagezoom.html file="/settings/settings_extent.png" alt="" %}

There are several areas clipped to show the results. In the following image the used extents are shown and their colors represent that of the results below.
{% include imagezoom.html file="/settings/extents.png" alt="" %}

**Green extent**
*Download [YAML]({{site.baseurl}}/assets/configs/extent_green.yml) and [OBJ]({{site.baseurl}}/assets/configs/extent_green.obj)*
{% include imagezoom.html file="/settings/extent_green.png" alt="" %}
**Blue extent**
*Download [YAML]({{site.baseurl}}/assets/configs/extent_blue.yml) and [OBJ]({{site.baseurl}}/assets/configs/extent_blue.obj)*
{% include imagezoom.html file="/settings/extent_blue.png" alt="" %}
**Purple extent**
*Download [YAML]({{site.baseurl}}/assets/configs/extent_purple.yml) and [OBJ]({{site.baseurl}}/assets/configs/extent_purple.obj)*
{% include imagezoom.html file="/settings/extent_purple.png" alt="" %}
**Red extent**
*Download [YAML]({{site.baseurl}}/assets/configs/extent_red.yml) and [OBJ]({{site.baseurl}}/assets/configs/extent_red.obj)*
{% include imagezoom.html file="/settings/extent_red.png" alt="" %}