
from optparse import OptionParser
import yaml
import sys
import subprocess


TMPFOLDER = "/Users/hugo/temp/0000/"
LAS2LAS = "wine /Users/hugo/software/lastools/bin/las2las"
LAS2TXT = "wine /Users/hugo/software/lastools/bin/las2txt"

def main():
    options, args = parse_arguments()
    
    #-- 1. read the LAS/LAZ files from the YAML file
    fin = open(args[0])
    myyaml = yaml.load(fin.read())
    lasfiles = []
    for each in myyaml['input_elevation']:
        for f in each['datasets']:
            lasfiles.append(f)
    print lasfiles

    #-- 2. convert them to txt and thin them by 10 with las2txt
    for i,each in enumerate(lasfiles):
        cmd = LAS2LAS
        cmd += " -keep_every_nth 10"
        cmd += " -keep_class 1"
        cmd += " -i "
        cmd += each
        cmd += " -o"
        cmd += " %s%s.laz" % (TMPFOLDER, i)
        print cmd
        # sys.exit()
        op = subprocess.Popen(cmd.split(' '),
                          stdout=subprocess.PIPE, 
                          stderr=subprocess.PIPE)
        R = op.wait()
        if R:
            res = op.communicate()
            raise ValueError(res[1])
        print "done"
        #-- 3. convert to txt
        cmd = LAS2TXT
        cmd += " -i"
        cmd += " %s%s.laz" % (TMPFOLDER, i)
        cmd += " -o"
        cmd += " %s%s.txt" % (TMPFOLDER, i)
        print cmd
        # sys.exit()
        op = subprocess.Popen(cmd.split(' '),
                          stdout=subprocess.PIPE, 
                          stderr=subprocess.PIPE)
        R = op.wait()
        if R:
            res = op.communicate()
            raise ValueError(res[1])
        print "done"

    #-- 4. take each laz.txt and merge and OFF file
    print "READING TXT FILES AND CREATING OFF FILE"
    # fOut = open(args[1], 'w')
    # fOut.write(COFF)

    thepts = []
    for i in range(len(lasfiles)):
        # print "---", TMPFOLDER, i
        f = open(TMPFOLDER + str(i) + ".txt")
        for l in f:
            thepts.append(l + " 48 206 77 100")
    print len(thepts)    




def parse_arguments():
    usage = "Usage: %prog myconfig.yml myoutput.off"
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("The input yaml file + output off must be specified.")
    return options, args

if __name__ == "__main__":
    main()
