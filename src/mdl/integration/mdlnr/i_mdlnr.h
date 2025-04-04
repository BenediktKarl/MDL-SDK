/******************************************************************************
 * Copyright (c) 2012-2025, NVIDIA CORPORATION. All rights reserved.
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

#ifndef MDL_INTERGRATION_I_MDLNR_H
#define MDL_INTERGRATION_I_MDLNR_H 1

#include <base/system/main/i_module.h>
#include <mi/mdl/mdl_generated_dag.h>

namespace mi {
    namespace mdl {
        class ICode_cache;
        class IMDL;
        class IModule;
//        class IGenerated_code_dag;
        class ILambda_function;
    }
}

namespace MI {

namespace MDL { class Mdl_module_wait_queue; }
namespace SYSTEM { class Module_registration_entry; }
namespace SERIAL { class Deserializer; class Serializer; }

namespace MDLC {

/// The interface of the MDL compiler module.
class Mdlc_module : public SYSTEM::IModule
{

public:

    /// Get an instance of the module.
    static SYSTEM::Module_registration_entry *get_instance();

    /// Get the name of the module.
    static const char *get_name() { return "MDLC"; }

    /// Get the mdl interface.
    virtual mi::mdl::IMDL *get_mdl() const = 0;

    /// Serializes the module to the given serializer.
    virtual void serialize_module(
        SERIAL::Serializer *serializer, const mi::mdl::IModule *module) = 0;

    /// Deserializes the module from the given deserializer.
    virtual const mi::mdl::IModule *deserialize_module(SERIAL::Deserializer *deserializer) = 0;

    /// Serializes the code DAG to the given serializer.
    virtual void serialize_code_dag(
        SERIAL::Serializer *serializer, const mi::mdl::IGenerated_code_dag *code) = 0;

    /// Deserializes the code DAG from the given deserializer.
    virtual const mi::mdl::IGenerated_code_dag *deserialize_code_dag(
        SERIAL::Deserializer *deserializer) = 0;

    /// Serializes the material instance to the given serializer.
    virtual void serialize_material_instance(
        SERIAL::Serializer *serializer, const mi::mdl::IMaterial_instance *instance) = 0;

    /// Deserializes the material instance from the given deserializer.
    virtual const mi::mdl::IMaterial_instance *deserialize_material_instance(SERIAL::Deserializer *deserializer) = 0;

    /// Serializes the lambda function to the given serializer.
    virtual void serialize_lambda_function(
        SERIAL::Serializer *serializer, const mi::mdl::ILambda_function *lambda) = 0;

    /// Deserializes the lambda function from the given deserializer.
    virtual mi::mdl::ILambda_function *deserialize_lambda_function(
        SERIAL::Deserializer *deserializer) = 0;

    /// Get the MDL code cache.
    virtual mi::mdl::ICode_cache *get_code_cache() const = 0;

    /// Configures, whether casts for compatible types should be inserted by the integration
    /// when needed.
    virtual void set_implicit_cast_enabled(bool value) = 0;

    /// Returns, whether casts for compatible types should be inserted by the integration
    /// when needed.
    virtual bool get_implicit_cast_enabled() const = 0;

    /// Defines whether an attempt is made to expose names of let expressions.
    virtual void set_expose_names_of_let_expressions( bool value) = 0;

    /// Indicates whether an attempt is made to expose names of let expressions.
    virtual bool get_expose_names_of_let_expressions() const = 0;

    /// Returns the module wait queue.
    virtual MDL::Mdl_module_wait_queue* get_module_wait_queue() const = 0;
};

} // namespace MDLC

} // namespace MI

#endif // MDL_INTERGRATION_I_MDLNR_H
