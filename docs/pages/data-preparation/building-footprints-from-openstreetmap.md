---
layout: default
title: Building footprints from OpenStreetMap
group: data-preparation
---

This guide is about extracting building footprints as a Shapefile of polygons from the [OpenStreetMap](https://www.openstreetmap.org) dataset, in order to use them as input for 3dfier.

## Download OSM data 

First, you need to download the OSM data for your area of interest:

1. Through your browser, visit the [OpenStreetMap](https://www.openstreetmap.org) website

1. Zoom at the area you want to work on

1. Press the `Export` button on the top

1. From the left panel, you may select to further specify the area you want to download (via the `Manually select a different area` option)

1. When finished, press the `Export` button from the left panel.

You should be asked to download an `.osm` file. Just store it somewhere on your computer.

## Extract buildings through QGIS 

Originally, the data from OpenStreetMap are just geometries with key-value pairs assigned to them. You can easily filter the buildings from all geometries inside an `.osm` file.

1. Open QGIS.

1. From the menu, select `Layers`->`Add Layer`->`Add Vector layer...`.

1. Select the `.osm` file with the area downloaded.

1. When prompted about the layer you want to add, you can only select the `multipolygons` layer (it's OK if you add all of them, but buildings are on this layer).

1. From the `Layers Panel`, right-click on the multipolygons layer you've just added and select `Filter...`.

1. On the dialog, provide the following expression: `"building" is not null` (essentially, what that means is that all polygons without a _building_ key, will be filtered out). You should now see only the buildings on the map.

1. From the `Layers Panel`, right-click on the multipolygons layer, again, and now select `Save as...`.

1. On the dialog, select the `ESRI Shapefile` format and provide the output file. You may also want to reproject the geometries on another CRS, now, if the elevation data you are going to use on 3dfier are not on WGS 84 (EPSG:4326).

When you save, you should have a shapefile with footprint of the buildings for this area, at the CRS you specified. You can use this, now, as an input for 3dfier.