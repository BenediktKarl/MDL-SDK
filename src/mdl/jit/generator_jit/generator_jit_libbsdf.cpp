/***************************************************************************************************
 * Copyright (c) 2017-2025, NVIDIA CORPORATION. All rights reserved.
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
 **************************************************************************************************/
/// \file

#include "pch.h"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>

#include "mdl/compiler/compilercore/compilercore_tools.h"
#include "mdl/compiler/compilercore/compilercore_assert.h"
#include "mdl/compiler/compilercore/compilercore_errors.h"

#include "generator_jit_llvm.h"

#include "libbsdf_bitcode_hsmp.h"
#include "libbsdf_bitcode_hsmn.h"
#include "libbsdf_bitcode_hsm1.h"
#include "libbsdf_bitcode_hsm2.h"
#include "libbsdf_bitcode_hsm4.h"
#include "libbsdf_bitcode_hsm8.h"
#include "libbsdf_bitcode_hsmp_debug.h"
#include "libbsdf_bitcode_hsmn_debug.h"
#include "libbsdf_bitcode_hsm1_debug.h"
#include "libbsdf_bitcode_hsm2_debug.h"
#include "libbsdf_bitcode_hsm4_debug.h"
#include "libbsdf_bitcode_hsm8_debug.h"

namespace mi {
namespace mdl {

// Load the libbsdf LLVM module.
std::unique_ptr<llvm::Module> LLVM_code_generator::load_libbsdf(
    llvm::LLVMContext &llvm_context, mdl::Df_handle_slot_mode hsm)
{
    unsigned char const *bitcode;
    size_t bitcode_size;
    switch (hsm) {
    case DF_HSM_POINTER:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsmp_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsmp_debug);
        } else {
            bitcode = libbsdf_bitcode_hsmp;
            bitcode_size = dimension_of(libbsdf_bitcode_hsmp);
        }
        break;

    case DF_HSM_NONE:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsmn_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsmn_debug);
        } else {
            bitcode = libbsdf_bitcode_hsmn;
            bitcode_size = dimension_of(libbsdf_bitcode_hsmn);
        }
        break;

    case DF_HSM_FIXED_1:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsm1_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm1_debug);
        } else {
            bitcode = libbsdf_bitcode_hsm1;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm1);
        }
        break;

    case DF_HSM_FIXED_2:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsm2_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm2_debug);
        } else {
            bitcode = libbsdf_bitcode_hsm2;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm2);
        }
        break;

    case DF_HSM_FIXED_4:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsm4_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm4_debug);
        } else {
            bitcode = libbsdf_bitcode_hsm4;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm4);
        }
        break;

    case DF_HSM_FIXED_8:
        if (m_enable_full_debug) {
            bitcode = libbsdf_bitcode_hsm8_debug;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm8_debug);
        } else {
            bitcode = libbsdf_bitcode_hsm8;
            bitcode_size = dimension_of(libbsdf_bitcode_hsm8);
        }
        break;

    default:
        MDL_ASSERT(!"Loading libbsdf bytecode version failed");
        return nullptr;
    }

    std::unique_ptr<llvm::MemoryBuffer> mem(llvm::MemoryBuffer::getMemBuffer(
        llvm::StringRef((char const *) bitcode, bitcode_size),
        "libbsdf",
        /*RequiresNullTerminator=*/ false));

    auto mod = llvm::parseBitcodeFile(*mem.get(), llvm_context);
    if (!mod) {
        error(PARSING_LIBBSDF_MODULE_FAILED, Error_params(get_allocator()));
        MDL_ASSERT(!"Parsing libbsdf failed");
        return nullptr;
    }
    return std::move(mod.get());
}

}  // mdl
}  // mi
