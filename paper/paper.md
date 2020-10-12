---
title: '3dfier: automatic reconstruction of 3D city models'
tags:
  - GIS
  - 3D city modelling
authors:
  - name: Hugo Ledoux^[Corresponding author]
    orcid: 0000-0002-1251-8654
    affiliation: 1 
  - name: Filip Biljecki
    affiliation: 2
  - Name: Balázs Dukai
    affiliation: 1
  - Name: Kavisha Kumar
    affiliation: 1
  - Name: Ravi Peters
    affiliation: 1
  - Name: Jantien Stoter    
    affiliation: 1
  - name: Tom Commandeur
    affiliation: 1
affiliations:
  - name: Delft University of Technology, the Netherlands
    index: 1
  - name: National University of Singapore
    index: 2
date: 12 October 2020
bibliography: paper.bib

# Optional fields if submitting to a AAS journal too, see this blog post:
# https://blog.joss.theoj.org/2018/12/a-new-collaboration-with-aas-publishing
# aas-doi: 10.3847/xxxxx <- update this with the DOI from AAS once you know it.
# aas-journal: Astrophysical Journal <- The name of the AAS journal.
---

# Summary

Three-dimensional city models are essential to assess the impact that environmental factors will have on citizens, because they are the input to several simulation and prediction software.
Examples of such environmental factors are noise [@Stoter08], air pollution[@Ujang13], and temperature [@Lee13; @Hsieh11].

However, those 3D models, which typically contain buildings and other man-made objects such as roads, overpasses, bridges, and trees, are in practice complex to obtain, and it is very time-consuming to reconstruct them manually.

The software *3dfier* helps us automate the 3D reconstruction process.
It takes 2D geographical datasets (eg topographical datasets) and "3dfies" them (as in "making them three-dimensional"). 
The elevation is obtained from an aerial point cloud dataset, and the semantics of every polygon is used to perform the lifting to the third dimension (so that it is realistic).
Several output formats are supported (including the international standards), and the models are optimised for used in different software (so they are error-free: no self-intersections, no gaps, etc.).


# Statement of need

The 3D city models needed as input in environmental simulations have specific requirements that go beyond the typical 3D models used for visualisation: they require semantic information (ie an object, modelled with one or more surfaces, "knows" what it is, for instance a window or a roof surface) and they should be free of geometric errors.
It is known that practitioners and researchers can spend a significant part of their time constructing and repairing the input 3D models, [@McKenney98] estimates as much as 70\% of their time.
Furthermore, the formats required by the different software and/or the agencies (for instance [the international standard CityGML](https://www.ogc.org/standards/citygml)), are arguably complex to generate [@Ledoux19].

The software *3dfier* automates the reconstruction step, can structure enrich the data with semantics, and supports several output formats (used in different fields).

It builds upon previous work done for reconstructing the whole country of the Netherlands (with its 10M+ buildings) [@OudeElberink13], and provides the following improvements: clear open-source license, recent complex formats supported, no geometric errors in output.



# Overview of the reconstruction steps

![Overview of 3dfier.\label{fig:overview}](extrusion.png)

As shown in \autoref{fig:overview}, as input we use geographical datasets that are readily available for an area (often as open data too):

  1. 2D polygons representing buildings, lakes, roads, parts, etc.;
  2. elevation points, usually acquired with a laser-scanner and available in [LAS format](https://www.asprs.org/wp-content/uploads/2010/12/LAS_1_4_r13.pdf), or derived from aerial images.

Each of the classes in the input 2D polygon is mapped to a specific class: *Terrain*, *Forest*, *Water*, *Road*, *Building*, *Bridge/Overpass*, and *Separation* (walls and fences).

![1D visualisation of the reconstruction process.\label{fig:steps}](steps.pdf)

The semantics of every input 2D polygon is used to perform the lifting to third dimension.
For example, water polygons are extruded to horizontal polygons, buildings to prismatic blocks, roads as smooth surfaces, etc. 
Every polygon is triangulated and in a next step the lifted polygons are "stitched" together so that one surface is reconstructed. 
The output of the software is one watertight surface with no intersecting triangles and no holes, and features such as buildings and trees can be added or omitted.  
This surface can be used directly as input in several urban applications, such as simulations.


# Use of the software


The software is a command-line interface (CLI), and uses a configuration as input (a [YAML file](https://yaml.org/)).

New topographic classes (for instance trees could be added) can be added by simply creating a new C++ class that inherits from the parent class, and the output for the different formats supported must be added.



# References