When preparing the BGT GML files as input for 3dfier, the BGT_conversion.bat batch script can be used. 
The script requires GDAL >2.0 to perform the filtering and conversion.
OGR is used to select only to use polygons and stroke CurvePolygons to discretized polygons., it also filters the history of objects by only selecting those of which the 'eindregistratie' is not set.

Usage:
- Extract the BGT GML files in a folder
- Copy all GFS files from "BGT gfs files" into the same folder
- Make sure all GFS files are newer then the GML files (assure this by editing all GFS files, add/remove a character, save, restore character, save)
- Run BGT_conversion.bat (or the commands within if on Linux/Mac)
- Use generated sqlite files as input to 3dfier