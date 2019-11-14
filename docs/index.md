---
title: The open-source tool for creation of 3D models
keywords: 3dfier homepage
summary: 
sidebar: 3dfier_sidebar
permalink: index.html
---

3dfier tries to fill the gap for simply creating 3D models. It takes 2D GIS datasets (e.g. topographical datasets) and "3dfies" them (as in "making them three-dimensional"). The elevation is obtained from a point cloud (we support LAS/LAZ at the moment), and the semantics of every polygon is used to perform the lifting. After lifting, elevation gaps between the polygons are removed by "stitching" together the polygons based on rules so that a watertight digital surface model (DSM) fused with 3D objects is constructed. A rule based stucture is used to extrude water as horizontal polygons, create LOD1 blocks for buildings, smooth road surfaces and construct bridges (3D polygonal surface). This software is developped by the [3D Geoinformation group](https://3d.bk.tudelft.nl) at **Delft University of Technology**.

Our aim is to obtain one model that is error-free, so no intersecting objects, no holes (the surface is watertight) and buildings are integrated in the surface.

{% include imagezoom.html file="leiden3dfier.png" alt="3dfier result of Delft" %}
