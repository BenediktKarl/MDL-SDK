#*****************************************************************************
# Copyright (c) 2022-2025, NVIDIA CORPORATION. All rights reserved.
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
set(PROJECT_NAME shaders-plugin-openimageio)

# collect sources
set(PROJECT_HEADERS
    "openimageio_image_file_reader_impl.h"
    "openimageio_image_file_writer_impl.h"
    "openimageio_image_plugin_impl.h"
    "openimageio_utilities.h"
    )

set(PROJECT_SOURCES
    "openimageio_image_plugin_impl.cpp"
    "openimageio_image_file_reader_impl.cpp"
    "openimageio_image_file_writer_impl.cpp"
    "openimageio_utilities.cpp"
    ${PROJECT_HEADERS}
    )

# create target from template
create_from_base_preset(
    TARGET ${PROJECT_NAME}
    TYPE SHARED
    SOURCES ${PROJECT_SOURCES}
    EMBED_RC "openimageio.rc"
    OUTPUT_NAME "nv_openimageio"
    EXPORTED_SYMBOLS mi_plugin_factory
    # The undefined symbol check might fail w.r.t. __exp2_finite and __exp_finite if
    # vcpkg and the MDL SDK are built on different systems.
    SKIP_UNDEFINED_SYMBOL_CHECK
)

# customize name
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(MACOSX)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so") # corresponding to the binary release
endif()

# target alias for the custom name
add_library(nv_openimageio ALIAS ${PROJECT_NAME})
add_library(mdl::nv_openimageio ALIAS ${PROJECT_NAME})

# add dependencies other dependencies
target_add_dependencies(TARGET ${PROJECT_NAME}
    DEPENDS
        openimageio
        mdl::base-system-version
    )

# create installation rules
add_target_install(
    TARGET ${PROJECT_NAME}
    )
