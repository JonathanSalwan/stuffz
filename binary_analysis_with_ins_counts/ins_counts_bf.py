#!/usr/bin/env python2
## -*- coding: utf-8 -*-
##
##  Copyright (C) 2013 - Jonathan Salwan - http://twitter.com/JonathanSalwan
## 
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
## 
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
## 
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

import sys
import commands

if __name__ == "__main__":
    
    if len(sys.argv) != 4:
        print "%s <binary> <size password> <sensitive>"
        sys.exit(-1)

    pwd  = "_" * int(sys.argv[2])
    base = 0x2e
    off  = 0x00
    sav  = 0x00

    while pwd.find('Good Password') == -1:
        pwd = pwd[:off] + chr(base) + pwd[off+1:];
        cmd = "../pin -t ./inscount0.so -- %s '%s'; cat inscount.out" %(sys.argv[1], pwd)
        res = int(commands.getstatusoutput(cmd)[1].split("Count")[1])
        print "insert('%s') = %d ins" %(pwd, res)
        if sav == 0x00:
            sav = res
        if res - sav > int(sys.argv[3]):
            off += 1
            if off >= len(pwd):
                break
            base = 0x2d
            sav = 0
        base += 1
        sav = res

    print "The password is %s" %(pwd)
    sys.exit(0)

