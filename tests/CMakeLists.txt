qt_add_executable(artemis-unit-tests
    unit/TestCore.cpp
    ../src/git/GitService.cpp
    ../src/git/GitService.h
    ../src/persistence/Database.cpp
    ../src/persistence/Database.h
    ../src/platform/Paths.cpp
    ../src/platform/Paths.h
)
target_include_directories(artemis-unit-tests PRIVATE ../src)
target_link_libraries(artemis-unit-tests PRIVATE Qt6::Core Qt6::Sql Qt6::Test)
add_test(NAME artemis-unit-tests COMMAND artemis-unit-tests)
