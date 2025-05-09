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
 ** \brief Source for the IFunction_definition implementation.
 **/

#include "pch.h"

#include <mi/base/handle.h>

#include "neuray_expression_impl.h"
#include "neuray_function_call_impl.h"
#include "neuray_function_definition_impl.h"
#include "neuray_mdl_execution_context_impl.h"
#include "neuray_transaction_impl.h"
#include "neuray_type_impl.h"

#include <mi/neuraylib/inumber.h>
#include <io/scene/mdl_elements/i_mdl_elements_function_call.h>
#include <io/scene/mdl_elements/i_mdl_elements_function_definition.h>

namespace MI {

namespace NEURAY {

DB::Element_base* Function_definition_impl::create_db_element(
    mi::neuraylib::ITransaction* transaction,
    mi::Uint32 argc,
    const mi::base::IInterface* argv[])
{
    if( !transaction)
        return nullptr;
    if( argc != 0)
        return nullptr;
    return new MDL::Mdl_function_definition;
}

mi::base::IInterface* Function_definition_impl::create_api_class(
    mi::neuraylib::ITransaction* transaction,
    mi::Uint32 argc,
    const mi::base::IInterface* argv[])
{
    if( !transaction)
        return nullptr;
    if( argc != 0)
        return nullptr;
    return (new Function_definition_impl())->cast_to_major();
}

mi::neuraylib::Element_type Function_definition_impl::get_element_type() const
{
    return mi::neuraylib::ELEMENT_TYPE_FUNCTION_DEFINITION;
}

const char* Function_definition_impl::get_module() const
{
    return get_db_element()->get_module_db_name();
}

const char* Function_definition_impl::get_mdl_name() const
{
    return get_db_element()->get_mdl_name();
}

const char* Function_definition_impl::get_mdl_module_name() const
{
    return get_db_element()->get_mdl_module_name();
}

const char* Function_definition_impl::get_mdl_simple_name() const
{
    return get_db_element()->get_mdl_simple_name();
}

const char* Function_definition_impl::get_mdl_parameter_type_name( Size index) const
{
    return get_db_element()->get_mdl_parameter_type_name( index);
}

const char* Function_definition_impl::get_prototype() const
{
    DB::Tag tag = get_db_element()->get_prototype();
    return get_db_transaction()->tag_to_name( tag);
}

void Function_definition_impl::get_mdl_version(
    mi::neuraylib::Mdl_version& since, mi::neuraylib::Mdl_version& removed) const
{
    return get_db_element()->get_mdl_version( since, removed);
}

mi::neuraylib::IFunction_definition::Semantics Function_definition_impl::get_semantic() const
{
    return get_db_element()->get_semantic();
}

bool Function_definition_impl::is_exported() const
{
    return get_db_element()->is_exported();
}

bool Function_definition_impl::is_declarative() const
{
    return get_db_element()->is_declarative();
}

bool Function_definition_impl::is_uniform() const
{
    return get_db_element()->is_uniform();
}

bool Function_definition_impl::is_material() const
{
    return get_db_element()->is_material();
}

const mi::neuraylib::IType* Function_definition_impl::get_return_type() const
{
    mi::base::Handle<Type_factory> tf( get_transaction()->get_type_factory());
    mi::base::Handle<const MDL::IType> result_int( get_db_element()->get_return_type());
    return tf->create( result_int.get(), this->cast_to_major());
}

mi::Size Function_definition_impl::get_parameter_count() const
{
    return get_db_element()->get_parameter_count();
}

const char* Function_definition_impl::get_parameter_name( mi::Size index) const
{
    return get_db_element()->get_parameter_name( index);
}

mi::Size Function_definition_impl::get_parameter_index( const char* name) const
{
    return get_db_element()->get_parameter_index( name);
}

const mi::neuraylib::IType_list* Function_definition_impl::get_parameter_types() const
{
    mi::base::Handle<Type_factory> tf( get_transaction()->get_type_factory());
    mi::base::Handle<const MDL::IType_list> result_int( get_db_element()->get_parameter_types());
    return tf->create_type_list( result_int.get(), this->cast_to_major());
}

const mi::neuraylib::IExpression_list* Function_definition_impl::get_defaults() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IExpression_list> result_int( get_db_element()->get_defaults());
    return ef->create_expression_list( result_int.get(), this->cast_to_major());
}

const mi::neuraylib::IExpression_list* Function_definition_impl::get_enable_if_conditions() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IExpression_list> result_int(
        get_db_element()->get_enable_if_conditions());
    return ef->create_expression_list(result_int.get(), this->cast_to_major());
}

