set(PROJECT_NAME CSC8503)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "logging/log.h"
    "GameTechRenderer.h"
    "GameTechRendererInterface.h"
    "GameTechVulkanRenderer.h"
    "NetworkedGame.h"
    "NetworkPlayer.h"
    "Oscillator.h"
    "Pane.h"
    "StateGameObject.h"
    "TutorialGame.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "logging/logger.cpp"
  "GameTechRenderer.cpp"
  "GameTechRendererInterface.cpp"
  "GameTechVulkanRenderer.cpp"
    "NetworkedGame.cpp"
    "NetworkPlayer.cpp"
    "Oscillator.cpp"
    "StateGameObject.cpp"
    "TutorialGame.cpp"
)

if(USE_VULKAN)
	file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)

  list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.cpp)

else()
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.cpp)
endif()

source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

if(USE_VULKAN)
    set(ALL_FILES 
        ${Header_Files}
        ${Source_Files}
        ${SHADER_FILES}
    )

foreach (file ${SHADER_FILES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)
	
	if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		continue()
	endif()
		
	if( file_ext STREQUAL  ".vert" OR
		file_ext STREQUAL  ".frag" OR
		file_ext STREQUAL  ".comp" OR
		file_ext STREQUAL  ".geom" OR
		file_ext STREQUAL  ".tesc" OR
		file_ext STREQUAL  ".tese" OR
		file_ext STREQUAL  ".rgen" OR
		file_ext STREQUAL  ".rint" OR
		file_ext STREQUAL  ".rahit" OR
		file_ext STREQUAL  ".rchit" OR
		file_ext STREQUAL  ".rmiss" OR
		file_ext STREQUAL  ".rcall" OR
		file_ext STREQUAL  ".task" OR
		file_ext STREQUAL  ".mesh"
	)
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
        set(SPIRV_OUTPUT ${file_name}.spv)
		set(SPIRV_ABS_INPUT ${file_dir})
		set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		add_custom_command(
			OUTPUT ${SPIRV_ABS_OUTPUT}
			
			COMMENT "Compiling GLSL shader:"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
            DEPENDS ${file}
			VERBATIM
		)
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_ABS_OUTPUT})   
	endif()
endforeach()

endif()

################################################################################
# Target
################################################################################

add_library(${PROJECT_NAME} STATIC
    ${ALL_FILES}
)
add_library(CSC8503::CSC8503 ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        FILE_SET HEADERS
)

set(ROOT_NAMESPACE CSC8503)

include(setupProj)
setupProj(${PROJECT_NAME})

################################################################################
# Dependencies
################################################################################
if(MSVC)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
endif()


target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC GLTFLoader)

if(USE_OPENGL)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)
endif()

if(USE_VULKAN)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC VulkanRendering)

    add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

    add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
    add_compile_definitions(VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=1)

    add_compile_definitions(VK_NO_PROTOTYPES)

    add_custom_target(
        SHADER_FILES
        DEPENDS ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${PROJECT_NAME} SHADER_FILES)
endif()
