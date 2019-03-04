---
layout: default
title: How to run
group: getting-started
---

This command does not generate any model but is to verify if the program is beeing properly executed. For running 3dfier with data go to [Examples]({{site.baseurl}}/pages/examples/example-data)

***
3dfier is a command-line utility that does not have a graphical user interface (GUI). This means you need to run it from [Command Prompt](https://www.lifewire.com/how-to-open-command-prompt-2618089).
You can do this by opening the Command Prompt (press windows button+R, type cmd and press enter) and dragging the 3dfier program into the Command Prompt window. If you now press enter the 3dfier program should output:

```
3dfier Copyright (C) 2015-2018  3D geoinformation research group, TU Delft
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