mi::Size Function_definition_impl::get_enable_if_users(mi::Size index) const
{
    return get_db_element()->get_enable_if_users(index);
}

mi::Size Function_definition_impl::get_enable_if_user(mi::Size index, mi::Size u_index) const
{
    return get_db_element()->get_enable_if_user(index, u_index);
}

const mi::neuraylib::IAnnotation_block* Function_definition_impl::get_annotations() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IAnnotation_block> result_int(
        get_db_element()->get_annotations());
    return ef->create_annotation_block( result_int.get(), this->cast_to_major());
}

const mi::neuraylib::IAnnotation_block* Function_definition_impl::get_return_annotations() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IAnnotation_block> result_int(
        get_db_element()->get_return_annotations());
    return ef->create_annotation_block( result_int.get(), this->cast_to_major());
}

const mi::neuraylib::IAnnotation_list* Function_definition_impl::get_parameter_annotations() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IAnnotation_list> result_int(
        get_db_element()->get_parameter_annotations());
    return ef->create_annotation_list( result_int.get(), this->cast_to_major());
}

const char* Function_definition_impl::get_thumbnail() const
{
    m_cached_thumbnail = get_db_element()->get_thumbnail();
    return !m_cached_thumbnail.empty() ? m_cached_thumbnail.c_str() : nullptr;
}

bool Function_definition_impl::is_valid( mi::neuraylib::IMdl_execution_context* context) const
{
    MDL::Execution_context default_context;
    MDL::Execution_context *mdl_context = unwrap_and_clear_context(context, default_context);

    return  get_db_element()->is_valid(get_db_transaction(), mdl_context);
}

const mi::neuraylib::IExpression* Function_definition_impl::get_body() const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IExpression> result_int(
        get_db_element()->get_body( get_db_transaction()));
    return ef->create<mi::neuraylib::IExpression>(
        result_int.get(), this->cast_to_major());
}

mi::Size Function_definition_impl::get_temporary_count() const
{
    return get_db_element()->get_temporary_count( get_db_transaction());
}

const mi::neuraylib::IExpression* Function_definition_impl::get_temporary( mi::Size index) const
{
    mi::base::Handle<Expression_factory> ef( get_transaction()->get_expression_factory());
    mi::base::Handle<const MDL::IExpression> result_int(
        get_db_element()->get_temporary( get_db_transaction(), index));
    return ef->create( result_int.get(), this->cast_to_major());
}

const char* Function_definition_impl::get_temporary_name( mi::Size index) const
{
    return get_db_element()->get_temporary_name( get_db_transaction(), index );
}

mi::neuraylib::IFunction_call* Function_definition_impl::create_function_call(
    const mi::neuraylib::IExpression_list* arguments, mi::Sint32* errors) const
{
    mi::base::Handle<const MDL::IExpression_list> arguments_int(
        get_internal_expression_list( arguments));
    std::shared_ptr<MDL::Mdl_function_call> db_call(
        get_db_element()->create_function_call( get_db_transaction(), arguments_int.get(), errors));
    if( !db_call)
        return nullptr;

    auto* api_call = get_transaction()->create<mi::neuraylib::IFunction_call>(  "__Function_call");
    static_cast<Function_call_impl* >( api_call)->get_db_element()->swap( *db_call);
    return api_call;
}

const char* Function_definition_impl::get_mangled_name() const
{
    m_cached_mangled_name = get_db_element()->get_mangled_name( get_db_transaction());
    return m_cached_mangled_name.c_str();
}

} // namespace NEURAY

} // namespace MI
