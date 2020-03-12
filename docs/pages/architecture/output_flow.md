---
title: Model output flow
keywords: output model flow
sidebar: 3dfier_sidebar
permalink: output_flow.html
---

## Writing a model including database and GDAL flow
To output a model all output formats defined in the command line options are iterated and the corresponding format is created. Below the steps for format decision making is shown including writing to GDAL formats and a PostGIS database. The code for writing to PostGIS is depending on GDAL too. Therefore the code for creating features, geometry and attributes is reused. The difference is where the GDAL formats create a dataset (file) per layer where the PostGIS database creates a single dataset (database) with multiple layers. For all other layers lookup the corresponding flow diagrams in the following sections.

{% include imagezoom.html file="flows/3dfier_writing_model.png" alt="Flow diagram for writing models" %}

## CityJSON
Creating CityJSON output is similar to [OBJ](#obj) with some small differences. CityJSON supports Solid objects and attributes where OBJ does not. For Building objects it makes LoD1 Solid objects and includes floors with `floor: true` configured. All other objects are created as MultiSurface objects.

For buildings it writes the `min height surface` attribute with the floor height and `measuredHeight` attribute with the absolute roof height (roof-floor).
{% include imagezoom.html file="flows/3dfier_writing_model_CityJSON.png" alt="Flow diagram for writing models in CityJSON" %}

## OBJ
OBJ output supports only writing objects as MultiSurface. Buildings are supported in both LoD0 and LoD1. When configured LoD0 produces a building with the roof polygon at floor height. This resembles the foundation of the house and can be used in case there are existing building models.

Using a work-around objects in an OBJ model contain an ID and are semantically labelled by defining a material.
~~~
o b222b6a92-00b5-11e6-b420-2bdcc4ab5d7f
usemtl Terrain
f 2596 2597 2598
f 2598 2599 2600
f 2597 2601 2602
~~~
{% include imagezoom.html file="flows/3dfier_writing_model_OBJ.png" alt="Flow diagram for writing models in OBJ" %}

## CityGML
The output for CityGML can be influenced by the configuration in two ways. Setting `triangulation: true` will create a triangulated MultiSurface Building object instead of a LoD1 Solid. `floor: true` will result in the model to contain the floor polygons in buildings. 

For buildings it writes the `min height surface` attribute with the floor height and `measuredHeight` attribute with the absolute roof height (roof-floor).
{% include imagezoom.html file="flows/3dfier_writing_model_CityGML.png" alt="Flow diagram for writing models in CityGML" %}

## IMGeo
IMGeo is an Application Domain Extension of [CityGML](#citygml). This means the core of the data model is equal to that of CityGML but some classes are extended to describe specific (meta)data. This data format is used by the government in The Netherlands and many of the datasets are in this data format. The flow is comparable to that of CityGML except for two things; 1) LoD0 floor and roof polygons are not stored and 2) IMGeo has specific attributes that are copied from the input to the correct output attributes.
{% include imagezoom.html file="flows/3dfier_writing_model_IMGeo.png" alt="Flow diagram for writing models in IMGeo" %}

## CSV
The model output in CSV format is a fantastic tool for building statistics. The data can be output in three different versions:
1. Single percentile - Write `object id,roof height at percentile,floor height at percentile`
2. Multiple percentile - Write `object id,roof height at percentiles,floor height at percentiles` for floor percentiles; 0, 10, 20, 30, 40, 50 and for roof percentiles 0, 10, 25, 50, 75, 90, 95, 99.
3. All elevation values - Write `object id,elevation value,elevation value,elevation value,...` 

{% include imagezoom.html file="flows/3dfier_writing_model_CSV.png" alt="Flow diagram for writing models in CSV" %}
