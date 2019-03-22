cmake_minimum_required (VERSION 2.8)

project (OnlineLibrary)
set(LIB_NAME "CJsonTool" "pthread" "hiredis" "mysqlclient")
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
#add_subdirectory(include)
add_subdirectory(CJsonObject)
add_subdirectory(deam)
add_executable(ser server.cc)
add_executable(cli client.cc)
add_executable(CreateJson make_test_json_file.cc)


target_link_libraries(ser ${LIB_NAME})
target_link_libraries(cli ${LIB_NAME})
target_link_libraries(CreateJson ${LIB_NAME})
