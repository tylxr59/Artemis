# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/artemis_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/artemis_autogen.dir/ParseCache.txt"
  "artemis_autogen"
  "tests/CMakeFiles/artemis-unit-tests_autogen.dir/AutogenUsed.txt"
  "tests/CMakeFiles/artemis-unit-tests_autogen.dir/ParseCache.txt"
  "tests/artemis-unit-tests_autogen"
  )
endif()
