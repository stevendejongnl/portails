# SPDX-License-Identifier: AGPL-3.0-or-later
cmake_minimum_required(VERSION 3.21)
project(portails VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel Test)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED IMPORTED_TARGET libsecret-1)

# Everything that does not need a window. The tests link this library.
add_library(portails_core STATIC
    src/platforms.cpp
    src/catalog_refresh.cpp
    src/tab_store.cpp
    src/tab_inputs.cpp
    src/navigation_rules.cpp
    src/user_agent.cpp
    src/accept_language.cpp
    src/crypto.cpp
    src/profile_vault.cpp)
target_include_directories(portails_core PUBLIC src)
target_link_libraries(portails_core PUBLIC Qt6::Core OpenSSL::Crypto)

add_executable(portails
    src/main.cpp
    src/main_window.cpp
    src/app_controller.cpp
    src/backend.cpp
    src/tab_views.cpp
    src/browser_features.cpp
    src/site_page.cpp
    src/signal_quit.cpp
    src/keyring.cpp
    src/splash.cpp)
target_link_libraries(portails PRIVATE portails_core Qt6::Widgets Qt6::WebEngineWidgets Qt6::WebChannel PkgConfig::LIBSECRET)

# The sidebar page, its scripts, fonts and logos are part of the binary.
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} ui/*)
qt_add_resources(portails "interface" PREFIX "/" BASE ${CMAKE_SOURCE_DIR}
    FILES ${UI_FILES} assets/icon.png assets/chrome_object.js)

install(TARGETS portails RUNTIME DESTINATION bin)
install(FILES packaging/portails.desktop DESTINATION share/applications)
foreach(size 16 32 48 64 128 256 512)
    install(FILES assets/icons/portails-${size}.png
            DESTINATION share/icons/hicolor/${size}x${size}/apps RENAME portails.png)
endforeach()
install(FILES assets/icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME portails.svg)
install(FILES LICENSE ui/fonts/Inter-LICENSE.txt DESTINATION share/licenses/portails)

enable_testing()
function(add_portails_test name)
    add_executable(${name} tests/${name}.cpp)
    target_link_libraries(${name} PRIVATE portails_core Qt6::Test)
    target_compile_definitions(${name} PRIVATE PORTAILS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_portails_test(test_platforms)
add_portails_test(test_catalog_refresh)
add_portails_test(test_tab_store)
add_portails_test(test_tab_inputs)
add_portails_test(test_navigation_rules)
add_portails_test(test_user_agent)
add_portails_test(test_accept_language)
add_portails_test(test_crypto)
add_portails_test(test_profile_vault)
