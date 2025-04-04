/***************************************************************************************************
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
 **************************************************************************************************/

#ifndef IO_SCENE_MDL_ELEMENTS_I_MDL_ELEMENTS_FUNCTION_CALL_H
#define IO_SCENE_MDL_ELEMENTS_I_MDL_ELEMENTS_FUNCTION_CALL_H

#include <mi/base/handle.h>
#include <mi/mdl/mdl_definitions.h>
#include <io/scene/scene/i_scene_scene_element.h>

#include "i_mdl_elements_expression.h" // needed by Visual Studio
#include "i_mdl_elements_module.h"

namespace mi { namespace mdl { class IGenerated_code_lambda_function; class IType; } }

namespace MI {

namespace MDL {

class IExpression;
class IExpression_factory;
class IExpression_list;
class IType;
class IType_factory;
class IType_list;
class IValue_factory;
class Execution_context;

/// The class ID for the #Mdl_function_call class.
static const SERIAL::Class_id ID_MDL_FUNCTION_CALL = 0x5f4d6663; // '_Mfc'

class Mdl_function_call
  : public SCENE::Scene_element<Mdl_function_call, ID_MDL_FUNCTION_CALL>
{
public:

    /// Default constructor.
    ///
    /// Does not create a valid instance, to be used by the deserializer only.
    Mdl_function_call();

    /// Constructor.
    ///
    /// \param is_material            Indicates whether the definition is a material or a function.
    /// \param definition_name        MDL name of the corresponding function definition.
    Mdl_function_call(
        DB::Tag module_tag,
        const char* module_db_name,
        DB::Tag definition_tag,
        Mdl_ident definition_ident,
        bool is_declarative,
        bool is_material,
        IExpression_list* arguments,
        mi::mdl::IDefinition::Semantics semantic,
        const char* definition_name,
        const IType_list* parameter_types,
        const IType* return_type,
        bool immutable,
        const IExpression_list* enable_if_conditions);

    // Copy constructor
    Mdl_function_call( const Mdl_function_call& other);

    Mdl_function_call& operator=( const Mdl_function_call&) = delete;

    // methods corresponding to mi::neuraylib::IFunction_call

    DB::Tag get_function_definition( DB::Transaction* transaction) const;

    const char* get_mdl_function_definition() const;

    bool is_declarative() const { return m_is_declarative; }

    bool is_material() const { return m_is_material; }

    const IType* get_return_type() const;

    mi::Size get_parameter_count() const;

    const char* get_parameter_name( mi::Size index) const;

    mi::Size get_parameter_index( const char* name) const;

    const IType_list* get_parameter_types() const;

    const IExpression_list* get_arguments() const;

    mi::Sint32 set_arguments( DB::Transaction* transaction, const IExpression_list* arguments);

    mi::Sint32 set_argument(
        DB::Transaction* transaction, mi::Size index, const IExpression* argument);

    mi::Sint32 set_argument(
        DB::Transaction* transaction, const char* name, const IExpression* argument);

    mi::Sint32 reset_argument( DB::Transaction* transaction, mi::Size index);

    mi::Sint32 reset_argument( DB::Transaction* transaction, const char* name);

    const IExpression_list* get_enable_if_conditions() const;

    /// \param target_type                 The intended type of the compiled material. Needs to be
    ///                                    from the struct category "::material_category". Can be
    ///                                    \c nullptr, which indicates the same type as this DB
    ///                                    material instance itself.
    Mdl_compiled_material* create_compiled_material(
        DB::Transaction* transaction,
        bool class_compilation,
        const IType_struct* target_type,
        Execution_context* context) const;

    // internal methods

    /// Indicates whether the function call is immutable.
    bool is_immutable() const { return m_immutable; }

    ///
    /// This method may only be set to \c true by the MDL integration itself, not by external
    /// callers.
    void make_mutable( DB::Transaction* transaction);

    /// Returns the core semantic of the corresponding definition.
    mi::mdl::IDefinition::Semantics get_core_semantic() const;

    /// Returns the module tag of the call's definition.
    DB::Tag get_module( DB::Transaction* transaction) const;

    /// Indicates whether the corresponding definition is valid.
    bool is_valid_definition( DB::Transaction* transaction) const;

    /// Returns the identifier of the definition.
    Mdl_ident get_definition_ident() const { return m_definition_ident; }

    /// Returns the DB name of the definition.
    const char* get_definition_db_name() const { return m_definition_db_name.c_str(); }

    /// Swaps *this and \p other.
    ///
    /// Used by the API to move the content of just constructed DB elements into the already
    /// existing API wrapper.
    void swap( Mdl_function_call& other);

    /// Creates a jitted function.
    ///
    /// \param transaction                 The transaction.
    /// \param mdl_meters_per_scene_unit   Conversion ratio between meters and scene units.
    /// \param mdl_wavelength_min          The smallest supported wavelength.
    /// \param mdl_wavelength_max          The largest supported wavelength.
    /// \param errors                      An optional pointer to an mi::Sint32 to which an error
    ///                                    code will be written. The error codes have the following
    ///                                    meaning:
    ///                                    -  0: Success.
    ///                                    - -1: The JIT backend is not available.
    ///                                    - -2: The function has the wrong return type (needs to be
    ///                                           \c "Spectrum").
    ///                                    - -3: An argument of the function call has an incorrect
    ///                                          type.
    ///                                    - -4: The JIT backend failed to compile the function.
    ///                                    - -5: The function does not point to a valid definition.
    /// \return                            The corresponding compiled function, or \c nullptr in
    ///                                    case of failure.
    mi::mdl::IGenerated_code_lambda_function* create_jitted_function(
        DB::Transaction* transaction,
        bool environment_context,
        mi::Float32 mdl_meters_per_scene_unit,
        mi::Float32 mdl_wavelength_min,
        mi::Float32 mdl_wavelength_max,
        Sint32* errors = nullptr) const;

    /// Creates a DAG material instance for this DB material instance.
    ///
    /// \param transaction                 The transaction.
    /// \param use_temporaries             Indicates whether temporaries are used to represent
    ///                                    common subexpressions.
    /// \param class_compilation           Flag that selects class compilation instead of instance
    ///                                    compilation.
    /// \param target_type                 The intended type of the compiled material. Needs to be
    ///                                    from the struct category "::material_category". Can be
    ///                                    \c nullptr, which indicates the same type as this DB
    ///                                    material instance itself.
    /// \param context                     execution context to propagate warnings and error
    ///                                    messages. The error codes have the following
    ///                                    meaning:
    ///                                    -  0: Success.
    ///                                    - -1: An argument of the material instance has an
    ///                                          incorrect type.
    ///                                    - -3: An varying argument was attached to an uniform
    ///                                          parameter.
    /// \return                            The DAG material instance, or \c nullptr in case of
    ///                                    failure.
    ///
    /// \note If it is detected, that a thin-walled material instance has different
    ///       transmission for surface and backface, a warning is generated.
    const mi::mdl::IMaterial_instance* create_dag_material_instance(
        DB::Transaction* transaction,
        bool use_temporaries,
        bool class_compilation,
        const IType_struct* target_type,
        Execution_context* context) const;

    /// Improved version of SERIAL::Serializable::dump().
    ///
    /// \param transaction   The DB transaction (for name lookups and tag versions). Can be
    ///                      \c nullptr.
    void dump( DB::Transaction* transaction) const;

    /// Checks, if the function call and its arguments still refer to valid definitions.
    bool is_valid(
        DB::Transaction* transaction,
        Execution_context* context) const;

    /// Checks, if the function call and its arguments still refer to valid definitions.
    bool is_valid(
        DB::Transaction* transaction,
        DB::Tag_set& tags_seen,
        Execution_context* context) const;

    /// Attempts to repair an invalid function call by trying to promote its definition
    /// tag identifier.
    /// \param transaction              the DB transaction.
    /// \param repair_invalid_calls     \c true, if invalid calls should be removed.
    /// \param remove_invalid_calls     \c true, if invalid calls should be repaired.
    /// \param level                    the recursion level.
    /// \param context                  will receive error messages.
    /// \return
    ///         -  0: Success.
    ///         - -1: Failure. Consult the context for details.
    mi::Sint32 repair(
        DB::Transaction* transaction,
        bool repair_invalid_calls,
        bool remove_invalid_calls,
        mi::Uint32 level,
        Execution_context* context);

    // methods of SERIAL::Serializable

    const SERIAL::Serializable* serialize( SERIAL::Serializer* serializer) const;

    SERIAL::Serializable* deserialize( SERIAL::Deserializer* deserializer);

    void dump() const { dump( /*transaction*/ nullptr); }

    // methods of DB::Element_base

    size_t get_size() const;

    DB::Journal_type get_journal_flags() const;

    Uint bundle( DB::Tag* results, Uint size) const;

    // methods of SCENE::Scene_element_base

    void get_scene_element_references( DB::Tag_set* result) const;

private:

    static mi::Sint32 repair_call(
        DB::Transaction* transaction,
        const DB::Access<Mdl_function_call>& call_access,
        mi::base::Handle<IExpression>& new_expr,
        const IExpression* default_expr,
        const IExpression_call* arg_call,
        IExpression_factory* ef,
        IValue_factory* vf,
        bool repair_invalid_calls,
        bool remove_invalid_calls,
        mi::Uint32 level,
        Execution_context* context);

    /// Repairs the given argument list according to the given rules.
    ///
    /// \param transaction  the transaction.
    /// \param arguments    the arguments to repair.
    /// \param defaults     argument defaults.
    /// \param repair_calls if \c true, invalid calls will be repaired, if possible.
    /// \param remove_calls if \c true, invalid calls will be replaced by a default. In case \p
    ///                     repair_calls is \c true, the call will be removed if the repair
    ///                     failed.
    /// \param level        recursion level
    /// \param context      execution context to propagate error messages.
    /// \return
    ///      -  0:   Success
    ///      - -1:   Repairing failed. See the context for details.
    static mi::Sint32 repair_arguments(
        DB::Transaction* transaction,
        IExpression_list* arguments,
        const IExpression_list* defaults,
        bool repair_calls,
        bool remove_calls,
        mi::Uint32 level,
        Execution_context* context);

    mi::base::Handle<IType_factory> m_tf;        ///< The type factory.
    mi::base::Handle<IValue_factory> m_vf;       ///< The value factory.
    mi::base::Handle<IExpression_factory> m_ef;  ///< The expression factory.

    // Members marked with (*) are duplicated from the corresponding function definition to avoid
    // frequent DB accesses.

    DB::Tag m_module_tag;                        ///< The corresponding MDL module. (*)
    DB::Tag m_definition_tag;                    ///< The corresponding function definition.
    Mdl_ident m_definition_ident;                ///< The corresponding function definition
                                                 ///  identifier.
    mi::mdl::IDefinition::Semantics m_core_semantic; ///< The semantic. (*)
    std::string m_module_db_name;                ///< The DB name of the module. (*)
    std::string m_definition_name;               ///< The MDL name of the function definition. (*)
    std::string m_definition_db_name;            ///< The DB name of the function definition. (*)
    bool m_is_declarative;                       ///< The declarative flag. (*)
    bool m_is_material;                          ///< Material or function definition (*)
    bool m_immutable;                            ///< The immutable flag (set for defaults).

    mi::base::Handle<const IType_list> m_parameter_types;            // (*)
    mi::base::Handle<const IType> m_return_type;                     // (*)
    mi::base::Handle<IExpression_list> m_arguments;
    mi::base::Handle<const IExpression_list> m_enable_if_conditions; // (*)
};

} // namespace MDL

} // namespace MI

#endif // IO_SCENE_MDL_ELEMENTS_I_MDL_ELEMENTS_FUNCTION_CALL_H
