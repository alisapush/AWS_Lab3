#ifndef RISCV_SIM_UNCACHEDMEMORY_H
#define RISCV_SIM_UNCACHEDMEMORY_H

#include "IMemory.h"
#include "MemoryStorage.h"

class UncachedMemory : public IMemory
{
public:
	explicit UncachedMemory(MemoryStorage &amem)
			: _mem(amem)
	{

	}

	void Request(Word ip)
	{
		_requestedIp = ip;
		_waitCycles = latency;
	}

	std::__1::optional<Word> Response()

	{
		if (_waitCycles > 0)
			return std::__1::optional<Word>();
		return _mem.Read(_requestedIp);
	}

	void Request(InstructionPtr &instr)
	{
		if (instr->_type != IType::Ld && instr->_type != IType::St)
			return;

		Request(instr->_addr);
	}

	bool Response(InstructionPtr &instr)
	{
		if (instr->_type != IType::Ld && instr->_type != IType::St)
			return true;

		if (_waitCycles != 0)
			return false;

		if (instr->_type == IType::Ld)
			instr->_data = _mem.Read(instr->_addr);
		else if (instr->_type == IType::St)
			_mem.Write(instr->_addr, instr->_data);

		return true;
	}

	void Clock()
	{
		if (_waitCycles > 0)
			--_waitCycles;
	}


private:
	static constexpr size_t latency = 120;
	Word _requestedIp = 0;
	size_t _waitCycles = 0;
	MemoryStorage &_mem;
};

#endif //RISCV_SIM_UNCACHEDMEMORY_H
