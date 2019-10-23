---
title: Supported file formats
keywords: file formats gdal shapefile geopackage obj cityjson citygml postgis
sidebar: 3dfier_sidebar
permalink: supported_file_formats.html
---

## Input formats

### 1. Polygons
- GDAL OGR driver supported vector

Multipolygons are supported. They will be split into single polygons. The unique indentifier will be duplicated and a trailing counter ``id-1, id-2, etc`` will be added.

### 2. Point cloud
- LAS
- LAZ

## Output formats
### CityJSON
[CityJSON](http://www.cityjson.org)

### OBJ
[Wikipedia OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

### CityGML
[CityGML.org](http://www.citygml.org/)

### IMGeo
[IMGeo catalog](https://www.geonovum.nl/geo-standaarden/bgt-imgeo/gegevenscatalogus-imgeo-versie-211)
[BGT-IMGeo](https://www.geonovum.nl/geo-standaarden/bgt-imgeo)

### CSV