cmake_minimum_required(VERSION 3.6.2)
project(agio)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(boost-cmake)

include_directories(${CMAKE_SOURCE_DIR}/FPlusPlus/FPlusPlus)
include_directories(${CMAKE_SOURCE_DIR}/NEAT/include)

file(GLOB neat_sources NEAT/src/*.cpp)
list(FILTER neat_sources EXCLUDE REGEX ".*neatmain.cpp$")
list(FILTER neat_sources EXCLUDE REGEX ".*experiments.cpp$")
set(project_sources 
	src/Utils/Utils.cpp
	src/Evolution/Individual.cpp
	src/Evolution/Population.cpp
	src/Evolution/Globals.cpp
	src/Serialization/SIndividual.cpp
	src/Serialization/SNetwork.cpp
	src/Serialization/SRegistry.cpp
	src/Evolution/MorphologyTag.cpp
	src/Core/Config.cpp
	src/Utils/SFMLRenderer.cpp
	${neat_sources})

add_library(agio_lib STATIC ${project_sources})

target_link_libraries(agio_lib Boost::serialization) # While libraries aren't linked, cmake seems to need the dependency

if (WIN32)
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /std:c++17 -DWITHOUT_NUMPY /W3 /Od")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 -DWITHOUT_NUMPY /W3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 -DWITHOUT_NUMPY /Ox /GL /W3 /Zi")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /DEBUG /OPT:REF")

	# Copy the SFML binaries
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/openal32.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-graphics-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-graphics-d-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-system-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-system-d-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-window-d-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	file(COPY ${CMAKE_SOURCE_DIR}/libs_windows/sfml/dll/sfml-window-d-2.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else ()
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -std=c++1z -msse4.1 -pthread -DWITHOUT_NUMPY -O0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++1z -msse4.1 -pthread -DWITHOUT_NUMPY")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++1z -msse4.1 -pthread -DWITHOUT_NUMPY -O3")
endif ()

option(build_tests "Build the AGIO demo tests after building the library" ON)
if (build_tests)
	add_executable(prey_predator
			src/Tests/PreyPredator/main.cpp
			src/Tests/PreyPredator/PreyPredatorEvolution.cpp
			src/Tests/PreyPredator/PreyPredator.cpp
			src/Tests/PreyPredator/PublicInterfaceImpl.cpp)
	
	add_executable(experiment_3d
		src/Tests/3DExperiment/3DExperiment.cpp
		src/Tests/3DExperiment/main.cpp)

	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
	if (WIN32)
		file(GLOB SFML_LIBRARIES libs_windows/sfml/lib/*.lib)
		set(SFML_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs_windows/sfml/include)
	else ()
		find_package(SFML COMPONENTS graphics window system REQUIRED)	
	endif ()

	include_directories(${SFML_INCLUDE_DIR})

	add_executable(complex_system
			src/Tests/ComplexSystem/main.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/ComplexSystemEvolution.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)
	
	add_executable(perf_test
			src/Tests/ComplexSystem/PerfTest.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)
	
	add_executable(nn_graph_dump
			src/Tests/ComplexSystem/NNGraphsTests.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)

	add_executable(interrelations_test
			src/Tests/ComplexSystem/InterrelationsTest.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)

	add_executable(sim_size_test
			src/Tests/PreyPredator/SimSizeTest.cpp
			src/Tests/PreyPredator/PreyPredatorEvolution.cpp
			src/Tests/PreyPredator/PreyPredator.cpp
			src/Tests/PreyPredator/PublicInterfaceImpl.cpp)

	target_link_libraries(sim_size_test
		agio_lib
		Boost::serialization
		${SFML_LIBRARIES})

	target_link_libraries(interrelations_test
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})

	target_link_libraries(nn_graph_dump
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})

	target_link_libraries(perf_test
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})

	target_link_libraries(complex_system
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})

	target_link_libraries(prey_predator
		agio_lib
		Boost::serialization
		${SFML_LIBRARIES})

	target_link_libraries(experiment_3d
		agio_lib
		Boost::serialization
		${SFML_LIBRARIES})

	# Some tests are Windows specific
	if (WIN32)
		add_executable(human_test
			src/Tests/ComplexSystem/HumanTest.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/ComplexSystemEvolution.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)

		add_executable(memory_test
			src/Tests/ComplexSystem/MemoryTest.cpp
			src/Tests/ComplexSystem/ComplexSystem.cpp
			src/Tests/ComplexSystem/PublicInterfaceImpl.cpp)

		target_link_libraries(human_test
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})

		target_link_libraries(memory_test
			agio_lib
			Boost::serialization
			${SFML_LIBRARIES})
	endif()
endif()
