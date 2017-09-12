
import sys
import os
import subprocess

meshlabserver = '/Applications/meshlab.app/Contents/MacOS/meshlabserver'
fin = open(sys.argv[1])

classes = ['Building', 'Terrain', 'Road', 'Water', 'Forest', 'Separation', 'Bridge']
fouts = []
for each in classes:
  fout = open(each+'.obj', 'w')
  fout.write('mtllib ./3dfier.mtl\n')
  fouts.append(fout)

lsPts = []
fBu = []

#-- process vertices
for l in fin:
  if l[0] == 'v':
    lsPts.append(l)
for f in fouts:
  for each in lsPts:
    f.write(each)

#-- process faces
fin.seek(0)
curc = -1
curo = ""
for l in fin:
  s = l.split()
  if len(s) == 0:
    continue
  if s[0] == 'f':
    fouts[curc].write(l)
  if s[0] == 'o':
    a = l.split()
    curo = a[1]
  if s[0] == 'usemtl':
    curc = classes.index(s[1])
    fouts[curc].write(curo)
    fouts[curc].write('\n')
    fouts[curc].write(l)

for f in fouts:
  f.close()

#-- clean the vertices with meshlabserver
for i,f in enumerate(fouts):
  s = meshlabserver
  s += " -i "
  s += classes[i]
  s += ".obj -s cleanvertices.mlx -o "
  s += classes[i]
  s += "_1.obj" 
  # print s
  os.system(s)

for each in classes:
  f = open(each + '_1.obj', 'r+')
  lines = f.readlines()
  f.seek(0)
  f.write("mtllib ./3dfier.mtl\n")
  f.write("usemtl ")
  f.write(each)
  f.write("\n")
  f.writelines(lines)
  f.close()

for each in classes:
  os.remove(each + '.obj')