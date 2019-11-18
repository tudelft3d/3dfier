---
title: Output options
keywords: settings config configuration
sidebar: 3dfier_sidebar
permalink: output_options.html
---

## options
~~~ yaml
building_radius_vertex_elevation: 3.0                 # Radius in meters used for point-vertex distance between 3D points and vertices of building polygons, radius_vertex_elevation used when not specified
radius_vertex_elevation: 1.0                          # Radius in meters used for point-vertex distance between 3D points and vertices of polygons
threshold_jump_edges: 0.5                             # Threshold in meters for stitching adjacent objects, when the height difference is larger then the threshold a vertical wall is created 
threshold_bridge_jump_edges: 0.5                      # Threshold in meters for stitching bridges to adjacent objects, if not specified it falls back to threshold_jump_edges
max_angle_curvepolygon: 0.0                           # The largest allowed angle along the stroked arc of a curved polygon. Use zero for the default setting. (https://gdal.org/doxygen/ogr__api_8h.html#a87f8bce40c82b3513e36109ea051dff2)
extent: xmin, ymin, xmax, ymax                        # Filter the input polygons to this extent
~~~

### radius_vertex_elevation
{% include imagezoom.html file="/settings/radius_vertex_elevation.png" alt="" %}

### building_radius_vertex_elevation
Same as [radius_vertex_elevation](#radius_vertex_elevation) but specific to vertices of buildings.

### threshold_jump_edges

### threshold_bridge_jump_edges

### max_angle_curvepolygon
Please refer to the OGR API documentation for the function [OGR_G_ApproximateArcAngles()](https://gdal.org/doxygen/ogr__api_8h.html#a87f8bce40c82b3513e36109ea051dff2) that is used to stroke an arc to a line string.

### extent
*Download [YAML]({{site.baseurl}}/assets/configs/extents.yml) and [OBJ]()*
As one can see from the examples below, all objects of which its bounding box intersects with the extent is added to the output.

{% include imagezoom.html file="/settings/settings_extent.png" alt="" %}

There are several areas clipped to show the results. In the following image the used extents are shown and their colors represent that of the results below.
{% include imagezoom.html file="/settings/extents.png" alt="" %}

**Green extent**
{% include imagezoom.html file="/settings/extent_green.png" alt="" %}
**Blue extent**
{% include imagezoom.html file="/settings/extent_blue.png" alt="" %}

**Purple extent**
{% include imagezoom.html file="/settings/extent_purple.png" alt="" %}