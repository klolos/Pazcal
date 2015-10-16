#!/usr/bin/env python

from subprocess import Popen, PIPE, STDOUT
import os, sys

# compile the assembler
p = Popen("make", shell=True, stdout=PIPE, stderr=STDOUT)
output = p.communicate()[0].rstrip()
if p.returncode != 0: # check if it compiled
    print "make: %s" % str(output)
    print "compilation failed, exiting."
    exit()

# reassemble the libraries
libs = "lib/*.o"
for f in os.listdir("lib"):
    # get the filename
    namelist = f.split(".")
    if len(namelist) != 2 or namelist[1] != "s":
        continue
    name = namelist[0]
    
    # assemble it with as
    p = Popen("as lib/"+name+".s -o lib/"+name+".o --32", \
              shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip()
    if p.returncode != 0: # check if assembling failed
        print "assembler: %s.s: %s" % (name, str(output))
        print "library assembling failed, exiting."
        exit()

# create the temp directory
tmp = "tmp"
if not os.path.exists(tmp):
    os.makedirs(tmp)

# check if an optimizations flag was given
if (len(sys.argv) > 2):
    print "Usage: %s [-o]" % sys.argv[0]
if (len(sys.argv) == 2 and sys.argv[1] == "-o"):
    opt_flag = "o"
else:
    opt_flag = ""

# run all the tests!
succeeded = 0
total = 0
for f in os.listdir("tests"):
    # check that the file has a .pz extension and get its name
    namelist = f.split(".")
    if len(namelist) != 2 or namelist[1] != "pz":
        continue
    name = namelist[0]
    total += 1

    # compile the file to assembly code
    p = Popen("./pazcal -f"+opt_flag+" < tests/"+name+".pz > "+tmp+"/"+name+".s", \
              shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip()
    if p.returncode != 0: # check if compilation failed
        print "compiler: %s.pz: %s" % (name, str(output))
        continue

    # assemble the compiled program
    tmpname = tmp + "/" + name
    p = Popen("as "+tmpname+".s -o "+tmpname+".o --32", \
              shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip()
    if p.returncode != 0: # check if assembling failed
        print "assembler: %s.pz: %s" % (name, str(output))
        continue

    # link the assembled program
    p = Popen("ld -m elf_i386 "+tmpname+".o "+libs+" -o "+tmpname, \
              shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip()
    if p.returncode != 0: # check if linking failed
        print "linker: %s.pz: %s" % (name, str(output))
        continue

    # run the program and get the output
    if os.path.isfile("tests/%s.in" % name):
        p = Popen(tmpname+" < tests/"+name+".in", shell=True, stdout=PIPE, stderr=STDOUT)
    else:
        p = Popen(tmpname, shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip().strip()

    # read the correct result if it exists, otherwise assume it should print nothing
    if os.path.isfile("tests/"+name+".out"):
        correct = open("tests/"+name+".out").read().rstrip()
    else:
        correct = ""

    # the program has to exit with code zero and output the correct result
    if p.returncode != 0:
        print "execution: %s.pz: exited with code %d" % (name, p.returncode)
        continue
    if output != correct:
        print "execution: %s.pz: output = %s, correct = %s" % (name, output, correct)
        continue
    
    succeeded += 1

# Run the failtests and verify they did not compile
faildir = "tests/failtests/"
for f in os.listdir(faildir):
    # check that the file has a .pz extension and get its name
    namelist = f.split(".")
    if len(namelist) != 2 or namelist[1] != "pz":
        continue
    name = namelist[0]
    total += 1

    # try to compile it
    p = Popen("./pazcal -f < "+faildir+name+".pz > /dev/null", \
              shell=True, stdout=PIPE, stderr=STDOUT)
    output = p.communicate()[0].rstrip()
    if not ("Error" in output or "error" in output): # compilation is supposed to fail
        print "compiler: %s.pz: compilation succeeded (it shouldn't!)" % name
        continue

    succeeded += 1


print "%d/%d test cases passed!" % (succeeded, total)


