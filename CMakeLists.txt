cmake_minimum_required(VERSION 3.28)
project(Lab3)

set(INCLUDE_DIRS "include")

if(WIN32)
    message(STATUS "Собираем под Windows")

    set(SOURCE 
        "src/win/program.c"
        "src/win/logger.c"
        "src/main.c"
    )

    set(EXTRA_LIBS
        kernel32
    )
else()
    set(SOURCE 
        "src/linux/program.c"
        "src/linux/logger.c"
        "src/main.c"
    )
    message(STATUS "Собираем под Linux")

    set(EXTRA_LIBS "")
endif()

add_executable(lab3 ${SOURCE})

target_link_libraries(lab3 ${EXTRA_LIBS})

target_include_directories(lab3 PRIVATE ${INCLUDE_DIRS})

target_compile_options(lab3 PUBLIC -std=c11)
