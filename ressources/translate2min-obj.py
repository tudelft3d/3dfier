
import sys
import os

fin = open(sys.argv[1])

#-- read vertices
lsPts = []
for l in fin:
  if l[0] == 'v':
    lsPts.append(l)

minx = 1e9
miny = 1e9
for l in lsPts:
  v = l.split()
  if float(v[1]) < minx:
    minx = float(v[1])
  if float(v[2]) < miny:
    miny = float(v[2])
  
for each in lsPts:
  v = each.split()
  print "v " + str(float(v[1]) - minx) + " " + str(float(v[2]) - miny) + " " + v[3]
fin.seek(0)
for l in fin:
  if l[0] != 'v':
    print l[:-1]


