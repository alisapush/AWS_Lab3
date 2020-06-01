//
// Created by Alisa Pushnova on 5/14/20.
//

#ifndef RISCV_SIM_CACHEDMEMORY_H
#define RISCV_SIM_CACHEDMEMORY_H

#include "IMemory.h"
#include "MemoryStorage.h"

class CachedMemory : public IMemory
{
public:
	explicit CachedMemory(MemoryStorage &amem): _mem(amem) { }

	void Request(Word ip)
	{
		_tag = to_line_addr(ip) / line_size_bytes;                      // evaluating _tag value
		_cached = false;
		_incomplete_iterations_count = _latency;
		for (auto &iter : _code_cache)
		{
			if (iter.first == _tag)
			{
				// if corresponding tag is found
				_line = iter.second;                                // get whole line from cache
				_cached = true;                                     // set cached to true
				_incomplete_iterations_count = 0;                                    // instructions are fetched from cache immediately
				break;
			}
		}
		_requested_address = ip;
	}

	std::__1::optional<Word> Response()
	{
		if (_incomplete_iterations_count > 0)
		{
			return std::__1::optional<Word>();
		}

		if (_cached)
		{
			size_t offset = to_line_offset(_requested_address);
			_cached_code_map[_tag] = clock();
			return _line[offset];
		}

		std::__1::optional<Word> response = _mem.Read(_requested_address);

		Line new_line = Line();
		_requested_address = to_line_addr(_requested_address);
		for (size_t i = 0; i < lineSizeWords; i++)
		{
			new_line[i] = _mem.Read(_requested_address);
			_requested_address += 4;
		}

		auto new_record = std::make_pair(_tag, new_line);
		if (_code_cache.size() >= codeCacheBytes / line_size_bytes)
		{
			auto min = *std::min_element(
					_cached_code_map.begin(),
					_cached_code_map.end(),
					CompareSecond());

			_data_cache.erase(min.first);
			_cached_code_map.erase(min.first);
		}
		_code_cache.push_back(new_record);
		_cached_code_map.insert({_tag, clock()});
		return response;
	}

	void Request(InstructionPtr &instr)
	{
		if (instr->_type != IType::Ld && instr->_type != IType::St)
			return;

		_tag = to_line_addr(instr->_addr) / line_size_bytes;
		_cached = false;
		_incomplete_iterations_count = _latency;
		if (_data_cache.find(_tag) != _data_cache.end())
		{
			_line = _data_cache[_tag].first;
			_cached = true;
			_incomplete_iterations_count = 3;
		}
		_requested_address = instr->_addr;
	}

	bool Response(InstructionPtr &instr)
	{
		if (instr->_type != IType::Ld && instr->_type != IType::St)
		{
			return true;
		}

		if (_incomplete_iterations_count > 0)
		{
			return false;
		}

		if (!_cached)
		{
			SaveInCache();
		}

		_cached_data_map[_tag] = clock();

		if (instr->_type == IType::Ld)
		{
			instr->_data = _data_cache[_tag].first[to_line_offset(_requested_address)];
		}
		else
		{
			_data_cache[_tag].first[to_line_offset(_requested_address)] = instr->_data;
			_data_cache[_tag].second = false;
		}

		return true;
	}

	void SaveInCache()
	{
		Line new_line = Line();
		Word line_begin = to_line_addr(_requested_address);
		for (size_t i = 0; i < lineSizeWords; i++)
		{
			new_line[i] = _mem.Read(line_begin);
			line_begin += 4;
		}

		std::__1::pair<Line, bool> new_record = std::make_pair(new_line, true);
		if (_data_cache.size() >= dataCacheBytes / line_size_bytes)
		{
			CleanCache();
		}

		_data_cache.insert({_tag, new_record});
		_cached_data_map.insert({_tag, clock()});
	}

	void CleanCache()
	{
		auto min = std::min_element(this->_cached_data_map.begin(), this->_cached_data_map.end(),
		                            CompareSecond());
		auto tag_of_min = min->first;

		if (!this->_data_cache[tag_of_min].second)
		{
			size_t ip = tag_of_min * line_size_bytes;
			Line line = this->_data_cache[tag_of_min].first;
			for (auto iter = line.begin(); iter != line.end(); iter++)
			{
				this->_mem.Write(ip, *iter);
				ip += 4;
			}
		}

		this->_data_cache.erase(tag_of_min);
		this->_cached_data_map.erase(tag_of_min);
	}

	void Clock()
	{
		if (_incomplete_iterations_count > 0)
			--_incomplete_iterations_count;
	}

private:
	struct CompareSecond
	{
		bool operator()(const TagClockPair &left, const TagClockPair &right) const
		{
			return left.second < right.second;
		}
	};

	std::__1::map<size_t, clock_t> _cached_code_map;
	std::__1::map<size_t, clock_t> _cached_data_map;
	static constexpr size_t _latency = 152;
	Word _requested_address = 0;
	size_t _incomplete_iterations_count = 0;

	size_t _tag;
	Line _line;
	MemoryStorage &_mem;
	std::__1::vector<std::__1::pair<size_t, Line>> _code_cache;
	std::__1::map<size_t, std::__1::pair<Line, bool>> _data_cache;
	bool _cached = false;
};

#endif //RISCV_SIM_CACHEDMEMORY_H
