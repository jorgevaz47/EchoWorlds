find_package(SDL3 REQUIRED)

# Add source to this project's executable.
add_executable (EchoWorlds "EchoWorlds.cpp" "EchoWorlds.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET EchoWorlds PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(EchoWorlds PRIVATE SDL3::SDL3)
