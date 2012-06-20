#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
#    vtrace-dump_memory.py - Dump memory from a breakpoint.
#
#    Syntax : ./vtrace-dump_memory.py <binary> <addr - breakpoint> <memory addr> <size dump>
#    Exemple: ./vtrace-dump_memory.py ./binary 0x8048438 0x08069322 256
#
#    Copyright (C) 2012-06 Jonathan Salwan - http://www.twitter.com/jonathansalwan
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import vtrace
import vdb
import sys
import envi
import struct
from envi.archs.i386 import *

def dump_memory(trace, memory, size):
   print "[+] Breakpoint at 0x%08x" %(trace.getRegister(REG_EIP))
   try:
      dump  = trace.readMemory(memory, size);
   except:
      print "[EE] Impossible to read from  0x%08x" %(memory)
      return
   try:
      fd = open("vtrace-memory.dump", "w")
      fd.write(dump)
      fd.close()
      print "[+] Dump successful"
   except:
      print "[EE] Impossible de save the dump file"
   return

def main(binary, breakpoint, memory, size):
   trace = vtrace.getTrace()
   try:
      trace.execute(binary)
   except:
      print "[EE] No such file"
   try:
      trace.addBreakByAddr(breakpoint)
   except:
      print "[EE] Invalide addr %s" %(hex(breakpoint))
      return 
   trace.run()
   dump_memory(trace, memory, size)
   return (0)
   
if __name__ == "__main__":
   if len(sys.argv) == 5:
      sys.exit(main(sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16), int(sys.argv[4])))
   else:
      print "Usage: %s <binary> <addr - breakpoint> <memory addr> <size dump>" %(sys.argv[0])
      sys.exit(-1)

