#include "Cpu.h"
#include "Memory/MemoryConfig.h"
#include "BaseTypes.h"
#include "Memory/MemoryStorage.h"
#include "Memory/CachedMemory.h"

#include <optional>

int main()
{
    MemoryStorage mem ;
    mem.LoadElf("program");
    std::unique_ptr<CachedMemory> memModelPtr(new CachedMemory (mem));
    Cpu cpu{*memModelPtr};
    cpu.Reset(0x200);

    int32_t print_int = 0;
    while (true)
    {
        cpu.Clock();
        memModelPtr->Clock();
        std::optional<CpuToHostData> msg = cpu.GetMessage();
        if (!msg)
            continue;

        auto type = msg.value().unpacked.type;
        auto data = msg.value().unpacked.data;

        if(type == CpuToHostType::ExitCode) {
            if(data == 0) {
                fprintf(stderr, "PASSED\n");
                return 0;
            } else {
                fprintf(stderr, "FAILED: exit code = %d\n", data);
                return data;
            }
            break;
        } else if(type == CpuToHostType::PrintChar) {
            fprintf(stderr, "%c", (char)data);
        } else if(type == CpuToHostType::PrintIntLow) {
            print_int = uint32_t(data);
        } else if(type == CpuToHostType::PrintIntHigh) {
            print_int |= uint32_t(data) << 16;
            fprintf(stderr, "%d", print_int);
        }
    }

}
