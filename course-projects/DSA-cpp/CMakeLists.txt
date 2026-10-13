cmake_minimum_required(VERSION 3.22)
project(LethalCompanyDSA VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Шлях до попередньо встановленого SFML 3.0.2
list(APPEND CMAKE_PREFIX_PATH "E:/Dev/Dependencies/SFML-3.0.2")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# Шляхи до сторонніх бібліотек у external/
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
set(IMGUI_SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-sfml")

# Збірка ImGui + ImGui-SFML як статичної бібліотеки
add_library(imgui_sfml STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

target_include_directories(imgui_sfml PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

target_compile_definitions(imgui_sfml PUBLIC
    IMGUI_USER_CONFIG="imconfig-SFML.h"
)

target_link_libraries(imgui_sfml PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    opengl32
)

# Головний виконуваний файл
add_executable(LethalCompanyDSA
    src/main.cpp
    src/map/BSPGenerator.cpp
    assets/app.rc
)

target_include_directories(LethalCompanyDSA PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

target_compile_definitions(LethalCompanyDSA PRIVATE
    IMGUI_USER_CONFIG="imconfig-SFML.h"
)

target_compile_options(LethalCompanyDSA PRIVATE
    -Wall
    -Wextra
)

target_link_libraries(LethalCompanyDSA PRIVATE
    imgui_sfml
    SFML::Graphics
    SFML::Window
    SFML::System
)

if(WIN32)
    target_link_libraries(LethalCompanyDSA PRIVATE dwmapi gdi32)
endif()


# Автоматичне копіювання SFML .dll файлів та папки assets у папку з бінарником
file(GLOB SFML_DLLS "E:/Dev/Dependencies/SFML-3.0.2/bin/*.dll")
add_custom_command(TARGET LethalCompanyDSA POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SFML_DLLS}
        $<TARGET_FILE_DIR:LethalCompanyDSA>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:LethalCompanyDSA>/assets
    COMMENT "Copying SFML DLLs and assets to binary directory..."
)
