#!/usr/bin/env python2
## -*- coding: utf-8 -*-
##
##  Jonathan Salwan - 2014-01-27
## 
##  http://www.shell-storm.org/
##  http://twitter.com/JonathanSalwan
## 
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software  Foundation, either  version 3 of  the License, or
##  (at your option) any later version.
##
##  Note: Just a simple way to generate valid samples. You can use script before compiling
##  your sources (metaprog) or on raw files.
##
##  =================================
##
##  Meta patterns supported:
##
##  __GENINTDEC#<uniqID>:[<start value>:<end_value>]__  == Generates the same file with all range values in decimal.
##  __GENINTHEX#<uniqID>:[<start value>:<end_value>]__  == Generates the same file with all range values in hexadecimal.
##  __GENSTR#<uniqID>:[<str1>,<str2>,<str3>,...]__      == Generates the same file with all strings listed.
##
##  =================================
##
##  Example 1:
##
##  #include <stdio.h>
##  
##  int main(int ac, const char *av[])
##  {
##    int a, b, c;
##    char *str1, *str2;
##  
##    a = __GENINTDEC#1:[-3:2]__;
##    b = __GENINTDEC#2:[6:10]__;
##    c = 7;
##  
##    str1 = "fixe";
##    str2 = __GENSTR#1:["AAAAAAAAAAAA","%x%x%x%x%x%x%x%x",NULL,"\x00\x01\x02\x03\x04"]__;
##    
##    foo(a, b, c, str1, str2);
##  }
##
##  =================================
##
##  Example 2:
## 
##  #include <sys/socket.h>
##  #include <sys/types.h>
##  
##  int main(int ac, const char *av[])
##  {
##    int sockfd;
##  
##    sockfd = socket(__GENSTR#1:[AF_UNIX,AF_INET,AF_INET6,AF_IPX,AF_NETLINK,AF_X25,AF_AX25,AF_ATMPVC,AF_APPLETALK,AF_PACKET]__, 
##                    __GENSTR#2:[SOCK_STREAM,SOCK_DGRAM,SOCK_SEQPACKET,SOCK_RAW,SOCK_RDM,SOCK_PACKET]__, 
##                    __GENINTDEC#1:[0:10]__);
##    /* ... */
##  }
##
##  =================================
##
##  Example 3:
##
##  60 23 40 00 00 00 00 00 00 00 00 00 00 00 00 00 
##  01 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 
##  0c 00 00 00 00 00 00 00 b0 15 40 00 00 00 00 00 
##  0d 00 00 00 00 00 00 00 6c 53 40 00 00 00 00 00 
##  19 00 00 00 00 00 00 00 f8 7d 60 00 00 00 00 00 
##  1b 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00 
##  1a 00 00 00 00 00 00 00 00 7e 60 00 __GENINTHEX#1:[128:129]__ 00 00 00 
##  1c 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00 
##  04 00 00 00 00 00 00 00 b0 02 40 00 00 00 00 00 
##  f5 fe ff 6f 00 00 00 00 70 05 40 00 00 00 00 00 
##  05 00 00 00 00 00 00 00 18 0c 40 00 00 00 00 00 
##  06 00 00 00 00 00 00 00 d0 05 40 00 00 00 00 00 
##  0a 00 00 __GENINTHEX#2:[44:51]__ 00 00 00 00 a5 02 00 00 00 00 00 00 
##  0b 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00 
##  15 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
##  03 00 00 00 00 00 00 00 00 80 60 00 00 00 00 00 
##  02 00 00 00 00 00 00 00 88 05 00 00 00 00 00 00 
##  14 00 00 00 00 00 00 00 07 00 00 00 00 00 00 00 
##  17 00 00 00 00 00 00 00 28 10 40 00 00 00 00 00 
##  07 00 00 00 00 00 00 00 98 0f 40 00 __GENINTHEX#3:[20:25]__ 00 00 00 
##  08 00 00 00 00 00 00 00 90 00 00 00 00 00 00 00 
##  09 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00 
##  fe ff ff 6f 00 00 00 00 48 0f 40 00 00 00 00 00 
##  ff ff ff 6f 00 00 00 00 01 00 00 00 00 00 00 00 
##  f0 ff ff 6f 00 00 00 00 be 0e 40 00 00 00 00 00 
##  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
##
##  =================================
##
##  Using example:
##
##  $ ./metaSample.py --file src_examples/example_1.c --outputDir ./output -v
##  [+] Opening file: src_examples/example_1.c
##  [+] Finding metadata...
##  [+] Metadata found:
##      -> __GENINTDEC#1:[-3:2]__
##      -> __GENINTDEC#2:[6:10]__
##      -> __GENSTR#1:["AAAAAAAAAAAA","%x%x%x%x%x%x%x%x",NULL,"\x00\x01\x02\x03\x04"]__
##  [+] Generating samples in progress...
##  [+] Number of samples generated: 120
##
##

