---
title: Generate terrain polygon
keywords: terrain
sidebar: 3dfier_sidebar
permalink: generate_terrain.html
---

Without a 2D polygon describing terrain extent, 3dfier is not able to create a terrain from a point cloud without. The solution is to create a polygon of the area of interest save this as a shapefile.

We make sure the terrain and buildings are stitched nicely and the buildings do not intersect with the terrain. We do this by using symetrical difference between the terrain polygon and the buildings. For nice tutorial about this see section *D. Symmetrical Difference* at [GrindGIS](http://grindgis.com/software/qgis/basic-editing-tools-in-qgis).

### Terrain
{% include imagezoom.html file="https://user-images.githubusercontent.com/30265851/32222599-30d38b70-be3a-11e7-90ad-000218305924.png" external=true %}

### 2D terrain + buildings
{% include imagezoom.html file="https://user-images.githubusercontent.com/30265851/32222677-7acc1a3a-be3a-11e7-8737-c0637005a6ba.png" external=true %}

### Symmetrical Difference operation result
{% include imagezoom.html file="https://user-images.githubusercontent.com/30265851/32222645-62cf1964-be3a-11e7-9a70-746222d0c53a.png" external=true %}

### 3D model as OBJ in Meshlab
{% include imagezoom.html file="https://user-images.githubusercontent.com/30265851/32222728-b87a5112-be3a-11e7-892e-32e4afee01ca.png" external=true %}

### Viewpoint below the 3D model
{% include imagezoom.html file="https://user-images.githubusercontent.com/30265851/32222770-e95b2a40-be3a-11e7-88c2-c026412ebc9e.png" external=true %}

Thanks to [@antoinebio](https://github.com/antoinebio) for the images supplied in [issue #48](https://github.com/tudelft3d/3dfier/issues/48).