#include <GrevirAVR.h>
#include <grevir/avr/legacy_register_definitions.hpp>

namespace avr = ardo::sys::avr;
using Byte = avr::nfp::IoRegisterDef<std::uint8_t, 5, 0x20>;
static_assert(Byte::addr == 0x25);
static_assert(Byte::ForType<std::uint16_t>::addr == 0x25);
using Direct = avr::nfp::MemRegisterDef<std::uint8_t, 0x25>;
static_assert(Direct::addr == Byte::addr);
static_assert(Direct::ForType<std::uint16_t>::addr == Direct::addr);
using Bit = setl::BitsRW<bool, 7>;
using Hardware = avr::base::RegisterSelector<avr::base::VolatileAccess>;
using Port = avr::base::Register<setl::BitFields<Bit>, Byte, Hardware::IoAccessor>;
// Compiled only. Native tests never execute this MMIO binding.
void instantiate_mmio() {
  Port::ReadModifyWrite(Bit{true});
  (void)Port::Read();
}
