cmake_minimum_required (VERSION 3.8)

project ("PhotoTime")

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

add_executable (PhotoTime WIN32
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

target_link_libraries(PhotoTime comctl32.lib)
target_link_libraries(PhotoTime comdlg32.lib)
target_link_libraries(PhotoTime advapi32.lib)
