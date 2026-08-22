cmake_minimum_required(VERSION 3.7.0)

project(SpriteChat VERSION 2.11.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Widgets Concurrent WebSockets UiTools)

if(NOT DEFINED SPRITECHAT_OUTPUT_DIRECTORY)
  set(SPRITECHAT_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" CACHE PATH "")
endif()

set(SPRITECHAT_RUNTIME_NAME "spritechat-runtime" CACHE STRING "Output filename executable")

qt_add_library(spritechat STATIC
  src/aoapplication.cpp
  src/aoapplication.h
  src/aoapplication_connection.cpp
  src/aoblipplayer.cpp
  src/aoblipplayer.h
  src/aobutton.cpp
  src/aobutton.h
  src/aocharbutton.cpp
  src/aocharbutton.h
  src/aoclocklabel.cpp
  src/aoclocklabel.h
  src/aoemotebutton.cpp
  src/aoemotebutton.h
  src/aoemotepreview.cpp
  src/aoemotepreview.h
  src/aoevidencebutton.cpp
  src/aoevidencebutton.h
  src/aoevidencedisplay.cpp
  src/aoevidencedisplay.h
  src/aoimage.cpp
  src/aoimage.h
  src/animationlayer.cpp
  src/animationlayer.h
  src/animationloader.h
  src/animationloader.cpp
  src/ao_widget_lookup.h
  src/aomusicplayer.cpp
  src/aomusicplayer.h
  src/aomusictrack.h
  src/aosfxplayer.cpp
  src/aosfxplayer.h
  src/aotextarea.cpp
  src/aotextarea.h
  src/aotextboxwidgets.cpp
  src/aotextboxwidgets.h
  src/aoutils.cpp
  src/aoutils.h
  src/area_registry.cpp
  src/area_registry.h
  src/asset_lookup.h
  src/charselect.cpp
  src/chatlogpiece.cpp
  src/chatlogpiece.h
  src/console_logger.cpp
  src/console_logger.h
  src/courtroom.cpp
  src/courtroom.h
  src/datatypes.h
  src/debug_functions.cpp
  src/debug_functions.h
  src/emotes.cpp
  src/eventfilters.cpp
  src/eventfilters.h
  src/evidence.cpp
  src/file_functions.cpp
  src/file_functions.h
  src/hardware_functions.cpp
  src/hardware_functions.h
  src/lobby.cpp
  src/lobby.h
  src/network_manager.cpp
  src/network_manager.h
  src/options.cpp
  src/options.h
  src/packet_distribution.cpp
  src/path_functions.cpp
  src/player_registry.cpp
  src/player_registry.h
  src/scrolltext.cpp
  src/scrolltext.h
  src/serverdata.cpp
  src/serverdata.h
  src/spritechat_info.cpp
  src/spritechat_info.h
  src/spritechat_log.cpp
  src/spritechat_log.h
  src/text_file_functions.cpp
  src/timer.cpp
  src/timer.h
  src/widgets/aooptionsdialog.cpp
  src/widgets/aooptionsdialog.h
  src/widgets/direct_connect_dialog.cpp
  src/widgets/direct_connect_dialog.h
  src/widgets/server_editor_dialog.cpp
  src/widgets/server_editor_dialog.h
  src/widgets/playerlistwidget.h src/widgets/playerlistwidget.cpp
  src/widgets/moderator_dialog.h src/widgets/moderator_dialog.cpp
  src/screenslidetimer.h src/screenslidetimer.cpp
  src/moderation_functions.h src/moderation_functions.cpp
  src/network/master_client.h src/network/master_client.cpp
  src/network/master_gateway.h src/network/master_gateway.cpp
  src/network/server_bookmark.h src/network/server_bookmark.cpp
  src/network/server_info_gateway.h src/network/server_info_gateway.cpp
  data.qrc
)

qt_add_executable(spritechat-runtime
  src/main.cpp
)

if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET spritechat-runtime PROPERTY WIN32_EXECUTABLE true)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/data/logo-client.rc")
    target_sources(spritechat-runtime PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
  endif()
endif()

target_include_directories(spritechat PUBLIC src external)
target_link_directories(spritechat PUBLIC external)
target_link_libraries(spritechat PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Concurrent
  Qt${QT_VERSION_MAJOR}::WebSockets
  Qt${QT_VERSION_MAJOR}::UiTools
  bass
  bassopus
  theory-game
  theory-protocol
)

target_link_libraries(spritechat-runtime PRIVATE spritechat)

target_compile_definitions(spritechat PUBLIC QT_NO_KEYWORDS)

set_target_properties(spritechat-runtime PROPERTIES
  OUTPUT_NAME "${SPRITECHAT_RUNTIME_NAME}"
  LIBRARY_OUTPUT_DIRECTORY "$<1:${SPRITECHAT_OUTPUT_DIRECTORY}>"
  RUNTIME_OUTPUT_DIRECTORY "$<1:${SPRITECHAT_OUTPUT_DIRECTORY}>"
)
