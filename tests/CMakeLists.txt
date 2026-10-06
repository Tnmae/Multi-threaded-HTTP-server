find_package(GTest REQUIRED)

add_executable(test_basic_response
  test_basic_response.cpp
)

#add_executable(test_response_fields
#  test_response_fields.cpp
#)
#
#add_executable(test_server_conn
#  test_server_conn.cpp
#)
#
#add_executable(test_server_creation
#  test_server_creation.cpp
#)
#

target_link_libraries(test_basic_response
  GTest::gtest
  GTest::gtest_main
)

#target_link_libraries(test_matrix
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)
#
#target_link_libraries(test_activation
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)
#
#target_link_libraries(test_kernelinit
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)
#
#target_link_libraries(test_optimizer
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)
#
#target_link_libraries(test_loss
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)
#
#target_link_libraries(test_denselayer
#  MLFromScratchLib
#  GTest::gtest
#  GTest::gtest_main
#)

include(GoogleTest)

gtest_discover_tests(test_basic_response)
#gtest_discover_tests(test_matrix)
#gtest_discover_tests(test_activation)
#gtest_discover_tests(test_kernelinit)
#gtest_discover_tests(test_optimizer)
#gtest_discover_tests(test_loss)
#gtest_discover_tests(test_denselayer)
