add_subdirectory(app)
add_subdirectory(thirdparty)
add_subdirectory(unit)

target_include_directories(rh PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# target_link_options(rh PRIVATE -lpthread)
