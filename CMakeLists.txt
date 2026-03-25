#
# mod-iaplayerbot — CMakeLists.txt
# AzerothCore module build configuration
#
# AC modules work by appending sources to AC_MODULE_SOURCES
# and include dirs to AC_MODULE_INCLUDE_DIRS. The AC build system
# picks these up when it creates the `modules` target later.
#

set(MOD_IABOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# Collect all our source files
file(GLOB_RECURSE MOD_IABOT_SRCS
    "${MOD_IABOT_DIR}/src/*.cpp"
    "${MOD_IABOT_DIR}/src/*.h"
)

# Append to the AC module sources list
list(APPEND AC_MODULE_SOURCES ${MOD_IABOT_SRCS})
set(AC_MODULE_SOURCES ${AC_MODULE_SOURCES} PARENT_SCOPE)

# Include directories
list(APPEND AC_MODULE_INCLUDE_DIRS
    "${MOD_IABOT_DIR}/src"
    "${MOD_IABOT_DIR}/src/Hooks"
    "${MOD_IABOT_DIR}/src/Manager"
    "${MOD_IABOT_DIR}/src/Scheduler"
    "${MOD_IABOT_DIR}/src/Config"
)
set(AC_MODULE_INCLUDE_DIRS ${AC_MODULE_INCLUDE_DIRS} PARENT_SCOPE)

# Register the script loader entry point
AC_ADD_SCRIPT("${MOD_IABOT_DIR}/src/IABotModule.cpp")

# Config file distribution
set(AC_MODULE_CONFIGS "${AC_MODULE_CONFIGS};iaplayerbot.conf" PARENT_SCOPE)
install(FILES "${MOD_IABOT_DIR}/conf/iaplayerbot.conf.dist" DESTINATION "${CMAKE_INSTALL_PREFIX}/etc")

message(STATUS "WORLD: mod-iaplayerbot loaded")
