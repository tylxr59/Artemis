qt_add_executable(artemis-unit-tests
    unit/TestCore.cpp
    ../src/app/AppController.cpp
    ../src/app/AppController.h
    ../src/git/GitService.cpp
    ../src/git/GitService.h
    ../src/models/ConversationModel.cpp
    ../src/models/ConversationModel.h
    ../src/models/ProjectTreeModel.cpp
    ../src/models/ProjectTreeModel.h
    ../src/persistence/Database.cpp
    ../src/persistence/Database.h
    ../src/platform/Paths.cpp
    ../src/platform/Paths.h
    ../src/platform/DesktopIntegration.cpp
    ../src/platform/DesktopIntegration.h
    ../src/providers/ProviderFactory.cpp
    ../src/providers/ProviderFactory.h
    ../src/providers/codex/CodexClient.cpp
    ../src/providers/codex/CodexClient.h
    ../src/domain/AgentProvider.h
)
target_include_directories(artemis-unit-tests PRIVATE ../src)
target_compile_definitions(artemis-unit-tests PRIVATE ARTEMIS_VERSION="${PROJECT_VERSION}")
target_link_libraries(artemis-unit-tests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Test
    Qt6::Widgets
    KF6::Notifications
)
target_compile_options(artemis-unit-tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
)
add_test(NAME artemis-unit-tests COMMAND artemis-unit-tests)
