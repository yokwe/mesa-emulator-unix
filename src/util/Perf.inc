//
// generated file  --  DO NOT EDIT
//

uint64_t memory::Fetch              = 0;
uint64_t memory::Store              = 0;
uint64_t memory::ReadDbl            = 0;
uint64_t memory::FetchMds           = 0;
uint64_t memory::StoreMds           = 0;
uint64_t memory::ReadDblMds         = 0;
uint64_t memory::GetCodeByte        = 0;
uint64_t memory::GetCodeWord        = 0;
uint64_t memory::FetchByte          = 0;
uint64_t memory::StoreByte          = 0;
uint64_t memory::ReadField          = 0;
uint64_t memory::WriteField         = 0;
uint64_t memory::WriteMap           = 0;
uint64_t memory::peek               = 0;
uint64_t memory::FetchPda           = 0;
uint64_t memory::StorePda           = 0;
uint64_t memory::FetchPage          = 0;
uint64_t memory::StorePage          = 0;
uint64_t opcode::Dispatch           = 0;
uint64_t opcode::DispatchEsc        = 0;
uint64_t opcode::FrameFault         = 0;
uint64_t opcode::PageFault          = 0;
uint64_t opcode::CodeTrap           = 0;
uint64_t opcode::EscOpcodeTrap      = 0;
uint64_t opcode::OpcodeTrap         = 0;
uint64_t opcode::UnboundTrap        = 0;
uint64_t processor::for_loop        = 0;
uint64_t processor::reschedule      = 0;
uint64_t processor::running         = 0;
uint64_t processor::not_running     = 0;
uint64_t processor::abort           = 0;
uint64_t processor::busyWait        = 0;
uint64_t processor::notifyInterrupt = 0;
uint64_t network::transmit          = 0;
uint64_t network::receive_request   = 0;
uint64_t network::receive_process   = 0;
uint64_t network::receive_packet    = 0;
uint64_t disk::process              = 0;
uint64_t disk::read                 = 0;
uint64_t disk::write                = 0;
uint64_t disk::verify               = 0;
uint64_t disk::process_time         = 0;
uint64_t agent::beep                = 0;
uint64_t agent::disk                = 0;
uint64_t agent::display             = 0;
uint64_t agent::floppy              = 0;
uint64_t agent::keyPress            = 0;
uint64_t agent::keyRelease          = 0;
uint64_t agent::mouse               = 0;
uint64_t agent::network             = 0;
uint64_t agent::processor           = 0;
uint64_t agent::stream              = 0;
uint64_t variable::MP               = 0;
uint64_t variable::WDC              = 0;
uint64_t variable::WDC_enable       = 0;
uint64_t variable::WDC_disable      = 0;
uint64_t variable::WP               = 0;
uint64_t variable::WP_exchange      = 0;
uint64_t variable::WP_fetch_or      = 0;
uint64_t variable::IT               = 0;
uint64_t variable::PSB              = 0;
uint64_t variable::MDS              = 0;
uint64_t variable::LF               = 0;
uint64_t variable::GF               = 0;
uint64_t variable::CB               = 0;
uint64_t variable::running          = 0;
uint64_t variable::running_start    = 0;
uint64_t variable::running_stop     = 0;
uint64_t variable::time_running     = 0;
uint64_t variable::time_not_running = 0;
uint64_t bpf::fillBuffer            = 0;
uint64_t bpf::fillBuffer_data       = 0;
uint64_t bpf::read                  = 0;
uint64_t bpf::read_empty            = 0;
uint64_t bpf::read_select           = 0;
uint64_t bpf::read_zero             = 0;

