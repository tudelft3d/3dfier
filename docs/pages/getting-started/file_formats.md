---
title: Supported file formats
keywords: file formats gdal shapefile geopackage obj cityjson citygml postgis
sidebar: 3dfier_sidebar
permalink: supported_file_formats.html
---

## Input formats
For 3dfier to do its magic you need to input at minimum a set of polygons and a point cloud. This page describes the file formats allowed for both. 

### 1. Polygons
For reading polygon input GDAL is used. This opens up the possibility of reading lots of different file types. Take into account that even though the file format can be read by 3dfier using GDAL, the data doesn't automatically in the right format. Some file formats like GML allow for a feature to have multiple geometry types. When the reader is not configured correctly the outcome can be unexpected. 

For all file formats supported for reading you can look the [OGR Vector drivers list](https://gdal.org/drivers/vector/index.html).

Multipolygons are supported. They will be split into single polygons. The unique identifier will be duplicated and a trailing counter will be added ``id-1, id-2, etc``.

Curvedpolygons are supported starting v1.3. When the reader comes across a curvedpolygon this is discretized using the default OGR settings. This algorithm makes sure the discretization for a curve into a linestring is identical for traversing the line from start->end as from end->start.

### 2. Point cloud
Point cloud reading is implemented using the [LASLib library](https://github.com/LAStools/LAStools/tree/master/LASlib) that is the OpenSource reading library of [LAStools](https://rapidlasso.com/lastools/). This library supports reading of LAS and LAZ point clouds.

## Output formats
The output format is defined using the command-line parameter e.g. `--CityJSON`. The output format is case sensitive.

### CityJSON
[CityJSON](http://www.cityjson.org)

CityJSON is a JSON-based encoding for storing 3D city models, also called digital maquettes or digital twins. CityJSON used the information model of CityGML.

### OBJ
[Wikipedia OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

OBJ is a geometry definition file format first developed by Wavefront Technologies. The file format is open and has been adopted by many 3D graphics application vendors. The OBJ file format is a simple data-format that represents 3D geometry alone.

When the goal is to visualise the geometries in a viewer like [MeshLab](http://www.meshlab.net/) this is the best choice as an output format. If you put the material definition file [3dfier.mtl](https://github.com/{{site.repository}}/raw/master/resources/3dfier.mtl) within the same folder of the .obj file, you will even have objects coloured according to their class.

### STL
[Wikipedia STL](https://en.wikipedia.org/wiki/STL_%28file_format%29)

3dfier currently exports to the text version of STL (binary support coming soon).

### CityGML
[CityGML.org](http://www.citygml.org/)

CityGML is a common information model and XML-based encoding for the representation, storage, and exchange of virtual 3D city and landscape models.

Writing of CityGML is supported. The output created consists of a valid CityGML schema as can be tested using this [CityGML schema validator](http://geovalidation.bk.tudelft.nl/schemacitygml/).

To write a separate file for each class there is the format specifier `CityGML-Multifile`. This will add the layer name behind the output filename followed by the proper extension like `filename + layername + .gml`. Take this into account when defining the output filename for example `testdata-` will result in `testdata-Buildings.gml`.

### IMGeo
[IMGeo catalog](https://www.geonovum.nl/geo-standaarden/bgt-imgeo/gegevenscatalogus-imgeo-versie-211)
[BGT-IMGeo](https://www.geonovum.nl/geo-standaarden/bgt-imgeo)

The Dutch basic registration BGT are stored in a CityGML ADE format called IMGeo. This `CityGML-IMGeo` output format creates a valid schema according to the [Geonovum IMGeo 2.1.1 GML Application Schema validator](http://validatie.geostandaarden.nl/etf-webapp/testruns/create-direct?testProjectId=a6a9ddd2-9ab6-3f87-98bc-bbdeb274d679)

To write a separate file for each class there is the format specifier `CityGML-IMGeo-Multifile`. This will add the layer name behind the output filename followed by the proper extension like `filename + layername + .gml`. Take this into account when defining the output filename for example `testdata-` will result in `testdata-Buildings.gml`.

### CSV
For the purpose of calculation building statistics some CSV outputs are implemented. The output writes a CSV with unique id and height statistics per building.

`CSV-BUILDINGS`, `CSV-BUILDINGS-MULTIPLE`, `CSV-BUILDINGS-ALL-Z`

The available options for CSV output are:

**`CSV-BUILDINGS`**

Returns the xx-th percentile of the z-values of the ground and roof points within a building polygon. Values are in **cm**. The percentiles are provided as:

```
lifting_options: 
  Building:
    ground:
      height: percentile-10
    roof:
      height: percentile-90
```
Output:

| id                                    | roof | floor |
|---------------------------------------|------|-------|
| b31e1feb1-00ba-11e6-b420-2bdcc4ab5d7f | 1248 | 20    |

**`CSV-BUILDINGS-MULTIPLE`**

Returns multiple percentiles of the z-values of the ground and roof points within a building polygon. Values are in **m**.

Output:

| id                                    | ground-0.00 | ground-0.10 | ground-0.20 | ground-0.30 | ground-0.40 | ground-0.50 | roof-0.00 | roof-0.10 | roof-0.25 | roof-0.50 | roof-0.75 | roof-0.90 | roof-0.95 | roof-0.99 |
|---------------------------------------|-------------|-------------|-------------|-------------|-------------|-------------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| b31bdd44c-00ba-11e6-b420-2bdcc4ab5d7f | -0.03       | 0.01        | 0.02        | 0.02        | 0.03        | 0.04        | -0.03     | 0.01      | 0.02      | 0.04      | 0.11      | 2.36      | 2.38      | 2.44      |

**`CSV-BUILDINGS-ALL-Z`**

Returns all z-values of the points within a building polygon. Values are in **cm**.

Output:

| id                                    | allzvalues |
|---------------------------------------|------------|
| b31bdfb7a-00ba-11e6-b420-2bdcc4ab5d7f | -3\|-3\|-2\|-2\|-2\|... |




### Shapefile
Writing of Shapefiles is supported in a simplified way; it writes MultiPolygonZ with vertical faces. Not all software has support for vertical faces and will show errors in the output. When using QGIS the file opens as expected.

To write a separate file for each class there is the format specifier `Shapefile-Multifile`. This will add the layer name behind the output filename followed by the proper extension like `filename + layername + .gml`. Take this into account when defining the output filename for example `testdata-` will result in `testdata-Buildings.gml`.

### PostGIS
Output of a PostGIS database is supported. Instead of the file name you have to supply the PostGIS connection string as used for GDAL too:
`3dfier example_data\testarea_config.yml --PostGIS "PG:dbname='3dfier' host='localhost' port='5432' user='username' password='password'"`.
Please make sure to double quote the connection string so its passed as a single parameter.

`PostGIS` specifier creates a database with a table per layer, a column per attribute and a geometry column with type of MultiPolygonZ.

`PostGIS-PDOK` outputs a PostGIS database as described before with an additional column XML that contains the IMGeo XML string of the object

`PostGIS-PDOK-CityGML` outputs a PostGIS database as described before with an additional column XML that contains the CityGML XML string of the object

### GDAL
Besides the list of previous described output formats 3dfier also implements a generic GDAL output driver. Same as for the file reading the output format might not support the geometric output as created. Nevertheless one can try to use this driver at its own discretion. For this driver to work you need to create a new section in the configuration file in which to configure the GDAL driver to use. The driver used needs to support creation of geometries and MultiPolygonZ.

~~~ yaml
output:
  gdal_driver: "ESRI Shapefile"
~~~