# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY Options)

add_spectre_library(${LIBRARY} INTERFACE)

spectre_target_headers(
  ${LIBRARY}
  INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/src
  HEADERS
  Factory.hpp
  Options.hpp
  OptionsDetails.hpp
  ParseOptions.hpp
  )

target_link_libraries(
  ${LIBRARY}
  INTERFACE
  ErrorHandling
  Utilities
  YamlCpp
  )
