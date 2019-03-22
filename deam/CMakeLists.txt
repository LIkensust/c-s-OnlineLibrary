add_executable(deam Mysqldeam.cc)
add_executable(deam1 mysqldemo2.cc)
target_link_libraries(deam ${LIB_NAME})
target_link_libraries(deam1 ${LIB_NAME})
