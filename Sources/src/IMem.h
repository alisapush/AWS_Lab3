
#ifndef RISCV_SIM_IMEM_H
#define RISCV_SIM_IMEM_H

#include "Memory.h"

class IMem {
public:
    IMem() = default;

    virtual ~IMem() = default;

    IMem(const IMem &) = delete;

    IMem(IMem &&) = delete;

    IMem &operator=(const IMem &) = delete;

    IMem &operator=(IMem &&) = delete;

    virtual void Request(Word ip) = 0;

    virtual std::__1::optional<Word> Response() = 0;

    virtual void Request(InstructionPtr &instr) = 0;

    virtual bool Response(InstructionPtr &instr) = 0;

    virtual void Clock() = 0;
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

#endif //RISCV_SIM_IMEM_H
