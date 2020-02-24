---
title: Main data flow
keywords: main flow
sidebar: 3dfier_sidebar
permalink: main_flow.html
---

## Guide for reading
How is the architecture of 3dfier described and how would you read it.

## Data flow through the program
**TODO: Explain different steps in the flow diagram. Show what data goes were and link to following flow diagrams.**
{% include imagezoom.html file="flows/3dfier_general_flow.png" alt="General flow diagram 3dfier" %}

### Configuration
Reading and validation of the configuration is done using the [YAML-CPP library](https://github.com/jbeder/yaml-cpp). It parses the file into an object and allows for easy access to all values. The configuration file is read and validated first. Value types are tested and options with limited options are verified. If validation fails an error message will be printed and the program execution will stop.

When validation passes, all values are used to set the corresponding parameters in the software. After this initial read the configuration file is not used anymore since all values are now stored in memory.

{% include imagezoom.html file="flows/3dfier_classes.png" alt="Diagram of 3dfier classes" %}

### Data integrity check
For all input files configured to use a check is done if they exist and readable. The polygon files are opened using an OGR reader and closed directly after. Same is done for the LAS/LAZ files, they are opened with LASlib and directly closed. This prevents failure of the program after partly executing. Also the output filepaths are tested to be existing before execution of the algorithm.

*NOTE: Output folder must exist, it's not created automatically*

### Reading polygons
If all sanity checks are passed the program turns to reading the polygon files. It takes into account the extent setting, only polygons which bounding box intersects with the given extent will be added. 

A file is opened for reading and the id and height_field attributes are loaded. If these attributes do not exist the program will stop execution with an exception. The total number of features in the layer is logged and counters are set for the amount of multipolygons. Next each feature is read, the attributes are stored in a new TopoFeature. A TopoFeature is the internal storage class used by the algorithm to store attributes, 2D and 3D geometries and functionality to lift the objects. The geometry is extracted and depending on the polygon type it is pre-processed. For a multipolygon each separate polygon is added to a new TopoFeature in which the attributes are copied. The id of the feature is altered by adding a trailing dash and counter, e.g. 'id-0, id-1'. Curvepolygons are automatically stoked into small straight line segments. 

*Important: When reading the geometry the boost::geometry library is used to read WKT and both functions boost::geometry::unique and boost::geometry::correct are used for removing duplicate vertices and correcting ring orientation.*

During the creation of a TopoFeature and storing its geometry, all used buffers and vectors are resized to correspond to the amount of points in the polygon outer and inner rings. This preallocates most of the needed memory for storing the elevation information acquired when reading the point clouds.

For a later stage a spatial index is created that stores the bounding box of the geometries with a reference to the TopoFeature. This speeds up filtering the TopoFeatures within range for the point to polygon distance calculations done with the points from the point cloud. The spatial index is a [Boost R-tree](https://www.boost.org/doc/libs/1_72_0/libs/geometry/doc/html/geometry/reference/spatial_indexes/boost__geometry__index__rtree.html). There are two R-trees used, one for Buildings and one for all other objects combined. The reason for this is the two vertex radius settings which can differ, [radius_vertex_elevation]({{site.baseurl}}/output_options.html#radius_vertex_elevation) and [building_radius_vertex_elevation]({{site.baseurl}}/output_options.html#building_radius_vertex_elevation).

The bounding box of all polygons is calculated from the two R-trees combined plus the vertex radius. Since the R-trees contain the bounding box of the object geometries the total bounding box can be larger then the actual area.

{% include imagezoom.html file="flows/3dfier_reading_polygons.png" alt="Flow diagram polygon reading" %}

### Reading points
The elevation information is read from LAS or LAZ files. The file is opened and the header is read. The bounding box from the header is intersected with the bounding box of all polygons. When the file doesn't overlap it is skipped. When the bounds overlap the point count is logged together with thinning setting if configured. For each point read the following things are checked before going into adding a point to a TopoFeature:
- is point to be used according to *i % thinning == 0*
- is classification not within the list of *omit_LAS_classes*
- is point within the bounding box of the polygons

When all checks pass the points is added to the map. The next step is to query both R-trees and return the objects that intersect with this point plus vertex radius in all directions. Each returned object is iterated and the point classification is checked based on the object types. The classification allowed refers to [use_LAS_classes]({{site.baseurl}}/lifting_options.html#use_las_classes) for each object class type configured in the [Lifting options]({{site.baseurl}}/lifting_options.html#). When the point with a certain class needs to be within the polygon, as configured in [use_LAS_classes_within]({{site.baseurl}}/lifting_options.html#use_las_classes_within), an additional boolean is set.

The elevation point is added to the object if its classification is configured for use. Then followed by a check if the point needs to be within the polygon, or just within range. The height is then assigned to each vertex within range of the point. The height is multiplied by 100 (for centimetre accuracy), rounded to an integer (to keep the memory footprint as low as possible) and then added to the elevation vector of the vertex.

Each object class has its own additional set of rules for a point to be used.

{% include imagezoom.html file="flows/3dfier_reading_points.png" alt="Flow diagram point reading" %}

#### Terrain and Forest (TIN)
These classes have the [simplification settings]({{site.baseurl}}/lifting_options.html#simplification) that adds points randomly. Also it stores all elevation points that are within the polygon respecting the [innerbuffer setting]({{site.baseurl}}/lifting_options.html#innerbuffer) in a vector. This vector is used to add additional points inside the polygons for a more precise ground model. 

#### Building and Water (Flat)
For these classes there is a difference to the others. Since these features are modelled as flat surfaces the elevation vector is not maintained per vector but for the TopoFeature as a whole. This lowers the memory footprint and calculation time while maintaining the same accuracy. All points within the polygon or within range of the vertices are stored as described in [radius_vertex_elevation]({{site.baseurl}}/output_options.html#radius_vertex_elevation). For buildings there are two elevation vectors, one for the ground and one for the roof.

#### Road, Separation and Bridge (Boundary3D)
These classes do not have any specific rules for assigning elevation information.

### Magic
The magic of the algorithm happens inside the threedfy [3-D-fy] function. This code is what makes the input data to be merged into a 3D model. Yellow boxes are explained in separate flow diagrams in the [Threedfy flow section]({{site.baseurl}}/threedfy_flow.html).

{% include imagezoom.html file="flows/threedfy.png" alt="Flow diagram for threedfy" %}

### Writing model
Besides creation of the data an important part is to write the created model into a data standard applicable for future use. There are several options available from a 3D modelling, computer graphics or statistics point of view. Most code for writing the model to a file is specifically created for that output format. The flow diagram contains part of all available flows. A more detailed description can be found in [Output flows]({{site.baseurl}}/output_flow).
{% include imagezoom.html file="flows/3dfier_writing_model.png" alt="Flow diagram for writing 3D models" %}