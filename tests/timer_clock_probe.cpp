#include <grevir/avr/timer/clock.hpp>
using namespace ardo::sys::avr::base;
enum class Code { stopped, unit, eight };
enum class Other { unit };
using Unit = DividerMapping<Code, Code::unit, 1>;
using Eight = DividerMapping<Code, Code::eight, 8>;
template <int Id> struct Case;
template <> struct Case<0> { using Type = DividerMappings<Unit, Eight>; };
template <> struct Case<1> { using Type = DividerMapping<Code, Code::unit, 0>; };
template <> struct Case<2> { using Type = DividerMappings<Eight, Unit>; };
template <> struct Case<3> { using Type = DividerMappings<Unit, DividerMapping<Code, Code::unit, 8>>; };
template <> struct Case<4> { using Type = DividerMappings<Unit, DividerMapping<Other, Other::unit, 8>>; };
template <> struct Case<5> { using Type = DividerMapping<Code, Code::unit, InvalidClockDivider>; };
template <> struct Case<6> { using Type = DividerMappings<Unit, Eight, DividerMapping<Code, Code::stopped, 4>>; };
template <> struct Case<7> { using Type = DividerMappings<Unit, Eight, DividerMapping<Code, Code::eight, 16>>; };
static_assert(sizeof(Case<CASE_ID>::Type) > 0);
