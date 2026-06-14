cmake_minimum_required(VERSION 3.8)
project(PhotoTime VERSION 0.1.0 LANGUAGES C)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

add_definitions(-DUNICODE -D_UNICODE)
add_executable(PhotoTime WIN32
    "src/about.c"
    "src/commctrls.c"
    "src/export.c"
    "src/gdip.c"
    "src/listview.c"
    "src/main.c"
    "src/main.rc"
    "src/parsest.c"
    "src/photo.c"
    "src/photoview.c"
    "src/statusbar.c"
    "src/toolbar.c"
    "src/utils.c")
target_link_libraries(PhotoTime comctl32 comdlg32 advapi32)
