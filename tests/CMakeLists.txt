add_executable(test_mathml2omml
    test_mathml2omml.cpp
)
target_link_libraries(test_mathml2omml PRIVATE mathml2omml GTest::gtest_main)
add_test(NAME test_mathml2omml COMMAND ./test_mathml2omml)
