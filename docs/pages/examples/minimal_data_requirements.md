---
title: Minimal data requirements
keywords: 3dfier homepage
sidebar: 3dfier_sidebar
permalink: minimal_data_requirements.html
---

## What to input?
Using the software can have different reasons and end-goals. These examples describe what the minimal data requirements are to create a 3D model using 3dfier. The first thing needed is polygons describing the objects to make 3D. Next is the height information to be added to these polygons in the form of a point cloud. Both input types must have a file format from the list of [Supported file formats]({{site.baseurl}}/supported_file_formats.html).

Besides the data files the software needs a configuration file formatted as YAML. This configuration contains all settings except the output format. All possibilities are explained in [Settings]({{site.baseurl}}/input_options.html). In the following sections various possibilities and their minimum data requirements are covered.

{% include imagezoom.html file="extrusion.png" alt="Data input and extrusion steps" %}

## Buildings only
For the creation of a 3D model consisting of only LoD1 buildings there is the need for the following data:
- Polygons of building footprints
- Point cloud with minimum classification:
	- Ground
	- Non-ground

*Why the need for classification in ground/non-ground for points?* While developing the algorithms thorough testing with non classified point clouds have been done. The results showed that for the algorithm to work it cannot do without differentiating between ground and non-ground. The main reason is due to vegetation. The noise introduced by vegetation cannot be overcome by the statistical analysis.

In this example configuration the ground height for the building is calculated by using only the points classified as ground (class 2). The height of the roof of buildings is calculated by using all point classes. This will also take into account ground points for the roof. However using a high percentile makes the result less prone to picking a ground points as the roof height.

*Note: having more LAS classes gives better results since there is less noise within the configured classes. E.g. there is (almost) no vegetation points in the building classes so building heights are not interfered by trees.*

~~~yaml
input_polygons:
  - datasets: 
      - bgt\bgt_pand.sqlite
    uniqueid: gml_id
    lifting: Building
    height_field: relatievehoogteligging

lifting_options: 
  Building:
    ground:
      height: percentile-10
      use_LAS_classes:
        - 2
    roof:
      height: percentile-90

input_elevation:
  - datasets:
      - ahn3\ahn3_cropped_1.laz
      - ahn3\ahn3_cropped_2.laz
~~~

## All objects
For the creation of a 3D model containing all objects in an area there is the need for the following data:
- Topologically connected polygons
- Point cloud with minimum classification:
	- Ground
	- Non-ground

In this example we configured all classes to used the ground/non-ground classification. All lifting_options of objects related to ground-like objects (terrain, forest, water and road) are set to only use ground (class 2) and other objects to use all points. 

~~~yaml
input_polygons:
  - datasets: 
      - bgt\bgt_waterdeel.sqlite
      - bgt\bgt_ondersteunendwaterdeel.sqlite
    uniqueid: gml_id
    lifting: Water
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_onbegroeidterreindeel.sqlite
    uniqueid: gml_id
    lifting: Terrain
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_wegdeel.sqlite
      - bgt\bgt_ondersteunendwegdeel.sqlite
    uniqueid: gml_id
    lifting: Road
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_pand.sqlite
    uniqueid: gml_id
    lifting: Building
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_begroeidterreindeel.sqlite
    uniqueid: gml_id
    lifting: Forest
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_scheiding.sqlite
      - bgt\bgt_kunstwerkdeel.sqlite
      - bgt\bgt_overigbouwwerk.sqlite
    uniqueid: gml_id
    lifting: Separation
    height_field: relatievehoogteligging
  - datasets: 
      - bgt\bgt_overbruggingsdeel.sqlite
    uniqueid: gml_id
    lifting: Bridge/Overpass
    height_field: relatievehoogteligging

lifting_options: 
  Building:
    ground:
      height: percentile-10
      use_LAS_classes:
        - 2
    roof:
      height: percentile-90
  Terrain:
    simplification_tinsimp: 0.1
    use_LAS_classes:
      - 2
  Forest:
    simplification_tinsimp: 0.1
    use_LAS_classes:
      - 2
  Water:
    height: percentile-10
  Road:
    height: percentile-50
    use_LAS_classes:
      - 2
  Separation:
    height: percentile-80
  Bridge\Overpass:
    height: percentile-50

input_elevation:
  - datasets:
      - ahn3\ahn3_cropped_1.laz
      - ahn3\ahn3_cropped_2.laz
~~~