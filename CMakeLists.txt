cmake_minimum_required(VERSION 3.0)
project(CG_Phase1)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} tinyxml)

# O Gerador continua igual
add_executable(generator generator.cpp)

# O Engine precisa do engine.cpp E do tinyxml2.cpp
add_executable(engine engine.cpp tinyxml/tinyxml2.cpp)
target_link_libraries(engine ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})