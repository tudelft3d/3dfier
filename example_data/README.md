
## To run:

`$ ./3dfier testarea_config.yml > out/myoutput.obj`


## 2D input data: the BGT

The files in the folder `bgt` are a crop of the [BGT datasets](http://www.kadaster.nl/web/Themas/Registraties/BGT.htm) in Delft. There were created with the script in `ressources/BGT_prepare/..` 

## LIDAR point cloud input: AHN3

The 2 files in the folder `ahn3` are part of the [AHN3](https://www.pdok.nl/nl/ahn3-downloads) cropped for the same area. The 2 tiles used are `c_37en1.laz` and `c_37en2.laz`.

## Example output 

What you should get when you run 3dfier is in `output/testarea.obj`.
If you use [MeshLab](http://meshlab.sourceforge.net) to view the file, the colours for each class can be activated in the menu `Render/Color/Per Face`. It looks like that:

![](https://www.dropbox.com/s/zjkqtoo1zyhn25p/Screenshot%202016-09-29%2011.43.39.png?dl=0)