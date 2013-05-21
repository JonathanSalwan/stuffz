#!/usr/bin/env python2
## -*- coding: utf-8 -*-
##
##  By Jonathan Salwan - http://twitter.com/JonathanSalwan
##

import sys

if __name__ == "__main__":
    

    if len(sys.argv) < 3:
        print "Syntax : %s <binary> <opcodes sequence>" %(sys.argv[0])
        sys.exit(1)

    op  = sys.argv[2].decode("hex")
    nop = "\x90" * len(op)

    fd = open(sys.argv[1], "r")
    raw = fd.read()
    fd.close()    

    raw = raw.replace(op, nop)
    
    fd = open(sys.argv[1] + ".patched", "w")
    fd.write(raw)
    fd.close()    

    print "Binary patched"

    sys.exit(0)

