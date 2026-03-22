cmake_minimum_required(VERSION 3.5)

PROJECT(CG_Phase1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define os teus executáveis
add_executable(generator generator.cpp)
add_executable(engine engine.cpp tinyxml/tinyxml2.cpp)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

if (WIN32)
	message(STATUS "Toolkits_DIR set to: " ${TOOLKITS_FOLDER})
	set(TOOLKITS_FOLDER "" CACHE PATH "Path to Toolkits folder")
	
	if (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")
		message(ERROR ": GLUT not found")
	endif ()	
	
	include_directories(${TOOLKITS_FOLDER}/glut)
	
    # Linkar as bibliotecas no Windows
	target_link_libraries(engine ${OPENGL_LIBRARIES} ${TOOLKITS_FOLDER}/glut/glut32.lib)
    # O generator não precisa do GLUT, apenas de C++ padrão
	
	if (EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll")
		file(COPY ${TOOLKITS_FOLDER}/glut/glut32.dll DESTINATION ${CMAKE_BINARY_DIR})
	endif()	
	
else (WIN32) #Linux and Mac

	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated")
	find_package(GLUT REQUIRED)
	include_directories(${GLUT_INCLUDE_DIR})
	link_directories(${GLUT_LIBRARY_DIRS})
	add_definitions(${GLUT_DEFINITIONS})
	
    # Linkar as bibliotecas no Linux/Mac
	target_link_libraries(engine ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
    # O generator não precisa do GLUT, apenas de C++ padrão

	if(NOT GLUT_FOUND)
	   message(ERROR ": GLUT not found!")
	endif(NOT GLUT_FOUND)
	
endif(WIN32)