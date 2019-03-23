add_executable(demo Mysqldeam.cc)
add_executable(demo1 mysqldemo2.cc)
target_link_libraries(demo ${LIB_NAME})
target_link_libraries(demo1 ${LIB_NAME})
