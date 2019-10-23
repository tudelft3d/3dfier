---
title: Getting started with 3dfier
keywords: 3dfier homepage
summary: These instructions will help you to get started with 3dfier. Other sections will provide additional information on the use of the software.
sidebar: 3dfier_sidebar
permalink: index.html
---

## Install using binaries
Binary release only exist for Windows users. Others will have to follow one of the other installation guides for [Linux]({{site.baseurl}}/install_ubuntu) or [Docker]({{site.baseurl}}/install_docker)
There exists a ready-to-use version of [3dfier for Windows 64-bit]({{site.github.repository_url}}/releases/latest). Download and extract the files to any given folder and follow the instructions in the [Get started guide]({{site.baseurl}}/index).

## Your first time running
3dfier is a command-line utility that does not have a graphical user interface (GUI). This means you need to run it from [Command Prompt](https://www.lifewire.com/how-to-open-command-prompt-2618089).
You can do this by opening the Command Prompt (press windows button+R, type cmd and press enter) and dragging the 3dfier program into the Command Prompt window. If you now press enter the 3dfier program should output:

```
3dfier Copyright (C) 2015-2019  3D geoinformation research group, TU Delft
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; for details run 3dfier with the '--license' option.

ERROR: one YAML config file must be specified.

Allowed options:
  --help                        View all options
  --version                     View version
  --license                     View license
  --OBJ arg                     Output
  --OBJ-NoID arg                Output
  --CityGML arg                 Output
  --CityGML-Multifile arg       Output
  --CityGML-IMGeo arg           Output
  --CityGML-IMGeo-Multifile arg Output
  --CityJSON arg                Output
  --CSV-BUILDINGS arg           Output
  --CSV-BUILDINGS-MULTIPLE arg  Output
  --CSV-BUILDINGS-ALL-Z arg     Output
  --Shapefile arg               Output
  --Shapefile-Multifile arg     Output
  --PostGIS arg                 Output
  --PostGIS-PDOK arg            Output
  --PostGIS-PDOK-CityGML arg    Output
  --GDAL arg                    Output
```

## Data that is generated
Only data is generated for polygons supplied in the input. If no polygon is input for terrain it is not generated. There is no option (yet) for the creation of terrain within the bounding box of the polygons extent or point file extent. To create a terrain polygon with building footprints cut out please follow the Data preperation [Generate terrain polygon]({{site.baseurl}}/generate_terrain)