---
title: Different LoD of buildings
keywords: LoD buildings examples
sidebar: 3dfier_sidebar
permalink: lod_buildings.html
---

## What's the difference between LoD0 and LoD1 buildings?
The most simple way to explain the difference between LoD0 and LoD1 is to use the way we describe a building in real life. The floor, walls and roof of a building together make a building. When reconstructing an LoD0 building only the floor is created as a flat surface within the surrounding terrain. A LoD1 building adds the walls and roof to a building. In the LoD1 version the roof is describe as a flat surface.


**IMPORTANT: LoD0 is currently implemented for OBJ output only**

## LoD0 reconstruction
*Download [YAML]({{site.baseurl}}/assets/configs/lod0.yml) and [OBJ]({{site.baseurl}}/assets/configs/lod0.obj)*

**Point classes used: ground points**

Buildings created in LoD0 are flat surfaces. The surface height is calculated according to the ground points detected around the footprint. The percentile statistics determine the final height. All objects around the building are stitched to these heights or in case of water an additional vertical wall is created. Result is that terrain around the building and the buildings flat surface reconstruction is identical to that of the LoD1 reconstruction.

The LoD0 reconstruction is particularly useful when not in need of buildings in the model. E.g. if there is an existing LoD2 model and a watertight terrain model is to be created.

{% include imagezoom.html file="/lod0-delft.png" alt="" %}
{% include imagezoom.html file="/lod0-delft-zoom.png" alt="" %}

Use the following settings for the example dataset to reconstruct LoD0 buildings:
~~~ yaml
input_polygons:
  - datasets: 
      - bgt\bgt_pand.sqlite
    uniqueid: gml_id
    lifting: Building
    height_field: relatievehoogteligging
~~~
~~~ yaml
lifting_options: 
  Building:
    lod: 0
    floor: true
    triangulate: false
    ground:
      height: percentile-10
      use_LAS_classes:
        - 2
        - 9
    roof:
      height: percentile-90
      use_LAS_classes:
        - 6
~~~
~~~ yaml
input_elevation:
  - datasets:
      - ahn3\ahn3_cropped_1.laz
      - ahn3\ahn3_cropped_2.laz
    omit_LAS_classes:
      - 0 # Never classified
      - 1 # Unclassified
    thinning: 0

options:
  building_radius_vertex_elevation: 3.0
  radius_vertex_elevation: 1.0
  threshold_jump_edges: 0.5
~~~

## LoD1 reconstruction
*Download [YAML]({{site.baseurl}}/assets/configs/lod1.yml) and [OBJ]({{site.baseurl}}/assets/configs/lod1.obj)*

**Point classes used: ground and non-ground points**

The reconstruction of buildings in LoD1 result in cubes with flat roof surfaces and extruded walls between floor and roof surfaces. Height for the floor is calculated like explained in [LoD0 reconstruction](#lod0-reconstruction). Roof height is based on all points in distance of the vertices and points within the footprint. Points are filtered by class set with the *use_LAS_classes* setting. The percentile statistics determine the final height.

{% include imagezoom.html file="/lod1-delft.png" alt="" %}

Use the following settings for the example dataset to reconstruct LoD1 buildings:
~~~ yaml
input_polygons:
  - datasets: 
      - bgt\bgt_pand.sqlite
    uniqueid: gml_id
    lifting: Building
    height_field: relatievehoogteligging
~~~
~~~ yaml
lifting_options: 
  Building:
    lod: 1
    floor: true
    inner_walls: true
    triangulate: false
    ground:
      height: percentile-10
      use_LAS_classes:
        - 2
        - 9
    roof:
      height: percentile-90
      use_LAS_classes:
        - 6
~~~
~~~ yaml
input_elevation:
  - datasets:
      - ahn3\ahn3_cropped_1.laz
      - ahn3\ahn3_cropped_2.laz
    omit_LAS_classes:
      - 0 # Never classified
      - 1 # Unclassified
    thinning: 0

options:
  building_radius_vertex_elevation: 3.0
  radius_vertex_elevation: 1.0
  threshold_jump_edges: 0.5
~~~