cmake_minimum_required(VERSION 3.24)

project(Artemis VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Sql
    Test
    Widgets
)
find_package(KF6Kirigami REQUIRED)
find_package(KF6SyntaxHighlighting REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

set(ARTEMIS_SOURCES
    src/app/AppController.cpp
    src/app/AppController.h
    src/domain/AgentProvider.h
    src/git/GitService.cpp
    src/git/GitService.h
    src/models/ConversationModel.cpp
    src/models/ConversationModel.h
    src/models/ProjectTreeModel.cpp
    src/models/ProjectTreeModel.h
    src/persistence/Database.cpp
    src/persistence/Database.h
    src/providers/codex/CodexClient.cpp
    src/providers/codex/CodexClient.h
    src/platform/Paths.cpp
    src/platform/Paths.h
)

qt_add_executable(artemis
    src/main.cpp
    ${ARTEMIS_SOURCES}
)

qt_add_qml_module(artemis
    URI org.artemis
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/ConversationDelegate.qml
        qml/components/ThreadPane.qml
        qml/dialogs/CommitDialog.qml
        qml/dialogs/SettingsDialog.qml
        qml/pages/DiagnosticsPage.qml
)

target_include_directories(artemis PRIVATE src)
target_compile_definitions(artemis PRIVATE
    ARTEMIS_VERSION="${PROJECT_VERSION}"
    QT_NO_CAST_FROM_ASCII
)
target_link_libraries(artemis PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Widgets
    KF6::Kirigami
    KF6::SyntaxHighlighting
)

install(TARGETS artemis RUNTIME DESTINATION bin)
install(FILES resources/org.artemis.Artemis.desktop
    DESTINATION share/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
