#*****************************************************************************
# Copyright (c) 2020-2025, NVIDIA CORPORATION. All rights reserved.
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

# -------------------------------------------------------------------------------------------------
# script expects the following variables:
# - __TARGET_ADD_DEPENDENCY_TARGET
# - __TARGET_ADD_DEPENDENCY_DEPENDS
# - __TARGET_ADD_DEPENDENCY_COMPONENTS
# - __TARGET_ADD_DEPENDENCY_NO_RUNTIME_COPY
# - __TARGET_ADD_DEPENDENCY_NO_RUNTIME
# - __TARGET_ADD_DEPENDENCY_NO_LINKING
# - __TARGET_ADD_DEPENDENCY_NO_INCLUDE
# -------------------------------------------------------------------------------------------------

# the dependency consists of a set of packages

# assuming the find_* script was successful
# if not, this is an error case. The corresponding project should not have been selected for build.
if(NOT MDL_BUILD_ARNOLD_PLUGIN)
    message(FATAL_ERROR "The dependency \"${__TARGET_ADD_DEPENDENCY_DEPENDS}\" for target \"${__TARGET_ADD_DEPENDENCY_TARGET}\" could not be resolved.")
    return()
endif()

# headers
if(NOT __TARGET_ADD_DEPENDENCY_NO_INCLUDE)
    target_include_directories(${__TARGET_ADD_DEPENDENCY_TARGET} 
        PRIVATE
            ${MDL_DEPENDENCY_ARNOLDSDK_INCLUDE}
        )
endif()

# static library
if(NOT __TARGET_ADD_DEPENDENCY_NO_LINKING AND MDL_DEPENDENCY_ARNOLDSDK_LIBS)
    target_link_libraries(${__TARGET_ADD_DEPENDENCY_TARGET} 
        PRIVATE
            ${MDL_DEPENDENCY_ARNOLDSDK_LIBS}
        )
endif()

# runtime dependencies
if(NOT __TARGET_ADD_DEPENDENCY_NO_RUNTIME AND MDL_DEPENDENCY_ARNOLDSDK_BIN)

    if(WINDOWS)
        target_add_vs_debugger_env_path(TARGET ${__TARGET_ADD_DEPENDENCY_TARGET}
            PATHS 
                ${MDL_DEPENDENCY_ARNOLDSDK_BIN}
            )
    endif()

    # on linux, the user has to setup the LD_LIBRARY_PATH when running examples
    # on mac, DYLD_LIBRARY_PATH, respectively.
endif()
