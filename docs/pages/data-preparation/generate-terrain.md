---
layout: default
title: Generate terrain
group: data-preparation
---

Without a 2D polygon describing terrain extent, 3dfier is not able to create a terrain from a point cloud without. The solution is to create a polygon of the area of interest save this as a shapefile.

We make sure the terrain and buildings are stitched nicely and the buildings do not intersect with the terrain. We do this by using symetrical difference between the terrain polygon and the buildings. For nice tutorial about this see section *D. Symmetrical Difference* at [GrindGIS](http://grindgis.com/software/qgis/basic-editing-tools-in-qgis).

### Terrain
![image](https://user-images.githubusercontent.com/30265851/32222599-30d38b70-be3a-11e7-90ad-000218305924.png)

### Symmetrical Difference operation result
![image](https://user-images.githubusercontent.com/30265851/32222645-62cf1964-be3a-11e7-9a70-746222d0c53a.png)

### 2D terrain + buildings
![image](https://user-images.githubusercontent.com/30265851/32222677-7acc1a3a-be3a-11e7-8737-c0637005a6ba.png)

### 3D model as OBJ in Meshlab
![image](https://user-images.githubusercontent.com/30265851/32222728-b87a5112-be3a-11e7-892e-32e4afee01ca.png)

### Viewpoint below the 3D model
![image](https://user-images.githubusercontent.com/30265851/32222770-e95b2a40-be3a-11e7-88c2-c026412ebc9e.png)

Thanks to [@antoinebio](https://github.com/antoinebio) for the images supplied in [issue #48](https://github.com/tudelft3d/3dfier/issues/48).