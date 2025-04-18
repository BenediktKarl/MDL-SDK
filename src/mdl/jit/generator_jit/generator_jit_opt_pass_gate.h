/******************************************************************************
 * Copyright (c) 2018-2025, NVIDIA CORPORATION. All rights reserved.
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

#ifndef MDL_GENERATOR_JIT_OPT_PASS_GATE_H
#define MDL_GENERATOR_JIT_OPT_PASS_GATE_H 1

#include "mdl/compiler/compilercore/compilercore_cc_conf.h"

#include <llvm/IR/OptBisect.h>
#include <llvm/Pass.h>

namespace llvm {
    class Function;
}

namespace mi {
namespace mdl {

/// Helper class to disable LLVM passes generating results we cannot handle in the SL code
/// generation.
class SLOptPassGate : public llvm::OptPassGate {
public:
    /// IRDescription is a textual description of the IR unit the pass is running
    /// over.
    bool shouldRunPass(
        const llvm::Pass *P,
        llvm::StringRef  IRDescription) MDL_FINAL
    {
        // Avoid this pass introducing irregular control flow
        if (P->getPassName() == "Jump Threading") {
            return false;
        }

        // This pass creates i33 values, which we currently cannot translate
        if (P->getPassName() == "Induction Variable Simplification") {
            return false;
        }

        // Let the HLSL compiler do loop unrolling to generate shorter code, not LLVM
        if (P->getPassName() == "Unroll loops") {
            return false;
        }

        // Don't turn while loops into do-while loops
        if (P->getPassName() == "Rotate Loops") {
            return false;
        }

        // Avoid introducing more memset and memcpy calls, which may introduce complex pointers
        if (P->getPassName() == "MemCpy Optimization") {
            return false;
        }

        return true;
    }

    /// isEnabled() should return true before calling shouldRunPass().
    bool isEnabled() const MDL_FINAL { return true; }
};

}  // mdl
}  // mi

#endif // MDL_GENERATOR_JIT_OPT_PASS_GATE_H
