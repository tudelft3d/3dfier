---
title: Main data flow
keywords: main flow
sidebar: 3dfier_sidebar
permalink: main_flow.html
---

## Guide for reading
How is the architecture of 3dfier described and how would you read it.

## Data flow through the program
Explain different steps in the flow diagram. Show what data goes were and link to following flow diagrams.
{% include imagezoom.html file="3dfier_general_flow.png" alt="General flow diagram 3dfier" %}

### Configuration
Reading and validation of the configuration is done using the [YAML-CPP library](https://github.com/jbeder/yaml-cpp). It parses the file into an object and allows for easy access to all values. The configuration file is read and validated first. Value types are tested and options with limited options are verified. If validation fails an error message will be printed and the program execution will stop.

When validation passes, all values are used to set the corresponding parameters in the software. After this initial read the configuration file is not used anymore since all values are now stored in memory.

### Data integrity check
For all input files configured to use a check is done if they exist and readable. The polygon files are opened using an OGR reader and closed directly after. Same is done for the LAS/LAZ files, they are opened with LASlib and directly closed. This prevents failure of the program after partly executing. Also the output filepaths are tested to be existing before execution of the algorithm.

*NOTE: Output folder must exist, it's not created automatically*

### Reading polygons
If all sanity checks are passed the program turns to reading the polygon files. It takes into account the extent setting, only polygons which bounding box intersects with the given extent will be added. 

A file is opened for reading and the id and height_field attributes are loaded. If these attributes do not exist the program will stop execution with an exception. The total number of features in the layer is logged and counters are set for the amount of multipolygons. Next each feature is read, the attributes are stored in a new TopoFeature. A TopoFeature is the internal storage class used by the algorithm to store attributes, 2D and 3D geometries and functionality to lift the objects. The geometry is extracted, depending on the polygon type it is pre-processed. For a multipolygon each separate polygon is added to a new TopoFeature in which the attributes are copied. The id of the feature is altered by adding a trailing dash and counter, e.g. 'id-0, id-1'. Curvepolygons are automatically stoked into small straight line segments. 

*Important: When reading the geometry the boost::geometry library is used to read WKT and both functions boost::geometry::unique and boost::geometry::correct are used for removing duplicate vertices and correcting ring orientation.*

During the creation of a TopoFeature and storing its geometry all used buffers and vectors are resized to correspond to the amount of points in the polygon outer and inner rings. This preallocates most the needed memory for storing the elevation information acquired when reading the point clouds.

### Reading points


### Magic


### Writing model
