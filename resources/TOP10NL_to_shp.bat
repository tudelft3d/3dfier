echo off
set outputdir=%1
set inputfile=%2
ogr2ogr -spat 84488 446576 85454 447500 -append -gt 65536 -where "OGR_GEOMETRY='Polygon'" %outputdir% %inputfile%