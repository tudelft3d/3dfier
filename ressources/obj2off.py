import sys
import StringIO

def main():
  fIn = open(sys.argv[1])
  fOut = convert(fIn)
  print fOut.getvalue()

def convert(fIn):
  fOut = StringIO.StringIO()
  #-- skip the header
  lsVertices = []
  lsFaces = []
  for l in fIn:
    if (len(l) == 0) or (l[0] == '#'):    
      continue
    if (l[0] == 'v'):
      lsVertices.append(l)
    if (l[0] == 'f'):
      lsFaces.append(l)  
  # print len(lsVertices)
  # print len(lsFaces)
  fOut.write("OFF\n" + str(len(lsVertices)) + " " + str(len(lsFaces)) + " 0\n")
  for i in lsVertices:
    v = i.split()
    fOut.write(v[1] + " " + v[2] + " " + v[3] +"\n")
  for i in lsFaces:
    f = i.split()
    le = len(f) - 1
    fOut.write(str(le) + " ")
    for each in f[1:]:
      st = int(each) - 1
      fOut.write(str(st) + " ")
    fOut.write("\n")
  return fOut

if __name__ == "__main__":
    main()  