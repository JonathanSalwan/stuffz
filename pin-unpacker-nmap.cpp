/*
** Intel Open Source License
**
** Copyright (c) 2002-2012 Intel Corporation. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.  Redistributions
** in binary form must reproduce the above copyright notice, this list of
** conditions and the following disclaimer in the documentation and/or
** other materials provided with the distribution.  Neither the name of
** the Intel Corporation nor the names of its contributors may be used to
** endorse or promote products derived from this software without
** specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
** ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
** END_LEGAL
**
** Note:
**       Checks if mmap is called, saves the address and the size. When
**       addr is called the dump begins.
**
** by Jonathan Salwan - https://twitter.com/JonathanSalwan
**
*/

#include <asm/unistd.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "pin.H"

ofstream trace_ins;

UINT enable       = 0;
UINT base_memory  = 0;
UINT size_memory  = 0;

VOID Instruction(INS ins, VOID *v)
{
    if (enable && INS_Address(ins) >= base_memory && INS_Address(ins) <= base_memory + size_memory){
        trace_ins << "0x" << std::hex << INS_Address(ins) << ": " << INS_Disassemble(ins) << std::endl;
    }
}

VOID Syscall_entry(THREADID thread_id, CONTEXT *ctx, SYSCALL_STANDARD std, void *v)
{
  if (PIN_GetSyscallNumber(ctx, std) == __NR_mmap){
      base_memory = static_cast<UINT>((PIN_GetSyscallArgument(ctx, std, 0)));
      size_memory = static_cast<UINT>((PIN_GetSyscallArgument(ctx, std, 1)));
      enable++;
  }
}

VOID Fini(INT32 code, VOID *v)
{
    trace_ins.close();
}

int main(int argc, char *argv[])
{
    trace_ins.open("instructions.trace");
    
    if (PIN_Init(argc, argv) == -1)
      return -1;

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(Syscall_entry, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    
    return 0;
}
