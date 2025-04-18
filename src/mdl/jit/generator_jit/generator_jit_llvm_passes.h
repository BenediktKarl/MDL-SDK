/******************************************************************************
 * Copyright (c) 2014-2025, NVIDIA CORPORATION. All rights reserved.
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

#ifndef MDL_GENERATOR_JIT_LLVM_PASSES_H
#define MDL_GENERATOR_JIT_LLVM_PASSES_H 1

namespace llvm {
class FunctionPass;
class ModulePass;
class PassRegistry;

/// Initialize the DeleteUnusedLibDevice pass.
void initializeDeleteUnusedLibDevicePass(PassRegistry &);

/// Initialize the FunctionInstCount pass.
void initializeFunctionInstCounterPass(PassRegistry &);

/// Creates the DeleteUnusedLibDevice pass.
///
/// A very simple pass that removes unused function from libdevice.
/// All functions starting with "__nv_" and marked with AlwaysInline
/// will be removed. Used functions will be marked as internal to avoid exporting them,
/// if AlwaysInline fails for some reason.
ModulePass *createDeleteUnusedLibDevicePass();

/// Creates the NVVM reflect pass.
FunctionPass *createNVVMReflectPass(unsigned int SmVersion);

/// Creates the FunctionInstCount pass.
FunctionPass *createFunctionInstCounterPass();

}  // llvm

#endif // MDL_GENERATOR_JIT_LLVM_PASSES_H
