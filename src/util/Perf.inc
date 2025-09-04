//
// generated file  --  DO NOT EDIT
//

uint64_t perf::memory::Fetch            = 0;
uint64_t perf::memory::Store            = 0;
uint64_t perf::memory::ReadDbl          = 0;
uint64_t perf::memory::FetchMds         = 0;
uint64_t perf::memory::StoreMds         = 0;
uint64_t perf::memory::ReadDblMds       = 0;
uint64_t perf::memory::GetCodeByte      = 0;
uint64_t perf::memory::GetCodeWord      = 0;
uint64_t perf::memory::FetchByte        = 0;
uint64_t perf::memory::StoreByte        = 0;
uint64_t perf::memory::ReadField        = 0;
uint64_t perf::memory::WriteField       = 0;
uint64_t perf::memory::WriteMap         = 0;
uint64_t perf::memory::GetAddress       = 0;
uint64_t perf::memory::FetchPda         = 0;
uint64_t perf::memory::StorePda         = 0;
uint64_t perf::memory::MemoryFetch      = 0;
uint64_t perf::memory::MemoryStore      = 0;
uint64_t perf::opcode::Dispatch         = 0;
uint64_t perf::opcode::FrameFault       = 0;
uint64_t perf::opcode::PageFault        = 0;
uint64_t perf::opcode::CodeTrap         = 0;
uint64_t perf::opcode::EscOpcodeTrap    = 0;
uint64_t perf::opcode::OpcodeTrap       = 0;
uint64_t perf::opcode::UnboundTrap      = 0;

namespace perf {
std::vector<perf::Entry> all {
    {"memory::Fetch"         , perf::memory::Fetch},
    {"memory::Store"         , perf::memory::Store},
    {"memory::ReadDbl"       , perf::memory::ReadDbl},
    {"memory::FetchMds"      , perf::memory::FetchMds},
    {"memory::StoreMds"      , perf::memory::StoreMds},
    {"memory::ReadDblMds"    , perf::memory::ReadDblMds},
    {"memory::GetCodeByte"   , perf::memory::GetCodeByte},
    {"memory::GetCodeWord"   , perf::memory::GetCodeWord},
    {"memory::FetchByte"     , perf::memory::FetchByte},
    {"memory::StoreByte"     , perf::memory::StoreByte},
    {"memory::ReadField"     , perf::memory::ReadField},
    {"memory::WriteField"    , perf::memory::WriteField},
    {"memory::WriteMap"      , perf::memory::WriteMap},
    {"memory::GetAddress"    , perf::memory::GetAddress},
    {"memory::FetchPda"      , perf::memory::FetchPda},
    {"memory::StorePda"      , perf::memory::StorePda},
    {"memory::MemoryFetch"   , perf::memory::MemoryFetch},
    {"memory::MemoryStore"   , perf::memory::MemoryStore},
    {"opcode::Dispatch"      , perf::opcode::Dispatch},
    {"opcode::FrameFault"    , perf::opcode::FrameFault},
    {"opcode::PageFault"     , perf::opcode::PageFault},
    {"opcode::CodeTrap"      , perf::opcode::CodeTrap},
    {"opcode::EscOpcodeTrap" , perf::opcode::EscOpcodeTrap},
    {"opcode::OpcodeTrap"    , perf::opcode::OpcodeTrap},
    {"opcode::UnboundTrap"   , perf::opcode::UnboundTrap},
};
}
