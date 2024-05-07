cmake_minimum_required(VERSION 3.8)

project("PhotoTime")

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

add_executable(PhotoTime WIN32
    "about.c"
    "commctrls.c"
    "export.c"
    "gdip.c"
    "listview.c"
    "main.c"
    "main.rc"
    "parsest.c"
    "photo.c"
    "photoview.c"
    "statusbar.c"
    "toolbar.c"
    "utils.c")

target_link_libraries(PhotoTime comctl32.lib comdlg32.lib advapi32.lib)

add_definitions(-DTEST_PARSEST)

add_executable(parsest
    "parsest.c"
    "utils.c")
