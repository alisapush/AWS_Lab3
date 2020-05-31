#ifndef RISCV_SIM_DATAMEMORY_H
#define RISCV_SIM_DATAMEMORY_H

#include "../Instruction.h"
#include <iostream>
#include <fstream>
#include "../elf.h"
#include <cstring>
#include <vector>
#include <cassert>
#include <map>
#include <queue>
#include <time.h>
#include <algorithm>
#include <array>

//static constexpr size_t memSize = 4*1024*1024; // memory size in 4-byte words
static constexpr size_t memSize = 1024 * 1024; // memory size in 4-byte words

static constexpr size_t lineSizeBytes = 128;
static constexpr size_t lineSizeWords = lineSizeBytes / sizeof(Word);
using Line = std::array<Word, lineSizeWords>;
using TagClockPair = std::pair<size_t, clock_t>;
static constexpr size_t dataCacheBytes = 2048; // data cache size in bytes
static constexpr size_t codeCacheBytes = 1024; // instructions cache size in bytes
static Word ToWordAddr(Word addr)
{ return addr >> 2u; }

static Word ToLineAddr(Word addr)
{ return addr & ~(lineSizeBytes - 1); }

static Word ToLineOffset(Word addr)
{ return ToWordAddr(addr) & (lineSizeWords - 1); }


#endif //RISCV_SIM_DATAMEMORY_H
