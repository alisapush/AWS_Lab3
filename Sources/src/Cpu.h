#ifndef RISCV_SIM_CPU_H
#define RISCV_SIM_CPU_H

#include "Memory/MemoryConfig.h"
#include "Decoder.h"
#include "RegisterFile.h"
#include "CsrFile.h"
#include "Executor.h"
#include "Memory/CachedMemory.h"

class Cpu
{
public:
	Cpu(CachedMemory &mem)
			: _mem(mem)
	{
		_status = Status::Ready;
	}

	void Ready()
	{
		_mem.Request(_ip);
	}

	bool Fetching()
	{
		_instruction_data = _mem.Response();
		if (_instruction_data == std::optional<Word>())
		{
			_status = Status::Load;
			return true;
		}
		_instruction = _decoder.Decode(*_instruction_data);
		_rf.Read(_instruction);
		_csrf.Read(_instruction);
		_exe.Execute(_instruction, _ip);
		_mem.Request(_instruction);

		return false;
	}

	bool Executing()
	{
		if (!_mem.Response(_instruction))
		{
			_status = Status::Process;
			return true;
		}
		_rf.Write(_instruction);
		_csrf.Write(_instruction);
		_csrf.InstructionExecuted();
		_ip = _instruction->_nextIp;
		_status = Status::Ready;

		return false;
	}

	void Clock()
	{
		_csrf.Clock();

		if (_status == Status::Ready)
		{
			Ready();
		}

		if (_status != Status::Process)
		{
			if (Fetching())
			{
				return;
			}
		}

		Executing();
	}

	void Reset(Word ip)
	{
		_csrf.Reset();
		_ip = ip;
	}

	std::optional<CpuToHostData> GetMessage()
	{
		return _csrf.GetMessage();
	}

private:
	Reg32 _ip;
	Decoder _decoder;
	RegisterFile _rf;
	CsrFile _csrf;
	Executor _exe;
	CachedMemory &_mem;

	InstructionPtr _instruction;
	std::optional<Word> _instruction_data;
	enum class Status
	{
		Ready,
		Load,
		Process
	};
	Status _status;
};

#endif //RISCV_SIM_CPU_H
