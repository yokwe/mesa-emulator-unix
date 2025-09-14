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
uint64_t perf::memory::FetchPage        = 0;
uint64_t perf::memory::StorePage        = 0;
uint64_t perf::opcode::Dispatch         = 0;
uint64_t perf::opcode::DispatchEsc      = 0;
uint64_t perf::opcode::FrameFault       = 0;
uint64_t perf::opcode::PageFault        = 0;
uint64_t perf::opcode::CodeTrap         = 0;
uint64_t perf::opcode::EscOpcodeTrap    = 0;
uint64_t perf::opcode::OpcodeTrap       = 0;
uint64_t perf::opcode::UnboundTrap      = 0;
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
uint64_t perf::disk::process_time       = 0;
uint64_t perf::agent::beep              = 0;
uint64_t perf::agent::disk              = 0;
uint64_t perf::agent::display           = 0;
uint64_t perf::agent::floppy            = 0;
uint64_t perf::agent::keyPress          = 0;
uint64_t perf::agent::keyRelease        = 0;
uint64_t perf::agent::mouse             = 0;
uint64_t perf::agent::network           = 0;
uint64_t perf::agent::processor         = 0;
uint64_t perf::agent::stream            = 0;
uint64_t perf::variable::MP             = 0;
uint64_t perf::variable::WDC            = 0;
uint64_t perf::variable::WDC_enable     = 0;
uint64_t perf::variable::WDC_disable    = 0;
uint64_t perf::variable::WP             = 0;
uint64_t perf::variable::WP_exchange    = 0;
uint64_t perf::variable::WP_fetch_or    = 0;
uint64_t perf::variable::IT             = 0;
uint64_t perf::variable::PSB            = 0;
uint64_t perf::variable::MDS            = 0;
uint64_t perf::variable::LF             = 0;
uint64_t perf::variable::GF             = 0;
uint64_t perf::variable::CB             = 0;
uint64_t perf::variable::running        = 0;
uint64_t perf::variable::running_start  = 0;
uint64_t perf::variable::running_stop   = 0;
uint64_t perf::variable::time_running   = 0;
uint64_t perf::variable::time_not_running = 0;

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
    {"memory::FetchPage"     , perf::memory::FetchPage},
    {"memory::StorePage"     , perf::memory::StorePage},
    {"opcode::Dispatch"      , perf::opcode::Dispatch},
    {"opcode::DispatchEsc"   , perf::opcode::DispatchEsc},
    {"opcode::FrameFault"    , perf::opcode::FrameFault},
    {"opcode::PageFault"     , perf::opcode::PageFault},
    {"opcode::CodeTrap"      , perf::opcode::CodeTrap},
    {"opcode::EscOpcodeTrap" , perf::opcode::EscOpcodeTrap},
    {"opcode::OpcodeTrap"    , perf::opcode::OpcodeTrap},
    {"opcode::UnboundTrap"   , perf::opcode::UnboundTrap},
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
    {"disk::process_time"    , perf::disk::process_time},
    {"agent::beep"           , perf::agent::beep},
    {"agent::disk"           , perf::agent::disk},
    {"agent::display"        , perf::agent::display},
    {"agent::floppy"         , perf::agent::floppy},
    {"agent::keyPress"       , perf::agent::keyPress},
    {"agent::keyRelease"     , perf::agent::keyRelease},
    {"agent::mouse"          , perf::agent::mouse},
    {"agent::network"        , perf::agent::network},
    {"agent::processor"      , perf::agent::processor},
    {"agent::stream"         , perf::agent::stream},
    {"variable::MP"          , perf::variable::MP},
    {"variable::WDC"         , perf::variable::WDC},
    {"variable::WDC_enable"  , perf::variable::WDC_enable},
    {"variable::WDC_disable" , perf::variable::WDC_disable},
    {"variable::WP"          , perf::variable::WP},
    {"variable::WP_exchange" , perf::variable::WP_exchange},
    {"variable::WP_fetch_or" , perf::variable::WP_fetch_or},
    {"variable::IT"          , perf::variable::IT},
    {"variable::PSB"         , perf::variable::PSB},
    {"variable::MDS"         , perf::variable::MDS},
    {"variable::LF"          , perf::variable::LF},
    {"variable::GF"          , perf::variable::GF},
    {"variable::CB"          , perf::variable::CB},
    {"variable::running"     , perf::variable::running},
    {"variable::running_start", perf::variable::running_start},
    {"variable::running_stop", perf::variable::running_stop},
    {"variable::time_running", perf::variable::time_running},
    {"variable::time_not_running", perf::variable::time_not_running},
};
}
