
#ifndef RISCV_SIM_CACHEDMEMORY_H
#define RISCV_SIM_CACHEDMEMORY_H

#include "IMemory.h"
#include "MemoryStorage.h"

class CachedMemory : public IMemory {
public:
    explicit CachedMemory(MemoryStorage &amem)
            : _mem(amem) {

    }

    void Request(Word ip) {
        _tag = ToLineAddr(ip) / lineSizeBytes;                      // evaluating _tag value
        _cached = false;
        _waitCycles = _latency;
        for (auto &iter : _code_cache) {
            if (iter.first == _tag) {                               // if corresponding tag is found
                _line = iter.second;                                // get whole line from cache
                _cached = true;                                     // set cached to true
                _waitCycles = 0;                                    // instructions are fetched from cache immediately
                break;
            }
        }
        _requestedIp = ip;
    }

    std::__1::optional<Word> Response()                                  // fetching instruction
    {
        if (_waitCycles > 0)                                        // if not ready yet - return empty instruction
            return std::__1::optional<Word>();

// if instr is cached - return requested value from cache
        if (_cached) {
            size_t offset = ToLineOffset(_requestedIp);
            _code_queue[_tag] = clock();
            return _line[offset];
        }

        // if instr is not cached - return from memory like usual but
        std::__1::optional<Word> response = _mem.Read(_requestedIp);

        // put the whole line in which instruction lives in cache
        Line new_line = Line();
        _requestedIp = ToLineAddr(_requestedIp);                    // set requested ip to line beginning
        for (size_t i = 0; i < lineSizeWords; i++) {                // fill the line with 32 instructions
            new_line[i] = _mem.Read(_requestedIp);
            _requestedIp += 4;
        }// create new cache record assigning _tag value to its tag
        auto new_record = std::make_pair(_tag, new_line);
        if (_code_cache.size() >= codeCacheBytes / lineSizeBytes)   // if cache is full
        {
            // find line with less usage in queue containing tag-clock pair
            auto min = *std::min_element(
                    _code_queue.begin(),
                    _code_queue.end(),
                    CompareSecond());

            _data_cache.erase(min.first);     // after data is saved to memory from cache it can be removed from cache
            _code_queue.erase(min.first);
        }
        _code_cache.push_back(new_record);                          // push new record in cache and to queue
        _code_queue.insert({_tag, clock()});
        return response;
    }

// if instruction is load/store type - set requested_ip to instr->addr from instruction
    void Request(InstructionPtr &instr) {
        if (instr->_type != IType::Ld && instr->_type != IType::St)
            return;

        _tag = ToLineAddr(instr->_addr) / lineSizeBytes;            // evaluating _tag value
        _cached = false;
        _waitCycles = _latency;
        if (_data_cache.find(_tag) != _data_cache.end()) {          // if record tagged with _tag exists
            _line = _data_cache[_tag].first;                        // get line of data from cache
            _cached = true;                                         // set cached to true
            _waitCycles = 3;                                        // set latency
        }
        _requestedIp = instr->_addr;                                // set requested ip to data addr in instruction
    }

    bool
    Response(InstructionPtr &instr)                            // checks whether the instruction is executed yet or not
    {
        if (instr->_type != IType::Ld && instr->_type !=
                                         IType::St)
            // if instruction type is not load or store - it's executed immediately, thus instruction is executed already
            return true;

        if (_waitCycles !=
            0)                                       // if there are cycles to wait for - instruction is not executed yet
            return false;

        if (!_cached) {
            Line new_line = Line();
            Word line_begin = ToLineAddr(_requestedIp);
            for (size_t i = 0; i < lineSizeWords; i++) {            // fill the line with 32 words from memory
                new_line[i] = _mem.Read(line_begin);
                line_begin += 4;
            }
            std::__1::pair<Line, bool> new_record = std::make_pair(new_line, true);
            if (_data_cache.size() >= dataCacheBytes / lineSizeBytes) {

                // find line with less usage in queue2 containing tag-clock pair
                auto min = std::min_element(_data_queue.begin(), _data_queue.end(),
                                            CompareSecond());
                auto tag_of_min = min->first;

                if (!_data_cache[tag_of_min].second) {
                    size_t ip = tag_of_min * lineSizeBytes;
                    Line line = _data_cache[tag_of_min].first;
                    for (auto iter = line.begin();
                         iter != line.end(); iter++) {            // iterate over line and write 32 words to memory
                        _mem.Write(ip, *iter);
                        ip += 4;
                    }
                }
                // after data is saved to memory from cache it can be removed from cache
                _data_cache.erase(
                        tag_of_min);
                _data_queue.erase(tag_of_min);
            }
            _data_cache.insert({_tag, new_record});
            _data_queue.insert({_tag, clock()});
        }

        _data_queue[_tag] = clock();
        if (instr->_type == IType::Ld)
            instr->_data = _data_cache[_tag].first[ToLineOffset(_requestedIp)];          // get data from cache
        else {
            _data_cache[_tag].first[ToLineOffset(_requestedIp)] = instr->_data;          // alter data in cache
            _data_cache[_tag].second = false;                                            // data is not fresh anymore(ew)
        }
        return true;
    }

    void Clock() {
        if (_waitCycles > 0)
            --_waitCycles;
    }


private:
    struct CompareSecond {
        bool operator()(const TagClockPair &left, const TagClockPair &right) const {
            return left.second < right.second;
        }
    };

    std::__1::map<size_t, clock_t> _code_queue;                           // key - _tag, value - last time accessed
    std::__1::map<size_t, clock_t> _data_queue;                           // key - _tag, value - last time accessed
    static constexpr size_t _latency = 152;
    Word _requestedIp = 0;
    size_t _waitCycles = 0;

    size_t _tag;
    Line _line;
    MemoryStorage &_mem;
    // pair of tag and line
    std::__1::vector<std::__1::pair<size_t, Line>> _code_cache;
    // tag, line and flag indicating whether thr record is fresh or not
    std::__1::map<size_t, std::__1::pair<Line, bool>> _data_cache;
    bool _cached = false;
};

#endif //RISCV_SIM_CACHEDMEMORY_H
