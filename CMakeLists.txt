cmake_minimum_required (VERSION 2.8)

project (OnlineLibrary)

#add_subdirectory(include)
add_subdirectory(CJsonObject)

add_executable(ser service.cc)
add_executable(cli client.cc)

set(LIB_NAME "CJsonTool" "pthread" "hiredis" "mysqlclient")

target_link_libraries(ser ${LIB_NAME})
target_link_libraries(cli ${LIB_NAME})
