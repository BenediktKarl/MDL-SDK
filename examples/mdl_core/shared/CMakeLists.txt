#*****************************************************************************
# Copyright (c) 2018-2025, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#*****************************************************************************

# name of the target and the resulting library
set(PROJECT_NAME examples-mdl_core-shared)

# add dummy-source to create a dummy-lib
set(DUMMY_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated/empty.cpp)
if(NOT EXISTS ${DUMMY_CPP})
    file(WRITE ${DUMMY_CPP} "")
endif()

# collect sources
set(PROJECT_SOURCES
    "example_cuda_shared.h"
    "example_shared.h"
    "example_shared.cpp"
    "example_shared_backends.h"
    "texture_support_native.h"
    "texture_support_cuda.h"
    ${DUMMY_CPP}
    )

if(MDL_ENABLE_VULKAN_EXAMPLES)
    set(PROJECT_SOURCES
        ${PROJECT_SOURCES}
        "example_vulkan_shared.h"
        "example_vulkan_shared.cpp"
        )
endif()

# create target from template
create_from_base_preset(
    TARGET ${PROJECT_NAME}
    NAMESPACE mdl_core
    OUTPUT_NAME "shared"
    SOURCES ${PROJECT_SOURCES}
    EXAMPLE
    STATIC_MSVC_RUNTIME
)
add_library(mdl_core::shared ALIAS ${PROJECT_NAME})

# inject the MDL_SAMPLES_ROOT path
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        "MDL_SAMPLES_ROOT=\"${MDL_EXAMPLES_FOLDER}\""
    )

# add dependencies
target_add_dependencies(TARGET ${PROJECT_NAME}
    DEPENDS
        mdl::mdl_core
    NO_RUNTIME_COPY
    )

if(MDL_ENABLE_VULKAN_EXAMPLES)
    target_add_dependencies(TARGET ${PROJECT_NAME}
    DEPENDS
        glfw
        vulkan
        openimageio
    NO_RUNTIME_COPY
    )
endif()

# add include directories
target_include_directories(${PROJECT_NAME}
    INTERFACE
        # the template makes non public headers private by default
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
