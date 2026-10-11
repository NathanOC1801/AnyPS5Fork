add_library(diagnostics STATIC
    src/Logger.cpp
)

target_include_directories(diagnostics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
