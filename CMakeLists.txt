cmake_minimum_required(VERSION 3.8)
project(PhotoTime VERSION 0.1.0 LANGUAGES C)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    add_definitions(/MT /O2)
endif()

add_definitions(-DUNICODE -D_UNICODE)
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
target_link_libraries(PhotoTime comctl32 comdlg32 advapi32)

add_definitions(-DTEST_PARSEST)
add_executable(parsest
    "parsest.c"
    "utils.c")
