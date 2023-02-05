add_library(heartbeat INTERFACE)

target_include_directories(heartbeat INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(heartbeat INTERFACE heartbeat.c)

#target_link_libraries(heartbeat INTERFACE)

# vim:set ts=4 sts=4 sw=4 tw=0 et:
