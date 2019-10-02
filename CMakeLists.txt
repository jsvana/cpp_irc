cmake_minimum_required(VERSION 3.1)
project(irc)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(OpenSSL REQUIRED)

find_package(Boost 1.71 REQUIRED system)

file(GLOB_RECURSE irc_sources src/*.cpp)
add_executable(irc ${irc_sources})

set_target_properties(irc PROPERTIES CXX_STANDARD 17)

target_include_directories(irc PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(
  irc

  OpenSSL::SSL
  ${OPENSSL_CRYPTO_LIBRARY}
  pthread
  ${Boost_LIBRARIES}
)
