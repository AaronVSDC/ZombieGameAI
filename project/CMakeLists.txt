# ADD NEW .cpp FILES HERE
add_library(Exam_Plugin SHARED
	"stdafx.cpp"
	"SurvivalAgentPlugin.cpp" "SteeringBehaviours.cpp" "SteeringBehaviours.h" "Grid.cpp" "Grid.h" "FiniteStateMachine.h" "FiniteStateMachine.cpp" "StateDecider.h" "StateDecider.cpp" "States.h" "Blackboard.h")

target_link_libraries(Exam_Plugin PUBLIC ${EXAM_LIB_DEBUG})
target_include_directories(Exam_Plugin PUBLIC ${EXAM_INCLUDE_DIR})

# Explicit debug info generation flags for MSVC
target_compile_options(Exam_Plugin PRIVATE
  $<$<CONFIG:Debug>:/Zi>)
target_link_options(Exam_Plugin PRIVATE
  $<$<CONFIG:Debug>:/DEBUG>)

set_target_properties(Exam_Plugin PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${CMAKE_SOURCE_DIR}/_DEMO_DEBUG"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${CMAKE_SOURCE_DIR}/_DEMO_DEBUG"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${CMAKE_SOURCE_DIR}/_DEMO_DEBUG"
	
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/_DEMO_RELEASE"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/_DEMO_RELEASE"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/_DEMO_RELEASE"
	
	OUTPUT_NAME_DEBUG "GPP_Plugin_d"
    OUTPUT_NAME_RELEASE "GPP_Plugin")

# Add the pre-built .exe as an imported target
add_executable(GPP_EXAM_EXE IMPORTED)

# Set paths to the .exe in Debug/Release directories
set_target_properties(GPP_EXAM_EXE PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${CMAKE_SOURCE_DIR}/_DEMO_DEBUG/GPP_EXAM_DEBUG.exe"
    IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/_DEMO_RELEASE/GPP_EXAM_RELEASE.exe"
)

# Ensure the DLL is built before the .exe is "built"
add_dependencies(GPP_EXAM_EXE Exam_Plugin)
