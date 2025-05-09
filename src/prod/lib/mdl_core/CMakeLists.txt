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
set(PROJECT_NAME prod-lib-mdl_core)

# collect sources
set(PROJECT_SOURCES 
    "mdl_core_factory.cpp"
    )

# create target from template
create_from_base_preset(
    TARGET ${PROJECT_NAME}
    TYPE SHARED
    SOURCES ${PROJECT_SOURCES}
    EMBED_RC "mdl_core.rc"
    OUTPUT_NAME "mdl_core"
    EXPORTED_SYMBOLS mi_mdl_factory
    )

# expose public header to installed target
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
)

# customize name
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")

if(MACOSX)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so") # corresponding to the binary release
endif()

# target alias for the custom name
add_library(mdl_core ALIAS ${PROJECT_NAME})
add_library(mdl::mdl_core ALIAS ${PROJECT_NAME})

# add mdl and other dependencies
target_add_dependencies(TARGET ${PROJECT_NAME}
    DEPENDS
        boost
        ${LINKER_WHOLE_ARCHIVE}
        ${LINKER_START_GROUP}
        mdl::include-mi
        mdl::mdl-compiler-compilercore
        mdl::mdl-compiler-compiler_glsl
        mdl::mdl-compiler-compiler_hlsl
        mdl::mdl-codegenerators-generator_dag
        mdl::mdl-codegenerators-generator_code
        mdl::mdl-runtime
        mdl::mdl-jit-generator_jit
        mdl::base-lib-libzip
        mdl::base-lib-zlib
        mdl::base-system-version
        ${LINKER_END_GROUP}
        llvm
        ${LINKER_NO_WHOLE_ARCHIVE}
    )

# create installation rules
add_target_install(
    TARGET ${PROJECT_NAME}
    )
