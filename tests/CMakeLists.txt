find_package(GTest REQUIRED)

add_executable(test_mathml2omml
    test_mathml2omml.cpp
    ${CMAKE_SOURCE_DIR}/mathml2omml.cpp
)
target_include_directories(test_mathml2omml PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_mathml2omml PRIVATE GTest::gtest_main Qt6::Core)
add_test(NAME test_mathml2omml COMMAND ./test_mathml2omml)
