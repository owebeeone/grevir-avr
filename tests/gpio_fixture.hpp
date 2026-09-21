#pragma once
#include <GrevirAVR.h>
#include <grevir/test/register_memory.hpp>

namespace avr_mock {
namespace avr = ardo::sys::avr;
using grevir::test::Event;
using grevir::test::Kind;
struct Identity;
using Memory = grevir::test::RegisterMemory<256, Identity>;
using Binding = avr::base::RegisterSelector<Memory>;
using Bit = setl::BitsRW<bool, 7>;
using Fields = setl::BitFields<Bit>;
// These addresses are fixture data, mirroring the legacy IO + 0x20 convention.
using PortReg = avr::base::Register<Fields, avr::nfp::IoRegisterDef<std::uint8_t, 5, 0x20>, Binding::IoAccessor>;
using PinReg = avr::base::Register<Fields, avr::nfp::IoRegisterDef<std::uint8_t, 3, 0x20>, Binding::IoAccessor>;
using DdReg = avr::base::Register<Fields, avr::nfp::IoRegisterDef<std::uint8_t, 4, 0x20>, Binding::IoAccessor>;
struct Barrier {
  Barrier() { Memory::events.push_back({Kind::BarrierEnter, 0, 0, 0}); }
  ~Barrier() { Memory::events.push_back({Kind::BarrierExit, 0, 0, 0}); }
};
using Definition = avr::base::GpioPortDefinition<Bit, Bit, Bit, PortReg, PinReg, DdReg>;
struct Pin : avr::base::GpioPort<Pin, Definition, Barrier> {};
struct Fixture { Fixture() { Memory::reset(); } };
} // namespace avr_mock
