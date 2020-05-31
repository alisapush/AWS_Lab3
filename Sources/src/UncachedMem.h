//
// Created by Alisa Pushnova on 5/31/20.
//

#ifndef RISCV_SIM_UNCACHEDMEM_H
#define RISCV_SIM_UNCACHEDMEM_H

#include "IMem.h"
#include "MemoryStorage.h"

class UncachedMem : public IMem {
public:
    explicit UncachedMem(MemoryStorage &amem)
            : _mem(amem) {

    }

    void Request(Word ip)                                   // sending request to fetch word
    {
        _requestedIp = ip;
        _waitCycles = latency;
    }

    std::__1::optional<Word> Response()
    // fetching word from requested ip - used for fetching instructions
    {
        if (_waitCycles > 0)
            return std::__1::optional<Word>();
        return _mem.Read(_requestedIp);
    }

    void Request(InstructionPtr &instr)
    // if instruction is load/store type - set requested_ip to instr->addr from instruction
    {
        if (instr->_type != IType::Ld && instr->_type != IType::St)
            return;

        Request(instr->_addr);
    }

    bool Response(InstructionPtr &instr)
    // checks whether the instruction is executed yet or not
    {
        if (instr->_type != IType::Ld && instr->_type != IType::St)
            // if instruction type is not load or store - it's executed immediately,
            // thus instruction is executed already
            return true;

        if (_waitCycles != 0)
            // if there are cycles to wait for - instruction is not executed yet
            return false;

        if (instr->_type == IType::Ld)
            // perform necessary operations and return true
            instr->_data = _mem.Read(instr->_addr);
        else if (instr->_type == IType::St)
            _mem.Write(instr->_addr, instr->_data);

        return true;
    }

    void Clock() {
        if (_waitCycles > 0)
            --_waitCycles;
    }


private:
    static constexpr size_t latency = 120;
    Word _requestedIp = 0;
    size_t _waitCycles = 0;
    MemoryStorage &_mem;
};

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cassert>
#include <map>
#include <queue>
#include <time.h>
#include <algorithm>
#include <array>
#include "Instruction.h"
#include "elf.h"
#include "IMem.h"

#endif //RISCV_SIM_UNCACHEDMEM_H
