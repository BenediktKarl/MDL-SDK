/******************************************************************************
 * Copyright (c) 2015-2025, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
/// \file mi/mdl/mdl_stdlib_types.h
/// \brief Predefined enum types from the MDL standard library
#ifndef MDL_STDLIB_TYPES_H
#define MDL_STDLIB_TYPES_H 1

namespace mi {
namespace mdl {

/// This namespace contains enum types that mimics MDL predefined enum types.
namespace stdlib {

/// The MDL tex::gamma_mode enum.
enum Tex_gamma_mode {
    gamma_default = 0,
    gamma_linear  = 1,
    gamma_srgb    = 2
};

/// The tex::wrap_mode enum.
enum Tex_wrap_mode {
    wrap_clamp           = 0,
    wrap_repeat          = 1,
    wrap_mirrored_repeat = 2,
    wrap_clip            = 3
};

/// MBSDFs can consist of two parts, which can be selected using this enumeration.
enum Mbsdf_part
{
    mbsdf_data_reflection = 0,
    mbsdf_data_transmission = 1
};

} // stdlib
} // mdl
} // mi

#endif
