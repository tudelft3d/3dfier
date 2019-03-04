---
layout: default
---

# The open-source tool for creation of 3D models
* * *

Developped by the **3D Geoinformation group** at **Delft University of Technology**, 3dfier tries to fill the gap for simply creatinging 3D models. It takes 2D GIS datasets (e.g. topographical datasets) and "3dfies" them (as in "making them three-dimensional"). The elevation is obtained from a point cloud (we support LAS/LAZ at this moment), and the semantics of every polygon is used to perform the lifting. After lifting, elevation gaps between the polygons are removed by "stitching" together the polygons based on rules so that a watertight digital surface model (DSM) fused with 3D objects is constructed. A rule based stucture is used to extrude water as horizontal polygons, create LOD1 blocks for buildings, smooth road surfaces and construct bridges (floating 3D polygons).

Our aim is to obtain one model that is error-free, so no intersecting objects, no holes (the surface is watertight) and buildings are integrated in the surface.

![3dfier output](/img/Delft_3dfier-3.png)
