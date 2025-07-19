cmake_minimum_required(VERSION 3.10)

project(turbofec VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------
# --- Compiler Flags
# --------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fvisibility=hidden")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# --------------------------------------------------------------------
# --- Library Target
# --------------------------------------------------------------------
add_library(turbofec
    src/conv_dec.c
    src/conv_enc.c
    src/conv_rate_match.c
    src/turbo_dec.c
    src/turbo_enc.c
    src/turbo_rate_match.c
)

add_library(turbofec::turbofec ALIAS turbofec)

target_include_directories(turbofec
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

# --------------------------------------------------------------------
# --- NEON Support Check
# --------------------------------------------------------------------
include(CheckCSourceCompiles)

set(NEON_TEST_CODE
"
#include <arm_neon.h>
float32x4_t test_neon(float32x4_t a) { return vaddq_f32(a, a); }
int main() {
    float32x4_t v = vdupq_n_f32(1.0);
    v = test_neon(v);
    return 0;
}
")

check_c_source_compiles("${NEON_TEST_CODE}" HAVE_NEON)

if(HAVE_NEON)
    message(STATUS "NEON support detected")
    target_compile_definitions(turbofec PUBLIC HAVE_NEON)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        message(STATUS "ARMv8-A 64-bit (aarch64/arm64) detected")
        target_compile_options(turbofec PUBLIC "-march=armv8-a+fp+simd+crypto+crc")
        target_compile_definitions(turbofec PUBLIC ARCH_AARCH64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv8")
        message(STATUS "ARMv8-A 32-bit detected")
        target_compile_options(turbofec PUBLIC "-mfpu=neon-fp-armv8")
        target_compile_definitions(turbofec PUBLIC ARCH_ARMV8)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        message(STATUS "ARMv7-A detected")
        target_compile_options(turbofec PUBLIC "-mfpu=neon")
        target_compile_definitions(turbofec PUBLIC ARCH_ARMV7)
    else()
        message(STATUS "ARM architecture detected, but no specific flags set for ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
else()
    message(STATUS "NEON support not detected")
endif()


# --------------------------------------------------------------------
# --- Testing

# --------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

# --------------------------------------------------------------------
# --- Installation
# --------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS turbofec
    EXPORT turbofec-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/turbofec/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/turbofec)

# --------------------------------------------------------------------
# --- Pkg-config and CMake Package
# --------------------------------------------------------------------
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(libdir \${prefix}/${CMAKE_INSTALL_LIBDIR})
set(includedir \${prefix}/${CMAKE_INSTALL_INCLUDEDIR})

configure_file(
  turbofec.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/turbofec.pc
  @ONLY
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/turbofec.pc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

install(EXPORT turbofec-targets
  FILE
    turbofec-targets.cmake
  NAMESPACE
    turbofec::
  DESTINATION
    lib/cmake/turbofec
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "turbofec-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/turbofec-config.cmake"
  INSTALL_DESTINATION "lib/cmake/turbofec"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/turbofec-config.cmake"
    "turbofec-config-version.cmake"
  DESTINATION
    "lib/cmake/turbofec"
)
