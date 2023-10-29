add_executable(test test.cpp)
target_link_libraries(test PRIVATE meow)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE meow)