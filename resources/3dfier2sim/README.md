
# 3dfier2sim
                                            
Small utility program to convert the output of 3dfier to a mesh that can be used directly (in most cases! we can't guarantee it) as input in [ANSYS](http://www.ansys.com/) software to perform simulation.

Main operations performed:

  1. the model, which is a surface, is closed to form a volume. This is done by expanding the edges of the dataset by roughly 200m and adding vertices at the elevation zero, and then adding new faces to close the volume.
  2. the surface of the model is automatically repaired 
    - by filling holes in it with new surfaces (which are also triangulated)
    - by flipping the orientation of the surfaces so that the model is a valid 2-manifold
  3. the new surfaces added to close the volume are all triangulated.

## Compilation

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ./3dfier2sim


## Usage

To convert the test-dataset of 3dfier (in `/example_data/`):

  1. remove the bridges in the yml file
  2. run 3dfier with OBJ output, let us call it `testarea.obj`
  3. it's a good idea to translate the area to (minx, miny) since the Dutch coordinates are rather large and can affect precision, use `/resources/translate2min-obj.py' and create the file `testarea_t.obj`
  4. convert that file to an [OFF file](https://en.wikipedia.org/wiki/OFF_(file_format)), eg `testarea.off` in our case
  5. `3dfier2sim testarea.off` will give a summary of the operations done and output a file `out.off`
  6. voilà, you should be able to import this file without spending too much time (semi-)manually fixing the errors.


