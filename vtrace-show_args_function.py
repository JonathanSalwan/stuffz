#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
#    vtrace-show_args_function.py - Script for vtrace API for display arguments function before call.
#
#    Syntax : ./show_stack_arg.py <binary> <addr call function> <Numbers of arg>
#    Exemple: ./show_stack_arg.py ./binary 0x8048438 2
#
#    Copyright (C) 2012-06-19 Jonathan Salwan - http://www.twitter.com/jonathansalwan
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

def print_stack(trace, arg_number):
   n = 0
   esp = trace.getRegister(REG_ESP)
   print "[+] Breakpoint at 0x%08x" %(trace.getRegister(REG_EIP))
   while n < arg_number:
      try:
         arg  = trace.readMemory((esp+(n*4)), 4);
      except:
         print "[EE] Invalide ESP pointeur (0x%08x)" %(esp)
         return
      arg  = struct.unpack("<I", arg)[0]
      try:
         arg  = trace.readMemory(arg, 256)
         print "[+] Arg [ESP+0x%x]\t: '%s'" %((n*4), arg.split('\0')[0])
      except:
         print "[+] Arg [ESP+0x%x]\t: %d (%x)" %((n*4), arg, arg)
      n = n + 1
   return

def main(binary, breakpoint, arg_number):
   trace = vtrace.getTrace()
   try:
      trace.execute(binary)
   except:
      print "[EE] No such file"
   try:
      bp_strcmp = trace.addBreakByAddr(breakpoint)
   except:
      print "[EE] Invalide addr %s" %(hex(breakpoint))
      return 
   trace.run()
   print_stack(trace, arg_number)
   return (0)
   
if __name__ == "__main__":
   if len(sys.argv) == 4:
      sys.exit(main(sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3])))
   else:
      print "Usage: %s <binary> <addr call function> <Numbers of arg>" %(sys.argv[0])
      sys.exit(-1)

