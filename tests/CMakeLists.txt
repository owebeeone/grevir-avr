if(GREVIR_BUILD_COMPILE_CHECKS)
  file(GLOB_RECURSE headers CONFIGURE_DEPENDS RELATIVE "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.hpp")
  set(sources)
  foreach(header IN LISTS headers)
    string(MAKE_C_IDENTIFIER "${header}" identifier)
    set(source "${CMAKE_CURRENT_BINARY_DIR}/${identifier}.cpp")
    file(WRITE "${source}" "#include <${header}>\n")
    list(APPEND sources "${source}")
  endforeach()
  add_library(grevir_avr_compile OBJECT ${sources} compile_usage.cpp timer_clock_static_tests.cpp timer_mode_static_tests.cpp
    timer_definition_static_tests.cpp timer_configuration_static_tests.cpp timer_output_static_tests.cpp atmega328p_static_tests.cpp)
  target_link_libraries(grevir_avr_compile PRIVATE grevir::avr)
  set_target_properties(grevir_avr_compile PROPERTIES CXX_EXTENSIONS OFF)
  if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    add_custom_target(grevir_timer_clock_checks ALL
      COMMAND "${CMAKE_COMMAND}" "-DCXX=${CMAKE_CXX_COMPILER}"
        "-DINCLUDE_DIR=${PROJECT_SOURCE_DIR}/src"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/timer_clock_probe.cpp"
        "-DLOG_DIR=${CMAKE_CURRENT_BINARY_DIR}/timer-clock-results"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_timer_clock.cmake"
      VERBATIM)
  endif()
endif()
if(GREVIR_BUILD_HOST_TESTS)
  if(NOT TARGET grevir::test_support)
    find_package(grevir-test-support CONFIG REQUIRED)
  endif()
  add_executable(grevir_avr_runtime gpio_test.cpp register_test.cpp timer_clock_test.cpp timer_clock_static_tests.cpp
    timer_mode_test.cpp timer_mode_static_tests.cpp timer_definition_test.cpp timer_definition_static_tests.cpp
    timer_configuration_test.cpp timer_configuration_static_tests.cpp timer_output_test.cpp timer_output_static_tests.cpp atmega328p_test.cpp atmega328p_static_tests.cpp)
  target_link_libraries(grevir_avr_runtime PRIVATE grevir::avr grevir::test_support Catch2::Catch2WithMain)
  set_target_properties(grevir_avr_runtime PROPERTIES CXX_EXTENSIONS OFF)
  catch_discover_tests(grevir_avr_runtime TEST_PREFIX "avr."
    PROPERTIES LABELS "avr-mock" TIMEOUT 10)
endif()
