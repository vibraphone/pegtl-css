cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(css-mapper)

list(APPEND CMAKE_PREFIX_PATH "/stage/build/misc/install") # NB: PkgConfig stuff appends "lib/pkgconfig" to this.

find_package(pegtl REQUIRED)
find_package(ICU REQUIRED COMPONENTS i18n uc data)

add_executable(parse-css parse-css.cxx)
target_link_libraries(parse-css
  taocpp::pegtl
  ICU::i18n
  ICU::data
  ICU::uc
)
