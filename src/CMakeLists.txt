add_executable(jinxle
    main.cpp
    jinxle.cpp
)

target_compile_features(jinxle PRIVATE cxx_std_20)

find_package(Curses REQUIRED)

target_include_directories(jinxle
    PUBLIC
        .
        ../extern/deeyou/include
        ../extern/ncursespp/include
    PRIVATE
        SYSTEM ${CURSES_INCLUDE_DIRS})
target_link_libraries(jinxle PRIVATE ncursespp ncursesw dl)

