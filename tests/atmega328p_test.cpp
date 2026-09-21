#include "atmega328p_fixture.hpp"
#include <catch2/catch_test_macros.hpp>
using namespace atmega328p_mock;
using A = base::TimerOutputPinSettings<base::OcrEnum::OcrA, true>;
using B = base::TimerOutputPinSettings<base::OcrEnum::OcrB, false>;
using T0 = Bindings::Timer0::BuiltInTop<1000,16000000,base::TimerMode::pwm,base::TimerPwmMode::fast,8>;
using T1 = Bindings::Timer1::FrequencyAccurate<1000,16000000,base::TimerMode::pwm,base::TimerPwmMode::fast,base::TimerTop::icr>;
using T2 = Bindings::Timer2::FrequencyAccurate<20000,16000000,base::TimerMode::pwm,base::TimerPwmMode::fast,base::TimerTop::ocra>;

TEST_CASE_METHOD(Fixture, "ATmega328P Timer0 applies actual byte addresses and PD6 PD5 routes", "[avr]") {
  using Pins = base::TimerPwmPinConfiguration<T0,A,B>;
  Memory::bytes[0x44] = 0x0c;
  Memory::bytes[0x45] = 0x20;
  Memory::bytes[0x25] = 0x12;
  Memory::bytes[0x6e] = 1;
  Memory::bytes[0x35] = 7;
  Pins::setup();
  CHECK(Memory::bytes[0x44] == 0xbf);
  CHECK(Memory::bytes[0x45] == 0x23);
  CHECK(Memory::bytes[0x47] == 127);
  CHECK(Memory::bytes[0x48] == 127);
  CHECK(Memory::bytes[0x2a] == 0x60);
  CHECK(Memory::bytes[0x25] == 0x12);
  CHECK(Memory::bytes[0x6e] == 1);
  CHECK(Memory::bytes[0x35] == 7);
  Pins::pwmWrite(A{},255);
  CHECK((Memory::bytes[0x2b] & 0x40) == 0x40);
  CHECK((Memory::bytes[0x44] & 0xc0) == 0);
  CHECK(Pins::getFrequency<std::uint32_t>() == 980);
  for (auto e : io()) { CHECK(e.width == 1); }
}

TEST_CASE_METHOD(Fixture, "ATmega328P Timer1 configures ICR and preserves output duty on PB1 PB2", "[avr]") {
  using Pins = base::TimerPwmPinConfiguration<T1,A,B>;
  Pins::setup();
  CHECK(Memory::bytes[0x80] == 0xb2);
  CHECK(Memory::bytes[0x81] == 0x19);
  CHECK(word(0x86) == 16000);
  CHECK(word(0x88) == 8000);
  CHECK(word(0x8a) == 8000);
  CHECK(Memory::bytes[0x24] == 6);
  CHECK(Pins::getFrequency<std::uint32_t>() == 1000);
  Pins::pwmWrite(A{},4000);
  CHECK(Pins::setFrequency(2000) == 8000);
  CHECK(word(0x86) == 8000);
  CHECK(word(0x88) == 2000);
  CHECK(word(0x8a) == 4000);
  CHECK(Bindings::Timer1::getTopCount() == 8000);
  const auto before = Memory::bytes;
  Memory::events.clear();
  CHECK(Pins::setFrequency(0) == 0);
  CHECK(Memory::bytes == before);
  for (auto e : io()) { CHECK(e.kind == Kind::Read); }
}

TEST_CASE_METHOD(Fixture, "ATmega328P Timer2 uses its divider map and OCRB PD3 output with OCRA TOP", "[avr]") {
  using Pins = base::TimerPwmPinConfiguration<T2,B>;
  Pins::setup();
  CHECK(Memory::bytes[0xb0] == 0x33);
  CHECK(Memory::bytes[0xb1] == 0x0a);
  CHECK(Memory::bytes[0xb3] == 100);
  CHECK(Memory::bytes[0xb4] == 50);
  CHECK(Memory::bytes[0x2a] == 8);
  CHECK(Memory::bytes[0x24] == 0);
  CHECK(Pins::setFrequency(2000) == 250);
  CHECK(Memory::bytes[0xb1] == 0x0b); // Timer2 /32 is code 3.
  CHECK(Memory::bytes[0xb3] == 250);
  CHECK(Memory::bytes[0xb4] == 125);
  CHECK(Pins::getFrequency<std::uint32_t>() == 2000);
}

