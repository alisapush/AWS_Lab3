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


static constexpr size_t memSize = 1024 * 1024;

static constexpr size_t line_size_bytes = 128;
static constexpr size_t lineSizeWords = line_size_bytes / sizeof(Word);
using Line = std::array<Word, lineSizeWords>;
using TagClockPair = std::pair<size_t, clock_t>;
static constexpr size_t dataCacheBytes = 2048;
static constexpr size_t codeCacheBytes = 1024;
static Word ToWordAddr(Word addr)
{ return addr >> 2u; }

static Word to_line_addr(Word addr)
{ return addr & ~(line_size_bytes - 1); }

static Word to_line_offset(Word addr)
{ return ToWordAddr(addr) & (lineSizeWords - 1); }

#endif //RISCV_SIM_DATAMEMORY_H
