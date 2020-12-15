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
    orcid: 0000-0002-6229-7749
    affiliation: 2
  - name: Balázs Dukai
    affiliation: 1
  - name: Kavisha Kumar
    affiliation: 1
  - name: Ravi Peters
    affiliation: 1
  - name: Jantien Stoter    
    affiliation: 1
  - name: Tom Commandeur
    affiliation: 1
affiliations:
  - name: Delft University of Technology, the Netherlands
    index: 1
  - name: National University of Singapore, Singapore
    index: 2
date: 16 October 2020
bibliography: paper.bib

# Optional fields if submitting to a AAS journal too, see this blog post:
# https://blog.joss.theoj.org/2018/12/a-new-collaboration-with-aas-publishing
# aas-doi: 10.3847/xxxxx <- update this with the DOI from AAS once you know it.
# aas-journal: Astrophysical Journal <- The name of the AAS journal.
---

# Summary

Three-dimensional city models are essential to assess the impact that environmental factors will have on citizens, because they are the input to several simulation and prediction software.
Examples of such environmental factors are noise [@Stoter08], wind [@GarciaSanchez14], air pollution [@Ujang13], and temperature [@Lee13; @Hsieh11].

However, those 3D models, which typically contain buildings and other man-made objects such as roads, overpasses, bridges, and trees, are in practice complex to obtain, and it is very time-consuming and tedious to reconstruct them manually.

The software *3dfier* addresses this issue by automating the 3D reconstruction process.
It takes 2D geographical datasets (eg topographic datasets) and "3dfies" them (as in "making them three-dimensional"). 
The elevation is obtained from an aerial point cloud dataset, and the semantics of every polygon is used to perform the lifting to the third dimension, so that it is realistic.
The resulting surface aims at being error-free: no self-intersections, no gaps, etc.
Several output formats are supported (including the international standards), and the 3D city models are optimised for used in different software 



# Statement of need

The 3D city models needed as input in environmental simulations have specific requirements that go beyond the typical 3D models used for visualisation: they require semantic information (ie an object, modelled with one or more surfaces, "knows" what it is, for instance a window or a roof surface) and they should be free of geometric errors.
It is known that practitioners and researchers wanting to perform some simulations or analysis can spend a significant part of their time constructing and repairing the input 3D models; @McKenney98 estimates this to as much as 70\% of their time.
Furthermore, the formats required by the different software and/or the agencies (for instance [the international standard CityGML](https://www.ogc.org/standards/citygml)), are complex to generate [@Ledoux19].

The software *3dfier* automates the reconstruction step, it enriches the data with semantics, and it supports several output formats (used in different fields).

It builds upon previous work done for reconstructing the whole country of the Netherlands (with 10M+ buildings) [@OudeElberink13], and provides the following improvements: use of recent and maintained libraries (eg [CGAL](https://www.cgal.org/), [GDAL](https://gdal.org/) and [Boost](https://www.boost.org/)), a clear open-source license, recent formats and international standards are supported, no geometric errors in output.


# Overview of the reconstruction steps

![Overview of 3dfier.\label{fig:overview}](extrusion.png){ width=90% }

As shown in \autoref{fig:overview}, as input we use geographical datasets that are readily available for an area (often as open data too):

  1. 2D polygons representing buildings, lakes, roads, parts, etc. ([OpenStreetMap](https://www.openstreetmap.org) is one option);
  2. elevation points, usually acquired with a laser-scanner and available in [LAS format](https://www.asprs.org/wp-content/uploads/2010/12/LAS_1_4_r13.pdf), or derived from aerial images.

Each of the classes in the input 2D polygons is mapped to a specific class: *Terrain*, *Forest*, *Water*, *Road*, *Building*, *Bridge/Overpass*, and *Separation* (walls and fences).

![1D visualisation of the reconstruction process.\label{fig:steps}](steps.pdf)

The semantics of every input 2D polygon is used to perform the lifting to the third dimension.
For example, water polygons are extruded to horizontal polygons, buildings to prismatic blocks, roads as smooth surfaces, etc. 
Every polygon is triangulated and in a next step the lifted polygons are "stitched" together so that one  surface is reconstructed. 
In this step, priority is given to "hard" objects such as roads, ie vegetation polygons are moved to be aligned with the road polygons.
The output of the software is one watertight surface with no intersecting triangles and no holes, and features such as buildings and trees can be added or omitted.  
Triangles are grouped and labelled with the class and the attributes that were in the input 2D polygons they decompose.
This surface can be used directly as input in several urban applications, such as simulations of noise or wind.


# Use of the software

The software is a command-line interface (CLI), and uses a configuration file as input (a [YAML file](https://yaml.org/)).
This file allows the user to control the mapping between the input and the *3dfier* classes, to specify which [LAS](https://www.asprs.org/wp-content/uploads/2010/12/LAS_1_4_r13.pdf) classes to use/ignore, to control how the lifting is performed for the different classes, etc.

The software, being modular, is also extensible for other use cases or for use in different countries.
As an example, new topographic classes (for instance trees) could be added by simply creating a new C++ class that inherits from the parent class, and the output for the different formats supported must be added.

Great care was taken to keep the software as efficient as possible and make it suitable for reconstructing very large areas. For instance *3dfier* is the software that enables the Dutch national mapping agency (Kadaster) to create the [3D base registration for the Netherlands](https://www.pdok.nl/3d-basisvoorziening).

![An example of the output of 3dfier, for the city of Leiden in the Netherlands.\label{fig:results}](results.png)


# Acknowlegements

This work was funded by the [Netherlands Kadaster](https://www.kadaster.nl/) and received funding from the European Research Council (ERC) under the European Unions Horizon2020 Research & Innovation Programme (grant agreement no. 677312 UMnD: Urban modelling in higher dimensions).

# References

