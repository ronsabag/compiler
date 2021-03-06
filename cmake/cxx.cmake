option(ENABLE_CCACHE "Enable ccache support" ON)
if(ENABLE_CCACHE)
  find_program(BIN_CCACHE ccache)
  mark_as_advanced(BIN_CCACHE)

  if(NOT BIN_CCACHE)
    message_colored(STATUS "Couldn't locate ccache, disabling ccache..." "33")
  else()
    # Enable only if the binary is found
    message_colored(STATUS "Using compiler cache ${BIN_CCACHE}" "32")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${BIN_CCACHE} CACHE STRING "")
  endif()
endif()

option(CXXLIB_CLANG "Link against libc++" OFF)
option(CXXLIB_GCC "Link against stdlibc++" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Compiler flags
include(CheckCXXCompilerFlag)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
check_cxx_compiler_flag("-Wsuggest-override" HAS_SUGGEST_OVERRIDE)
if (HAS_SUGGEST_OVERRIDE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override")
endif()
unset(HAS_SUGGEST_OVERRIDE)

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  # Need dprintf() for FreeBSD 11.1 and older
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WITH_DPRINTF")
  # libinotify uses c99 extension, so suppress this error
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c99-extensions")
  # Ensures that libraries from dependencies in LOCALBASE are used
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=parentheses-equality")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-length-array")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -fdata-sections -ffunction-sections")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -Wl,--gc-sections,--icf=safe")
endif()

# Check compiler
if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4.0")
    message_colored(FATAL_ERROR "Compiler not supported (Requires clang-3.4+ or gcc-5.1+)" 31)
  else()
    message_colored(STATUS "Using supported compiler ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}" 32)
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1.0")
    message_colored(FATAL_ERROR "Compiler not supported (Requires clang-3.4+ or gcc-5.1+)" 31)
  else()
    message_colored(STATUS "Using supported compiler ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}" 32)
  endif()
else()
  message_colored(WARNING "Using unsupported compiler ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION} !" 31)
endif()

# Set compiler and linker flags for preferred C++ library
if(CXXLIB_CLANG)
  message_colored(STATUS "Linking against libc++" 32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++ -lc++abi")
elseif(CXXLIB_GCC)
  message_colored(STATUS "Linking against libstdc++" 32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
endif()

# Custom build type ; SANITIZE
SET(CMAKE_CXX_FLAGS_SANITIZE "-O0 -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls"
  CACHE STRING "Flags used by the C++ compiler during sanitize builds." FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_SANITIZE ""
  CACHE STRING "Flags used for linking binaries during sanitize builds." FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_SANITIZE ""
  CACHE STRING "Flags used by the shared libraries linker during sanitize builds." FORCE)
MARK_AS_ADVANCED(
  CMAKE_CXX_FLAGS_SANITIZE
  CMAKE_EXE_LINKER_FLAGS_SANITIZE
  CMAKE_SHARED_LINKER_FLAGS_SANITIZE)

# Custom build type ; Coverage
SET(CMAKE_CXX_FLAGS_COVERAGE
  "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_COVERAGE} --coverage")
SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE
  "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")
SET(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
  "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${CMAKE_SHARED_LINKER_FLAGS_COVERAGE}")
