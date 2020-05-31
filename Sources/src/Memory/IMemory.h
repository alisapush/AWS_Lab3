
#ifndef RISCV_SIM_IMEMORY_H
#define RISCV_SIM_IMEMORY_H
#include "MemoryConfig.h"

class IMemory
{
public:
	IMemory() = default;

	virtual ~IMemory() = default;

	IMemory(const IMemory &) = delete;

	IMemory(IMemory &&) = delete;

	IMemory &operator=(const IMemory &) = delete;

	IMemory &operator=(IMemory &&) = delete;

	virtual void Request(Word ip) = 0;

	virtual std::__1::optional<Word> Response() = 0;

	virtual void Request(InstructionPtr &instr) = 0;

	virtual bool Response(InstructionPtr &instr) = 0;

	virtual void Clock() = 0;
};


#endif //RISCV_SIM_IMEMORY_H
