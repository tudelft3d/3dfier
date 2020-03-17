---
title: Threedfy flow
keywords: threedfy magic flow
sidebar: 3dfier_sidebar
permalink: threedfy_flow.html
---

## Threedfy flow
All steps done by the Threedfy algorithm are described in the flow diagram below. The yellow blocks refer to nested flow diagrams described in the other sections on this page.

{% include imagezoom.html file="flows/threedfy.png" alt="Flow diagram for threedfying data" %}

## Lifting
Lifting means that all vertices imported from the 2D polygonal geometries are lifted to 3D by statistical analysis of the height points read from the point clouds. All 3D points in a given radius around the vertex are used to extract an appropriate height for the objects vertex.
{% include imagezoom.html file="flows/threedfy_lifting.png" alt="Flow diagram for lifting of polygons" %}

## Stitching
Stitching of polygons is filling holes created by the lifting step in the threedfy algorithm. Topologically connected objects are stitched (like sewing) vertex wise by looking at their type, height an connectedness. Stitching is the most complex part of the algorithm that defines the rules for the final model.

{% include imagezoom.html file="flows/threedfy_stitching.png" alt="Flow diagram for stitching of polygons" %}

## Fix bow ties
A bow tie is a location where due to the stitching the heights of two adjacent objects intersect like a bow tie. An example is two vertices A and B from two adjacent objects. Vertex A from object 1 has height 0 and vertex A from object 2 has height 10 meters. Vertex B in object 1 has a height of 5 meters and vertex B in object 2 has a height of 0 meters. This will result in the creation of a bow tie and therefore not a closed watertight surface. When fixing these issues object types are checked like with the stitching and one of the two objects is stitched to the other in a way the bow tie will be removed.

{% include imagezoom.html file="flows/threedfy_fix_bowties.png" alt="Flow diagram for fixing bow ties" %}

## Create vertical walls
Vertical walls are assigned to objects during the stitching process. When the height difference between the objects is larger then the configured height jump threshold a boolean is set in the object to register it contains vertical walls. The vertical walls are created in this final step. The walls are reconstructed as a collection of triangles in the shape of a fan. Using this the algorithm can make sure all possible heights are topologically connected to the neighbouring objects.

{% include imagezoom.html file="flows/threedfy_vertical_walls.png" alt="Flow diagram for creation of vertical walls" %}