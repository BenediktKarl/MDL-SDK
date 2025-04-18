/***************************************************************************************************
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
 **************************************************************************************************/

/** \file
 ** \brief Header for the IFunction_definition implementation.
 **/

#ifndef API_API_NEURAY_NEURAY_FUNCTION_DEFINITION_IMPL_H
#define API_API_NEURAY_NEURAY_FUNCTION_DEFINITION_IMPL_H

#include <mi/neuraylib/ifunction_definition.h>

#include "neuray_db_element_impl.h"
#include "neuray_attribute_set_impl.h"

namespace MI {

namespace MDL { class Mdl_function_definition; }

namespace NEURAY {

/// This class implements an MDL function definition.
class Function_definition_impl final
    : public Attribute_set_impl<Db_element_impl<mi::neuraylib::IFunction_definition,
                                                MDL::Mdl_function_definition> >
{
public:

    static DB::Element_base* create_db_element(
        mi::neuraylib::ITransaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    static mi::base::IInterface* create_api_class(
        mi::neuraylib::ITransaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    // public API methods (IScene_element)

    mi::neuraylib::Element_type get_element_type() const final;

    // public API methods (IFunction_definition)

    const char* get_module() const final;

    const char* get_mdl_name() const final;

    const char* get_mdl_module_name() const final;

    const char* get_mdl_simple_name() const final;

    const char* get_mdl_parameter_type_name( Size index) const final;

    const char* get_prototype() const final;

    void get_mdl_version(
        mi::neuraylib::Mdl_version& since, mi::neuraylib::Mdl_version& removed) const final;

    mi::neuraylib::IFunction_definition::Semantics get_semantic() const final;

    bool is_exported() const final;

    bool is_declarative() const final;

    bool is_uniform() const final;

    bool is_material() const final;

    const mi::neuraylib::IType* get_return_type() const final;

    mi::Size get_parameter_count() const final;

    const char* get_parameter_name( mi::Size index) const final;

    mi::Size get_parameter_index( const char* name) const final;

    const mi::neuraylib::IType_list* get_parameter_types() const final;

    const mi::neuraylib::IExpression_list* get_defaults() const final;

    const mi::neuraylib::IExpression_list* get_enable_if_conditions() const final;

    mi::Size get_enable_if_users( mi::Size index) const final;

    mi::Size get_enable_if_user( mi::Size index, mi::Size u_index) const final;

    const mi::neuraylib::IAnnotation_block* get_annotations() const final;

    const mi::neuraylib::IAnnotation_block* get_return_annotations() const final;

    const mi::neuraylib::IAnnotation_list* get_parameter_annotations() const final;

    const char* get_thumbnail() const final;

    bool is_valid( mi::neuraylib::IMdl_execution_context* context) const final;

    const mi::neuraylib::IExpression* get_body() const final;

    mi::Size get_temporary_count() const final;

    const mi::neuraylib::IExpression* get_temporary( mi::Size index) const final;

    const char* get_temporary_name( mi::Size index) const final;

    mi::neuraylib::IFunction_call* create_function_call(
        const mi::neuraylib::IExpression_list* arguments,
        mi::Sint32* errors) const final;

    const char* get_mangled_name() const final;

private:
    mutable std::string m_cached_thumbnail;
    mutable std::string m_cached_mangled_name;
};

} // namespace NEURAY

} // namespace MI

#endif // API_API_NEURAY_NEURAY_FUNCTION_DEFINITION_IMPL_H
