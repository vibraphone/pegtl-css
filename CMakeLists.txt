cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(css-mapper)

find_package(pegtl REQUIRED)
find_package(ICU QUIET REQUIRED COMPONENTS i18n uc data)
# ICU pollutes our configuration.
mark_as_advanced(
 ICU_DERB_EXECUTABLE       ICU_GENBRK_EXECUTABLE     ICU_GENCCODE_EXECUTABLE
 ICU_GENCFU_EXECUTABLE     ICU_GENCMN_EXECUTABLE     ICU_GENCNVAL_EXECUTABLE
 ICU_GENDICT_EXECUTABLE    ICU_GENNORM2_EXECUTABLE   ICU_GENRB_EXECUTABLE
 ICU_GENSPREP_EXECUTABLE   ICU_ICU-CONFIG_EXECUTABLE ICU_ICUINFO_EXECUTABLE
 ICU_ICUPKG_EXECUTABLE     ICU_INCLUDE_DIR           ICU_MAKECONV_EXECUTABLE
 ICU_MAKEFILE_INC          ICU_PKGDATA_EXECUTABLE    ICU_PKGDATA_INC
 ICU_UCONV_EXECUTABLE
)

option(CSS_DBG_PARSE "Print debug information while parsing." ON)
option(CSS_DBG_GRAMMAR "Analyze the grammar to detect cycles." ON)

if (NOT DEFINED CSS_PEGTL_NAMESPACE)
  set(CSS_PEGTL_NAMESPACE "css_pegtl")
endif()

set(headers
  css/token/grammar.h
  css/composite/grammar.h
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/css/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/css/config.h
  @ONLY
)
list(APPEND headers ${CMAKE_CURRENT_BINARY_DIR}/css/config.h)

add_library(css INTERFACE)
target_link_libraries(css
  INTERFACE
    taocpp::pegtl
    ICU::i18n
    ICU::data
    ICU::uc
)
target_include_directories(css
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>"
    "$<INSTALL_INTERFACE:include/>"
)

add_executable(parse-css parse-css.cxx)
target_link_libraries(parse-css
  css
  taocpp::pegtl
  ICU::i18n
  ICU::data
  ICU::uc
)
