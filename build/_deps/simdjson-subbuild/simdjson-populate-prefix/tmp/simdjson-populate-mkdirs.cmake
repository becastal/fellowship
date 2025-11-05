# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-src"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-build"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/tmp"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src"
  "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/becastal/Documents/fellowship/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
