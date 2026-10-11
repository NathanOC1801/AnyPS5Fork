add_test_executable(diagnostics_tests
    DiagnosticsTests.cpp
)

target_link_libraries(diagnostics_tests PRIVATE
    diagnostics
)

add_test(NAME diagnostics COMMAND diagnostics_tests)
