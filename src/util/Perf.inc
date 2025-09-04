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
uint64_t perf::running::start           = 0;
uint64_t perf::running::stop            = 0;
uint64_t perf::interrupt::notify        = 0;
uint64_t perf::interrupt::wakeup        = 0;
uint64_t perf::interrupt::interrupt     = 0;
uint64_t perf::interrupt::request       = 0;
uint64_t perf::timer::timer             = 0;
uint64_t perf::timer::timeout           = 0;
uint64_t perf::timer::updatePTC         = 0;
uint64_t perf::processor::abort         = 0;
uint64_t perf::processor::requestReschedule = 0;
uint64_t perf::processor::needReschedule = 0;
uint64_t perf::processor::checkRequestReschedule = 0;
uint64_t perf::processor::rescheduleRequest = 0;
uint64_t perf::processor::interruptFlag = 0;
uint64_t perf::processor::interrupt     = 0;
uint64_t perf::processor::timerFlag     = 0;
uint64_t perf::processor::timer         = 0;
uint64_t perf::processor::running       = 0;
uint64_t perf::network::transmit        = 0;
uint64_t perf::network::wait_for        = 0;
uint64_t perf::network::receive         = 0;
uint64_t perf::network::select          = 0;
uint64_t perf::disk::process            = 0;
uint64_t perf::disk::read               = 0;
uint64_t perf::disk::write              = 0;
uint64_t perf::disk::verify             = 0;

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
    {"running::start"        , perf::running::start},
    {"running::stop"         , perf::running::stop},
    {"interrupt::notify"     , perf::interrupt::notify},
    {"interrupt::wakeup"     , perf::interrupt::wakeup},
    {"interrupt::interrupt"  , perf::interrupt::interrupt},
    {"interrupt::request"    , perf::interrupt::request},
    {"timer::timer"          , perf::timer::timer},
    {"timer::timeout"        , perf::timer::timeout},
    {"timer::updatePTC"      , perf::timer::updatePTC},
    {"processor::abort"      , perf::processor::abort},
    {"processor::requestReschedule", perf::processor::requestReschedule},
    {"processor::needReschedule", perf::processor::needReschedule},
    {"processor::checkRequestReschedule", perf::processor::checkRequestReschedule},
    {"processor::rescheduleRequest", perf::processor::rescheduleRequest},
    {"processor::interruptFlag", perf::processor::interruptFlag},
    {"processor::interrupt"  , perf::processor::interrupt},
    {"processor::timerFlag"  , perf::processor::timerFlag},
    {"processor::timer"      , perf::processor::timer},
    {"processor::running"    , perf::processor::running},
    {"network::transmit"     , perf::network::transmit},
    {"network::wait_for"     , perf::network::wait_for},
    {"network::receive"      , perf::network::receive},
    {"network::select"       , perf::network::select},
    {"disk::process"         , perf::disk::process},
    {"disk::read"            , perf::disk::read},
    {"disk::write"           , perf::disk::write},
    {"disk::verify"          , perf::disk::verify},
};
}