std::vector<Entry> all {
    {"memory"   , "memory::Fetch"             , memory::Fetch},
    {"memory"   , "memory::Store"             , memory::Store},
    {"memory"   , "memory::ReadDbl"           , memory::ReadDbl},
    {"memory"   , "memory::FetchMds"          , memory::FetchMds},
    {"memory"   , "memory::StoreMds"          , memory::StoreMds},
    {"memory"   , "memory::ReadDblMds"        , memory::ReadDblMds},
    {"memory"   , "memory::GetCodeByte"       , memory::GetCodeByte},
    {"memory"   , "memory::GetCodeWord"       , memory::GetCodeWord},
    {"memory"   , "memory::FetchByte"         , memory::FetchByte},
    {"memory"   , "memory::StoreByte"         , memory::StoreByte},
    {"memory"   , "memory::ReadField"         , memory::ReadField},
    {"memory"   , "memory::WriteField"        , memory::WriteField},
    {"memory"   , "memory::WriteMap"          , memory::WriteMap},
    {"memory"   , "memory::peek"              , memory::peek},
    {"memory"   , "memory::FetchPda"          , memory::FetchPda},
    {"memory"   , "memory::StorePda"          , memory::StorePda},
    {"memory"   , "memory::FetchPage"         , memory::FetchPage},
    {"memory"   , "memory::StorePage"         , memory::StorePage},
    {"opcode"   , "opcode::Dispatch"          , opcode::Dispatch},
    {"opcode"   , "opcode::DispatchEsc"       , opcode::DispatchEsc},
    {"opcode"   , "opcode::FrameFault"        , opcode::FrameFault},
    {"opcode"   , "opcode::PageFault"         , opcode::PageFault},
    {"opcode"   , "opcode::CodeTrap"          , opcode::CodeTrap},
    {"opcode"   , "opcode::EscOpcodeTrap"     , opcode::EscOpcodeTrap},
    {"opcode"   , "opcode::OpcodeTrap"        , opcode::OpcodeTrap},
    {"opcode"   , "opcode::UnboundTrap"       , opcode::UnboundTrap},
    {"processor", "processor::for_loop"       , processor::for_loop},
    {"processor", "processor::reschedule"     , processor::reschedule},
    {"processor", "processor::running"        , processor::running},
    {"processor", "processor::not_running"    , processor::not_running},
    {"processor", "processor::abort"          , processor::abort},
    {"processor", "processor::busyWait"       , processor::busyWait},
    {"processor", "processor::notifyInterrupt", processor::notifyInterrupt},
    {"network"  , "network::transmit"         , network::transmit},
    {"network"  , "network::receive_request"  , network::receive_request},
    {"network"  , "network::receive_process"  , network::receive_process},
    {"network"  , "network::receive_packet"   , network::receive_packet},
    {"disk"     , "disk::process"             , disk::process},
    {"disk"     , "disk::read"                , disk::read},
    {"disk"     , "disk::write"               , disk::write},
    {"disk"     , "disk::verify"              , disk::verify},
    {"disk"     , "disk::process_time"        , disk::process_time},
    {"agent"    , "agent::beep"               , agent::beep},
    {"agent"    , "agent::disk"               , agent::disk},
    {"agent"    , "agent::display"            , agent::display},
    {"agent"    , "agent::floppy"             , agent::floppy},
    {"agent"    , "agent::keyPress"           , agent::keyPress},
    {"agent"    , "agent::keyRelease"         , agent::keyRelease},
    {"agent"    , "agent::mouse"              , agent::mouse},
    {"agent"    , "agent::network"            , agent::network},
    {"agent"    , "agent::processor"          , agent::processor},
    {"agent"    , "agent::stream"             , agent::stream},
    {"variable" , "variable::MP"              , variable::MP},
    {"variable" , "variable::WDC"             , variable::WDC},
    {"variable" , "variable::WDC_enable"      , variable::WDC_enable},
    {"variable" , "variable::WDC_disable"     , variable::WDC_disable},
    {"variable" , "variable::WP"              , variable::WP},
    {"variable" , "variable::WP_exchange"     , variable::WP_exchange},
    {"variable" , "variable::WP_fetch_or"     , variable::WP_fetch_or},
    {"variable" , "variable::IT"              , variable::IT},
    {"variable" , "variable::PSB"             , variable::PSB},
    {"variable" , "variable::MDS"             , variable::MDS},
    {"variable" , "variable::LF"              , variable::LF},
    {"variable" , "variable::GF"              , variable::GF},
    {"variable" , "variable::CB"              , variable::CB},
    {"variable" , "variable::running"         , variable::running},
    {"variable" , "variable::running_start"   , variable::running_start},
    {"variable" , "variable::running_stop"    , variable::running_stop},
    {"variable" , "variable::time_running"    , variable::time_running},
    {"variable" , "variable::time_not_running", variable::time_not_running},
    {"bpf"      , "bpf::fillBuffer"           , bpf::fillBuffer},
    {"bpf"      , "bpf::fillBuffer_data"      , bpf::fillBuffer_data},
    {"bpf"      , "bpf::read"                 , bpf::read},
    {"bpf"      , "bpf::read_empty"           , bpf::read_empty},
    {"bpf"      , "bpf::read_select"          , bpf::read_select},
    {"bpf"      , "bpf::read_zero"            , bpf::read_zero},
};
