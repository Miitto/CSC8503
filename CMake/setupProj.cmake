function(setupProj TARGET)
use_props(${TARGET} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

set_target_properties(${TARGET} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${TARGET} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################


if(MSVC)
    target_compile_definitions(${TARGET} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
    )
endif()

target_precompile_headers(${TARGET} PRIVATE
    <vector>
    <map>
    <stack>
    <list>   
	<set>   
	<string>
    <thread>
    <atomic>
    <functional>
    <iostream>
	<chrono>
	<sstream>
	
	"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../NCLCoreClasses/Vector.h"
  "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../NCLCoreClasses/Quaternion.h"
  "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../NCLCoreClasses/Plane.h"
  "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../NCLCoreClasses/Matrix.h"
  "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../NCLCoreClasses/GameTimer.h"
)

if(USE_VULKAN)
target_precompile_headers(${TARGET} PRIVATE
    <vulkan/vulkan.hpp>
    <SmartTypes.h>
    <VulkanRenderer.h>
    #<VulkanShader.h>
    #<VulkanShaderBuilder.h>
    <VulkanTexture.h>
    <VulkanMesh.h>
    <VulkanPipelineBuilder.h>
    <VulkanDynamicRenderBuilder.h>

    <VulkanTextureBuilder.h>

    <VulkanDescriptorSetLayoutBuilder.h>
    <VulkanRenderPassBuilder.h>
    #<VulkanCompute.h>
    <VulkanComputePipelineBuilder.h>
	<VulkanBufferBuilder.h>
)
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${TARGET} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
    )
endif()



endfunction()