import getopt
import hashlib
import re
import sys

class metaSample:
 
    def __init__(self):
        self.__fileName     = ""
        self.__outputDir    = "/tmp"
        self.__verbose      = False
        self.__numSampleGen = 0

    """ verbose attribute getter """
    def getVerbose(self):
        return self.__verbose

    """ fileName attribute getter. """
    def getFileName(self):
        return self.__fileName
    
    """ outputDir attribute getter. """
    def getOutputDir(self):
        return self.__outputDir

    """ Print help function. """
    def printHelp(self):
        print "Syntax: metaSample.py [options] (args)\n"
        print "Options:"
        print "\t-h, --help\t\tPrint help"
        print "\t-f, --file\t\tSpecifies an input file"
        print "\t-o, --outputDir\t\tSpecifies an output directory"
        sys.exit(1)

    """ getOpt function. Need sys.argv in parameter. """
    def getOpt(self, argv):
        try:
            opts, args = getopt.getopt(argv, "hfo:v", ["help", "file=", "outputDir="])
        except getopt.GetoptError:
            self.printHelp()

        for opt, arg in opts:
            if opt == "-v":
                self.__verbose = True
            elif opt in ("-h", "--help"): 
                self.printHelp()
            elif opt in ("-f", "--file"):
                self.__fileName = arg
            elif opt in ("-o", "--outputDir"):
                self.__outputDir = arg

    """ This function generate the samples base on the input file. """
    def generate(self):
        if not self.__fileName:
            print "[-] Error: input file not found. Please see --help."
            sys.exit(-1)
        self.__constructMeta()

    def __constructMeta(self):
        if self.getVerbose() == True:
            print "[+] Opening file: %s" %(self.getFileName())

        try:
            f = open(self.getFileName(), "r")
            raw = f.read()
            f.close()
        except:
            print "[-] Error: Can't open or read the file: %s" %(self.getFileName())
            sys.exit(-1)

        if self.getVerbose() == True:
            print "[+] Finding metadata..."
        metadata = re.findall(r'__GEN.*?__', raw)

        if self.getVerbose() == True:
            print "[+] Metadata found:"
            for elem in metadata:
                print "    -> %s" %(elem)
        self.__genFile(raw, metadata)

    def saveGeneratedFile(self, raw):
        m = hashlib.md5()
        m.update(raw)
        path = self.getFileName()
        prefix = path[path.rfind("/")+1:path.rfind(".")]
        extens = path[path.rfind("."):]
        name = self.getOutputDir() + '/' + prefix + '-' + m.hexdigest() + extens
        try:
            f = open(name, "w+")
            f.write(raw)
            f.close()
        except:
            print "[-] Error: Can't write the file: %s" %(name)
            sys.exit(-1)
        self.__numSampleGen += 1

    def __genFile(self, raw, metadata):
        
        if self.getVerbose() == True:
            print "[+] Generating samples in progress..."
    
        generation = []
        for elem in metadata:
            try:
                min = int(elem.split('[')[1].split(':')[0])
                max = int(elem.split('[')[1].split(':')[1].split(']')[0])
                generation += [[elem, min, max]]
            except:
                item = elem.split('[')[1].split(']')[0].split(",")
                generation += [[elem, item]]

        x = 0
        execEval = ""
        for elem in generation:
            if elem[0].find('__GENSTR') == 0:
                execEval += "%sfor i%d in range(len(generation[%d][1])):\n" %('\t' * x, x, x)
            elif elem[0].find('__GENINT') == 0:
                execEval += "%sfor i%d in range(generation[%d][1], generation[%d][2]+1):\n" %('\t' * x, x, x, x)
            x += 1
        execEval += "%sraw2 = raw\n" %('\t' * x)
        y = 0
        for elem in generation:
            if elem[0].find('__GENSTR') == 0:
                execEval += "%sraw2 = raw2.replace(r'%s', generation[%d][1][i%d])\n" %('\t' * x, elem[0], y, y)
            elif elem[0].find('__GENINTDEC') == 0:
                execEval += "%sraw2 = raw2.replace('%s', str(i%d))\n" %('\t' * x, elem[0], y)
            elif elem[0].find('__GENINTHEX') == 0:
                execEval += "%sraw2 = raw2.replace('%s', str(hex(i%d))[2:])\n" %('\t' * x, elem[0], y)
            y += 1
        execEval += "%sself.saveGeneratedFile(raw2)\n" %('\t' * y)
        exec(execEval)

        if self.getVerbose() == True:
            print "[+] Number of samples generated: %d" %(self.__numSampleGen)

if __name__ == '__main__':

    metaSample = metaSample()
    metaSample.getOpt(sys.argv[1:])
    metaSample.generate()

    sys.exit(0)