TEST_CASE_METHOD(Fixture, "ATmega328P word access orders bytes and brackets the shared latch", "[avr]") {
  using R = Bindings::Timer1Def::Registers;
  R::ReadModifyWrite(device::BitsICR1{0xabcd});
  REQUIRE(Memory::events == std::vector<Event>{{Kind::BarrierEnter,0,0,0},
    {Kind::Write,0x87,1,0xab},{Kind::Write,0x86,1,0xcd},{Kind::BarrierExit,0,0,0}});
  CHECK(word(0x86) == 0xabcd);
  Memory::events.clear();
  CHECK(R::Read<device::BitsICR1>() == 0xabcd);
  REQUIRE(Memory::events == std::vector<Event>{{Kind::BarrierEnter,0,0,0},
    {Kind::Read,0x86,1,0xcd},{Kind::Read,0x87,1,0xab},{Kind::BarrierExit,0,0,0}});
  R::ReadModifyWrite(device::BitsTCNT1{0x1234},device::BitsOCR1A{0x5678},device::BitsOCR1B{0x9abc});
  CHECK(word(0x84) == 0x1234);
  CHECK(word(0x88) == 0x5678);
  CHECK(word(0x8a) == 0x9abc);
  CHECK(R::Read<device::BitsTCNT1>() == 0x1234);
  CHECK(R::Read<device::BitsOCR1A>() == 0x5678);
}

TEST_CASE_METHOD(Fixture, "ATmega328P combined controls touch only the selected byte", "[avr]") {
  Memory::bytes[0x80] = 0x53;
  Memory::bytes[0x81] = 0x19;
  using Capture = Bindings::Timer1Def::TimerCaptureType;
  Capture::NoiseCanceller::Activate::apply<Bindings::Timer1Def::Registers>();
  CHECK(io() == std::vector<Event>{{Kind::Read,0x81,1,0x19},{Kind::Write,0x81,1,0x99}});
  CHECK(Memory::bytes[0x80] == 0x53);
  Capture::Rising::apply<Bindings::Timer1Def::Registers>();
  CHECK(Memory::bytes[0x81] == 0xd9);
  Memory::events.clear();
  Bindings::Timer0Def::Registers::ReadModifyWrite(device::BitsFOC0A{true});
  CHECK(io() == std::vector<Event>{{Kind::Read,0x45,1,0},{Kind::Write,0x45,1,0x80}});
  CHECK(Memory::bytes[0x45] == 0);
  Memory::events.clear();
  Bindings::Timer2Def::Registers::ReadModifyWrite(device::BitsFOC2B{true});
  CHECK(io() == std::vector<Event>{{Kind::Read,0xb1,1,0},{Kind::Write,0xb1,1,0x40}});
}

TEST_CASE_METHOD(Fixture, "ATmega328P flag clearing does not echo unrelated pending flags", "[avr]") {
  Memory::bytes[0x36] = 0x27;
  Bindings::Timer1Def::Registers::ReadModifyWrite(device::BitsOCF1A{true});
  CHECK(io() == std::vector<Event>{{Kind::Write,0x36,1,2}});
  CHECK(Memory::bytes[0x36] == 0x25);
  Bindings::Timer1Def::Registers::ReadModifyWrite(device::BitsICF1{true});
  CHECK(Memory::bytes[0x36] == 5);
  Bindings::Timer1Def::Registers::ReadModifyWrite(device::BitsTOV1{false});
  CHECK(Memory::bytes[0x36] == 5);
  Memory::bytes[0x35] = 7;
  Memory::bytes[0x37] = 7;
  Bindings::Timer0Def::Registers::ReadModifyWrite(device::BitsOCF0B{true});
  Bindings::Timer2Def::Registers::ReadModifyWrite(device::BitsOCF2A{true});
  CHECK(Memory::bytes[0x35] == 3);
  CHECK(Memory::bytes[0x37] == 5);
}

TEST_CASE_METHOD(Fixture, "ATmega328P capture interrupt and asynchronous status fields are present", "[avr]") {
  Memory::bytes[0x6f] = 3;
  Bindings::Timer1Def::Registers::ReadModifyWrite(device::BitsICIE1{true});
  CHECK(Memory::bytes[0x6f] == 0x23);
  Memory::bytes[0xb6] = 0x10;
  device::BitsTCN2UB status;
  setl::RegisterSelector<std::tuple<Bindings::RegisterASSR>>::Read(status);
  CHECK(status.value);
}
