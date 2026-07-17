# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Project definition
project(
	Elevator_capacity_controller
	VERSION 1.0
	LANGUAGES C CXX ASM
)

# Include the definition of the slc_Elevator_capacity_controller target,
# which contains the content of the SLC project
include(Elevator_capacity_controller.cmake)

add_executable(Elevator_capacity_controller
    # Add additional sources here
)

target_include_directories(Elevator_capacity_controller PUBLIC
    # Add additional include paths here
)

target_compile_definitions(Elevator_capacity_controller PUBLIC
    # Add additional macros here
)

target_compile_options(Elevator_capacity_controller PUBLIC
    # Set additional compiler flags here
)

target_link_options(Elevator_capacity_controller PUBLIC
    # Set additional linker flags here
)

# Link with the content defined in the SLC project
target_link_libraries(Elevator_capacity_controller PRIVATE
    slc
)

# Include managed project content if available
include(Elevator_capacity_controller_project.cmake OPTIONAL RESULT_VARIABLE managed_project)
if(managed_project)
    message(STATUS "Using managed project content from ${managed_project}")
endif()

# Force the gcc linker command
set_target_properties(Elevator_capacity_controller PROPERTIES LINKER_LANGUAGE C)

# Create .bin, .hex and .s37 artifacts after building the project
add_custom_command(TARGET Elevator_capacity_controller
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_SREC_CMD} "$<TARGET_FILE:Elevator_capacity_controller>" "$<TARGET_FILE_DIR:Elevator_capacity_controller>/$<TARGET_FILE_BASE_NAME:Elevator_capacity_controller>.s37"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_IHEX_CMD} "$<TARGET_FILE:Elevator_capacity_controller>" "$<TARGET_FILE_DIR:Elevator_capacity_controller>/$<TARGET_FILE_BASE_NAME:Elevator_capacity_controller>.hex"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_BIN_CMD}  "$<TARGET_FILE:Elevator_capacity_controller>" "$<TARGET_FILE_DIR:Elevator_capacity_controller>/$<TARGET_FILE_BASE_NAME:Elevator_capacity_controller>.bin" 
)

# Run post-build pipeline to perform additional post-processing
if(post_build_command)
add_custom_command(TARGET Elevator_capacity_controller
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMAND ${post_build_command}
)
endif()