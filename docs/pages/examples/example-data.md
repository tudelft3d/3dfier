---
layout: default
title: Example data
group: examples
---

## Prepare example data
For this example we use [BGT_Delft_Example.zip]({{site.github.repository_url}}/raw/master/resources/Example_data/BGT_Delft_Example.zip) from the GitHub repository located in `3dfier/resources/Example_data/`. Create a folder with 3dfier and the depencency dll's and add the `example_data folder`.

![Folder layout](/img/example_folder.png)

## Running with the example data
Opening the Command Prompt (press windows button+R, type cmd and press enter) and navigate to the folder where 3dfier is located.

Now go into the example data folder using `cd example_data` and run `..\3dfier.exe testarea_config.yml --OBJ output\myfirstmodel.obj`. Now 3dfier will start processing and when finished it produced its first 3D model!

The output file can be found here `example_data\output\testarea.obj` and console output should be as follows.

```
3dfier Copyright (C) 2015-2018  3D geoinformation research group, TU Delft
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; for details run 3dfier with the '--license' option.

Config file is valid.
Reading input dataset: bgt\bgt_waterdeel.sqlite
        Layer: waterdeel
        (3 features --> Water)
Reading input dataset: bgt\bgt_ondersteunendwaterdeel.sqlite
        Layer: ondersteunendwaterdeel
        (0 features --> Water)
Reading input dataset: bgt\bgt_onbegroeidterreindeel.sqlite
        Layer: onbegroeidterreindeel
        (81 features --> Terrain)
Reading input dataset: bgt\bgt_wegdeel.sqlite
        Layer: trafficarea
        (151 features --> Road)
Reading input dataset: bgt\bgt_ondersteunendwegdeel.sqlite
        Layer: auxiliarytrafficarea
        (0 features --> Road)
Reading input dataset: bgt\bgt_pand.sqlite
        Layer: buildingpart
        (160 features --> Building)
Reading input dataset: bgt\bgt_begroeidterreindeel.sqlite
        Layer: plantcover
        (126 features --> Forest)
Reading input dataset: bgt\bgt_scheiding.sqlite
        Layer: scheiding
        (57 features --> Separation)
Reading input dataset: bgt\bgt_kunstwerkdeel.sqlite
        Layer: kunstwerkdeel
        (1 features --> Separation)
Reading input dataset: bgt\bgt_overigbouwwerk.sqlite
        Layer: overigbouwwerk
        (0 features --> Separation)
Reading input dataset: bgt\bgt_overbruggingsdeel.sqlite
        Layer: bridgeconstructionelement
        (3 features --> Bridge/Overpass)

Total # of polygons: 570
Constructing the R-tree... done.
Spatial extent: (84,616.468, 447,422.999) (85,140.839, 447,750.636)
Reading LAS/LAZ file: ahn3\ahn3_cropped_1.laz
        (640,510 points in the file)
        (all points used, no skipping)
        (omitting LAS classes: 0 1 )
[==================================================] 100%
Reading LAS/LAZ file: ahn3\ahn3_cropped_2.laz
        (208,432 points in the file)
        (all points used, no skipping)
        (omitting LAS classes: 0 1 )
[==================================================] 100%
All points read in 1 seconds || 00:00:01
3dfying all input polygons...
===== /LIFTING =====
===== LIFTING/ =====
=====  /ADJACENT FEATURES =====
=====  ADJACENT FEATURES/ =====
=====  /STITCHING =====
=====  STITCHING/ =====
=====  /BOWTIES =====
=====  BOWTIES/ =====
=====  /VERTICAL WALLS =====
=====  VERTICAL WALLS/ =====
Lifting, stitching and vertical walls done in 0 seconds || 00:00:00
=====  /CDT =====
=====  CDT/ =====
CDT created in 0 seconds || 00:00:00
...3dfying done.
OBJ output: output\myfirstmodel.obj
Features written in 0 seconds || 00:00:00
Successfully terminated in 2 seconds || 00:00:02
```