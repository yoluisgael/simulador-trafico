cmake_minimum_required(VERSION 3.25)
project(trafico)

set(CMAKE_CXX_STANDARD 17)


add_executable(trafico main.cpp)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio REQUIRED)

include_directories(c:/SFML/include)

# Link against SFML, ImGui, and ImGui-SFML
target_link_libraries(trafico PRIVATE sfml-system sfml-window sfml-graphics sfml-audio )
