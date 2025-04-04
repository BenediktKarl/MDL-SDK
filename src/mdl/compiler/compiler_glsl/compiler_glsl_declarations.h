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

#ifndef MDL_COMPILER_GLSL_DECLARATIONS_H
#define MDL_COMPILER_GLSL_DECLARATIONS_H 1

#include <cstddef>

#include <mi/mdl/mdl_iowned.h>

#include <mdl/compiler/compilercore/compilercore_memory_arena.h>
#include <mdl/compiler/compilercore/compilercore_ast_list.h>

#include "compiler_glsl_cc_conf.h"
#include "compiler_glsl_locations.h"
#include "compiler_glsl_assert.h"

namespace mi {
namespace mdl {
namespace glsl {

class Definition;
class Expr;
class Stmt;
class Symbol;
class Type;

// forward.
class Declaration;


/// GLSL storage qualifier.
enum Storage_qualifier {
    SQ_NONE      = 0,       ///< None set.
    SQ_CONST     = 1 <<  0, ///< a variable whose value cannot be changed
    SQ_IN        = 1 <<  1, ///< linkage into a shader from a previous stage, variable is copied in
    SQ_OUT       = 1 <<  2, ///< linkage out of a shader to a subsequent stage, variable is
    SQ_INOUT     = SQ_IN | SQ_OUT,
    ///  copied out
    SQ_ATTRIBUTE = 1 <<  3, ///< compatibility profile only and vertex language only; same as in
    ///  when in a vertex shader
    SQ_UNIFORM   = 1 <<  4, ///< value does not change across the primitive being processed,
    ///  uniforms form the linkage between a shader, OpenGL, and the
    ///  application
    SQ_VARYING   = 1 <<  5, ///< compatibility profile only and vertex and fragment languages only;
    ///  same as out when in a vertex shader and same as in when in a
    ///  fragment shader
    SQ_BUFFER    = 1 <<  6, ///< value is stored in a buffer object, and can be read or written
    ///  both by shader invocations and the OpenGL API
    SQ_SHARED    = 1 <<  7, ///< compute shader only; variable storage is shared across all work
    ///  items in a local work group
    SQ_CENTROID  = 1 <<  8, ///< centroid-based interpolation
    SQ_SAMPLE    = 1 <<  9, ///< per-sample interpolation
    SQ_PATCH     = 1 << 10, ///< per-tessellation-patch attributes
    SQ_COHERENT  = 1 << 11, ///< coherent reads and writes from other shader invocations
    SQ_VOLATILE  = 1 << 12, ///< volatile memory access
    SQ_RESTRICT  = 1 << 13, ///< alias-free access
    SQ_READONLY  = 1 << 14, ///< read-only access
    SQ_WRITEONLY = 1 << 15, ///< write-only access
}; // can be or'ed

/// GLSL interpolation qualifiers.
enum Interpolation_qualifier {
    IQ_NONE,          ///< None set.
    IQ_SMOOTH,        ///< perspective correct interpolation
    IQ_FLAT,          ///< no interpolation
    IQ_NOPERSPECTIVE  ///> linear interpolation
};

/// GLSL parameter qualifiers.
///
/// \note The parameter qualifiers are mapped to the corresponding storage qualifiers.
enum Parameter_qualifier {
    PQ_CONST = SQ_CONST,  ///< for function parameters that cannot be written to
    PQ_IN    = SQ_IN,     ///< for function parameters passed into a function
    PQ_OUT   = SQ_OUT,    ///< for function parameters passed back out of a function,
                          ///< but not initialized
    PQ_INOUT = SQ_INOUT,  ///< for function parameters passed both into and out of a function
};

/// GLSL precision qualifiers.
enum Precision_qualifier {
    PQ_NONE,     ///< None set.
    PQ_HIGHP,
    PQ_MEDIUMP,
    PQ_LOWP,
};

/// Implementation of a GLSL name.
class Name : public Interface_owned
{
    typedef Interface_owned Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Get the symbol if any.
    ///
    /// \return the symbol of this name or NULL if this name represents an unnamed
    ///         struct, get_struct_decl() must not return NULL in that case
    Symbol *get_symbol() const { return m_sym; }

    /// Get the location of this simple name.
    Location const &get_location() { return m_loc; }

    /// Get the type of this name.
    Type *get_type();

    /// Get the definition for this name.
    Definition *get_definition() const { return m_def; }

    /// Set the definition for this name.
    void set_definition(Definition *def);

private:
    /// Constructor.
    ///
    /// \param loc  the location of this name
    /// \param sym  the symbol of this name.
    explicit Name(
        Location const &loc,
        Symbol         *sym);

private:
    // non copyable
    Name(Name const &) GLSL_DELETED_FUNCTION;
    Name &operator=(Name const &) GLSL_DELETED_FUNCTION;

private:
    /// The location of this name.
    Location const m_loc;

    /// The symbol of this name.
    Symbol * const m_sym;

    /// The definition of this name if any.
    Definition *m_def;
};

/// A GLSL layout qualifier id.
class Layout_qualifier_id : public Ast_list_element<Layout_qualifier_id>
{
    typedef Ast_list_element<Layout_qualifier_id> Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Check if this is a shared layout qualifier id.
    bool is_shared() const { return m_ident == NULL; }

    /// Get the identifier of this layout qualifier id.
    Name *get_identifier() { return m_ident; }

    /// Get the identifier of this layout qualifier id.
    Name const *get_identifier() const { return m_ident; }

    /// Get the expression of the identifier layout qualifier id if any.
    Expr *get_expr() { return m_expr; }

    /// Get the expression of the identifier layout qualifier id if any.
    Expr const *get_expr() const { return m_expr; }

private:
    /// Constructor.
    ///
    /// \param loc         the location of this layout qualifier id
    /// \param identifier  if non-NULL, the identifier of this id
    /// \param expr        if non-NULL, the expression of this id
    explicit Layout_qualifier_id(
        Location const &loc,
        Name           *identifier,
        Expr           *expr);

private:
    // non copyable
    Layout_qualifier_id(Layout_qualifier_id const &) GLSL_DELETED_FUNCTION;
    Layout_qualifier_id &operator=(Layout_qualifier_id const &) GLSL_DELETED_FUNCTION;

private:
    /// The location of this layout qualifier id.
    Location const m_loc;

    /// If this is a Identifier, its name, else it is "SHARED"
    Name *m_ident;

    /// If non-NULL the identifier expression.
    Expr *m_expr;
};

typedef Ast_list<Layout_qualifier_id> Layout_qualifier;

/// Implementation of GLSL type qualifiers.
class Type_qualifier : public Interface_owned
{
    typedef Interface_owned Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef unsigned Storage_qualifiers;
    typedef unsigned Parameter_qualifiers;

    /// Get the storage qualifier set.
    Storage_qualifiers get_storage_qualifiers() const { return m_storage_qualifier; }

    /// Add a storage qualifier.
    void set_storage_qualifier(Storage_qualifier qual);

    /// Get the parameter qualifier set.
    Parameter_qualifiers get_parameter_qualifiers() const;

    /// Add a parameter qualifier.
    void set_parameter_qualifier(Parameter_qualifier qual);

    /// Get the layout qualifier.
    Layout_qualifier &get_layout_qualifier() { return m_layout_qualifier; }

    /// Get the layout qualifier.
    Layout_qualifier const &get_layout_qualifier() const { return m_layout_qualifier; }

    /// Get the precision qualifier.
    Precision_qualifier get_precision_qualifier() const { return m_precision_qualifier; }

    /// Set the precision qualifier.
    void set_precision_qualifier(Precision_qualifier qual);

    /// Get the interpolation qualifier.
    Interpolation_qualifier get_interpolation_qualifier() const {
        return m_interpolation_qualifier;
    }

    /// Set the interpolation qualifier.
    void set_interpolation_qualifier(Interpolation_qualifier qual);

    /// Check if this type name is marked with INVARIANT.
    bool is_invariant() const { return m_is_invariant; }

    /// Set the INVARIANT qualifier.
    void set_invariant();

    /// Check if this type name is marked with PRECISE.
    bool is_precise() const { return m_is_precise; }

    /// Set the PRECISE qualifier.
    void set_precise();

public:
    /// Default constructor, creates empty type qualifier.
    Type_qualifier();

private:
    // non copyable
    Type_qualifier(Type_qualifier const &) GLSL_DELETED_FUNCTION;
    Type_qualifier &operator=(Type_qualifier const &) GLSL_DELETED_FUNCTION;

private:
    /// The storage qualifier (for parameters partly remapped to parameter qualifier):
    Storage_qualifiers m_storage_qualifier;

    /// The layout qualifier.
    Layout_qualifier m_layout_qualifier;

    /// The precision qualifier.
    Precision_qualifier m_precision_qualifier;

    /// The interpolation qualifier.
    Interpolation_qualifier m_interpolation_qualifier;

    /// Set if the INVARIANT qualifier is set.
    bool m_is_invariant;

    /// Set if the PRECISE qualifier is set.
    bool m_is_precise;
};

/// Implementation of a GLSL array specifier.
class Array_specifier : public Ast_list_element<Array_specifier>
{
    typedef Ast_list_element<Array_specifier> Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Get the location of this array specifier.
    Location const &get_locarion() const { return m_loc; }

    /// Get the array size if any.
    Expr *get_size() { return m_size; }

    /// Get the array size if any.
    Expr const *get_size() const { return m_size; }

public:
    /// Constructor.
    ///
    /// \param loc   the location of this specifier
    /// \param size  if non-NULL, the array size expression
    explicit Array_specifier(
        Location const &loc,
        Expr           *size);

private:
    /// The location of this specifier.
    Location const m_loc;

    /// The array size if any.
    Expr *m_size;
};

typedef Ast_list<Array_specifier> Array_specifiers;

/// Implementation of a GLSL type name.
/// This holds two possible grammar elements:
/// - [ type_qualifier ] type_specifier (aka fully_specified_type)
/// - precision_qualifier type_specifier
class Type_name : public Interface_owned
{
    typedef Interface_owned Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Access the location.
    Location const &get_location() const { return m_loc; }

    /// Get the type qualifier.
    Type_qualifier &get_qualifier() { return m_type_qualifier; }

    /// Get the type qualifier.
    Type_qualifier const &get_qualifier() const { return m_type_qualifier; }

    /// Get the name of this type_name if any.
    Name *get_name() { return m_name; }

    /// Get the name of this type_name if any.
    Name const *get_name() const { return m_name; }

    /// Set the name of this type name if any.
    void set_name(Name *name);

    /// Get the struct definition for this type name if any.
    ///
    /// \return if non-NULL, this type name represents a named or unnamed struct declaration
    Declaration *get_struct_decl() { return m_struct_decl; }

    /// Get the struct definition for this type name if any.
    ///
    /// \return if non-NULL, this type name represents a named or unnamed struct declaration
    Declaration const *get_struct_decl() const { return m_struct_decl; }

    /// Set the struct declaration of this type name.
    ///
    /// \param decl  the declaration
    void set_struct_decl(Declaration *decl);

    /// Get the array specifiers.
    Array_specifiers &get_array_specifiers() { return m_array_specifiers; }

    /// Get the array specifiers.
    Array_specifiers const &get_array_specifiers() const { return m_array_specifiers; }

    /// Add an array specifier.
    void add_array_specifier(Array_specifier *arr_spec);

    /// Check if this is an array.
    bool is_array() const { return !m_array_specifiers.empty(); }

    /// Get the type represented by this type name.
    Type *get_type() const { return m_type; }

    /// Set the type represented by this name.
    void set_type(Type *type);

    /// Check if this is an incomplete array.
    bool is_incomplete_array() const;

private:
    /// Constructor.
    ///
    /// \param loc  the location of this type name
    explicit Type_name(
        Location const &loc);

private:
    // non copyable
    Type_name(Type_name const &) GLSL_DELETED_FUNCTION;
    Type_name &operator=(Type_name const &) GLSL_DELETED_FUNCTION;

private:
    /// The qualifiers of this type.
    Type_qualifier m_type_qualifier;

    /// The array specifiers if any.
    Array_specifiers m_array_specifiers;

    /// The type name.
    Name *m_name;

    /// The struct declaration of this name if any.
    Declaration *m_struct_decl;

    /// The described type.
    Type *m_type;

    /// The Location of this name.
    Location const m_loc;
};

/// Base class of all GLSL declarations.
class Declaration : public Ast_list_element<Declaration>
{
    typedef Ast_list_element<Declaration> Base;
public:
    enum Kind {
        DK_INVALID,   ///< An invalid declaration.
        DK_PRECISION, ///< A precision declaration.
        DK_VARIABLE,  ///< A variable declaration.
        DK_PARAM,     ///< A parameter declaration.
        DK_FUNCTION,  ///< A function declaration.
        DK_FIELD,     ///< A field declaration.
        DK_STRUCT,    ///< A struct declaration.
        DK_INTERFACE, ///< An interface declaration.
        DK_QUALIFIER, ///< A qualifier declaration.
    };

    /// Get the declaration kind.
    virtual Kind get_kind() const = 0;

    /// Get the location of this declaration.
    Location const &get_location() const { return m_loc; }

    /// Get the definition.
    Definition const *get_definition() const { return m_def; }

    /// Get the definition.
    Definition *get_definition() { return m_def; }

    /// Set the definition of this declaration.
    void set_definition(Definition *def) { m_def = def; }

protected:
    /// Constructor.
    ///
    /// \param loc the location of this declaration
    explicit Declaration(Location const &loc);

private:
    // non copyable
    Declaration(Declaration const &) GLSL_DELETED_FUNCTION;
    Declaration &operator=(Declaration const &) GLSL_DELETED_FUNCTION;

private:
    /// The location of this declaration.
    Location const m_loc;

    /// The definition of this declaration.
    Definition *m_def;
};

typedef Ast_list<Declaration> Declaration_list;

/// A GLSL invalid declaration.
class Declaration_invalid : public Declaration
{
     typedef Declaration Base;
     friend class mi::mdl::Arena_builder;
public:
    static Kind const s_kind = DK_INVALID;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

private:
    /// Constructor.
    ///
    /// \param loc the location of this declaration
    explicit Declaration_invalid(Location const &loc);
};

/// A GLSL precision declaration.
class Declaration_precision : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    static Kind const s_kind = DK_PRECISION;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type name with set precision here.
    Type_name *get_type_name() { return m_type_name; }

    /// Get the type name with set precision here.
    Type_name const *get_type_name() const { return m_type_name; }

    /// Set the precision of this declaration.
    void set_precision(Precision_qualifier qual);

private:
    /// Constructor.
    ///
    /// \param loc        the location of this declaration
    /// \param type_name  the type name
    Declaration_precision(
        Location const &loc,
        Type_name      *name);

private:
    /// The type name with set precision.
    Type_name *m_type_name;
};

/// A GLSL init declarator.
class Init_declarator : public Ast_list_element<Init_declarator>
{
    typedef Ast_list_element<Init_declarator> Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Get the identifier of the declarator.
    Name *get_name() { return m_name; }

    /// Get the identifier of the declarator.
    Name const *get_name() const { return m_name; }

    /// Set the identifier of this declarator.
    void set_name(Name *sym);

    /// Get the array specifiers.
    Array_specifiers &get_array_specifiers() { return m_array_specifiers; }

    /// Get the array specifiers.
    Array_specifiers const &get_array_specifiers() const { return m_array_specifiers; }

    /// Add an array specifier.
    void add_array_specifier(Array_specifier *arr_spec);

    /// Get the initializer of this declarator if any.
    Expr *get_initializer() { return m_init; }

    /// Get the initializer of this declarator if any.
    Expr const *get_initializer() const { return m_init; }

    /// Set the initializer of this declarator.
    void set_initializer(Expr *init);

    /// Get the location of the declarator (== location of the identifier).
    Location const &get_location() const { return m_loc; }

private:
    /// Constructor.
    ///
    /// \param loc  the location of this declarator
    explicit Init_declarator(
        Location const &loc);

private:
    /// The location of this declarator.
    Location const m_loc;

    /// The identifier of the declarator.
    Name *m_name;

    /// The array specifiers of this declarator if any.
    Array_specifiers m_array_specifiers;

    /// The init expression if any.
    Expr *m_init;
};

typedef Ast_list<Init_declarator> Init_declarator_list;

/// A GLSL variable declaration.
class Declaration_variable : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Init_declarator_list::iterator       iterator;
    typedef Init_declarator_list::const_iterator const_iterator;

    static Kind const s_kind = DK_VARIABLE;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type name of this variable declaration.
    Type_name *get_type_name() { return m_type_name; }

    /// Get the type name of this variable declaration.
    Type_name const *get_type_name() const { return m_type_name; }

    /// Returns true if the list of declared variables is empty.
    bool empty() const { return m_var_list.empty(); }

    /// Get The first declared variable.
    iterator begin() { return m_var_list.begin(); }

    /// Get the end iterator.
    iterator end() { return m_var_list.end(); }

    /// Get The first declared variable.
    const_iterator begin() const { return m_var_list.begin(); }

    /// Get the end iterator.
    const_iterator end() const { return m_var_list.end(); }

    /// Add a variable declaration.
    ///
    /// \param decl  the init declarator to add
    void add_init(Init_declarator *decl);

private:
    /// Constructor.
    ///
    /// \param name  the type name of this declaration
    explicit Declaration_variable(
        Type_name *name);

private:
    /// The type of this variable declaration.
    Type_name *m_type_name;

    /// The variable list.
    Init_declarator_list m_var_list;
};

/// A GLSL parameter declarator.
class Declaration_param : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    static Kind const s_kind = DK_PARAM;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type name of this parameter declaration.
    Type_name *get_type_name() { return m_type_name; }

    /// Get the type name of this parameter declaration.
    Type_name const *get_type_name() const { return m_type_name; }

    /// Get the name of the parameter if any.
    Name *get_name() { return m_name; }

    /// Get the name of the parameter if any.
    Name const *get_name() const { return m_name; }

    /// Set the name of the parameter.
    void set_name(Name *sym);

    /// Get the default argument of this parameter if any.
    Expr *get_default_argument() const { return m_default_argument; }

    /// Set the default argument of this parameter.
    void set_default_argument(Expr *expr);

    /// Get the array specifiers of this parameter.
    Array_specifiers &get_array_specifiers() { return m_array_specifiers; }

    /// Get the array specifiers of this parameter.
    Array_specifiers const &get_array_specifiers() const { return m_array_specifiers; }

    /// Add an array specifier.
    void add_array_specifier(Array_specifier *arr_spec);

private:
    /// Constructor.
    ///
    /// \param name  the type name of this parameter declaration
    explicit Declaration_param(
        Type_name *name);

private:
    /// The type name of this parameter declaration.
    Type_name *m_type_name;

    /// The name of this field.
    Name *m_name;

    /// The default initializer if any.
    Expr *m_default_argument;

    /// The array specifiers if any.
    Array_specifiers m_array_specifiers;
};

/// A GLSL function declaration.
class Declaration_function : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Declaration_list::iterator       iterator;
    typedef Declaration_list::const_iterator const_iterator;

    static Kind const s_kind = DK_FUNCTION;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the return type name of this function declaration.
    Type_name *get_ret_type() { return m_ret_type; }

    /// Get the return type name of this function declaration.
    Type_name const *get_ret_type() const { return m_ret_type; }

    /// Get the identifier of this function declaration.
    Name *get_identifier() { return m_name; }

    /// Get the identifier of this function declaration.
    Name const *get_identifier() const { return m_name; }

    /// Returns true if this function has no arguments.
    bool has_void_args() const { return m_params.empty(); }

    /// Returns the number of parameters.
    size_t get_param_count() const { return m_params.size(); }

    /// Get the first function parameter.
    iterator begin() { return m_params.begin(); }

    /// Get the end iterator of the function parameters.
    iterator end() { return m_params.end(); }

    /// Get the first function parameter.
    const_iterator begin() const { return m_params.begin(); }

    /// Get the end iterator of the function parameters.
    const_iterator end() const { return m_params.end(); }

    /// Add a function parameter.
    void add_param(Declaration *param);

    /// Get the function body if any.
    Stmt *get_body() { return m_body; }

    /// Get the function body if any.
    Stmt const *get_body() const { return m_body; }

    /// Set the function body.
    void set_body(Stmt *body);

    /// Returns true if this is a function proto type only.
    bool is_prototype() const { return m_body == NULL; }

private:
    /// Constructor.
    ///
    /// \param type_name  the return type of this function
    /// \param name       the identifier of this function
    explicit Declaration_function(
        Type_name *type_name,
        Name      *name);

private:
    /// The return type of this function declaration.
    Type_name *m_ret_type;

    /// The name of this function.
    Name *m_name;

    /// The function parameters.
    Declaration_list m_params;

    // The function body if this is a definition, else NULL.
    Stmt *m_body;
};

/// A GLSL Field declarator.
class Field_declarator : public Ast_list_element<Field_declarator>
{
    typedef Ast_list_element<Field_declarator> Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Get the location of this field.
    Location const &get_location() const { return m_loc; }

    /// Get the name of the field.
    Name *get_name() { return m_name; }

    /// Get the name of the field.
    Name const *get_name() const { return m_name; }

    /// Set the name of the field.
    void set_name(Name *name);

    /// Get the array specifiers of this field.
    Array_specifiers &get_array_specifiers() { return m_array_specifiers; }

    /// Get the array specifiers of this field.
    Array_specifiers const &get_array_specifiers() const { return m_array_specifiers; }

    /// Add an array specifier.
    void add_array_specifier(Array_specifier *arr_spec);

private:
    /// Constructor.
    ///
    /// \param loc  the location of this field declarator
    explicit Field_declarator(
        Location const &loc);

private:
    // non copyable
    Field_declarator(Field_declarator const &) GLSL_DELETED_FUNCTION;
    Field_declarator &operator=(Field_declarator const &) GLSL_DELETED_FUNCTION;

private:
    /// The location of this field declarator.
    Location const m_loc;

    /// The name of this field.
    Name *m_name;

    /// The array specifiers if any.
    Array_specifiers m_array_specifiers;
};

typedef Ast_list<Field_declarator> Field_declarator_list;

/// A GLSL field declaration.
class Declaration_field : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Field_declarator_list::iterator       iterator;
    typedef Field_declarator_list::const_iterator const_iterator;

    static Kind const s_kind = DK_FIELD;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type name of this field declaration.
    Type_name *get_type_name() { return m_type_name; }

    /// Get the type name of this field declaration.
    Type_name const *get_type_name() const { return m_type_name; }

    /// Get the first declared field.
    iterator begin() { return m_fields.begin(); }

    /// Get the end of the declared fields.
    iterator end() { return m_fields.end(); }

    /// Get the first declared field.
    const_iterator begin() const { return m_fields.begin(); }

    /// Get the end of the declared fields.
    const_iterator end() const { return m_fields.end(); }

    /// Add a field declarator.
    void add_field(Field_declarator *declarator);

private:
    /// Constructor.
    ///
    /// \param type_name  the type name
    explicit Declaration_field(
        Type_name *type_name);

private:
    /// The type name of this field declaration.
    Type_name *m_type_name;

    /// The declared fields.
    Field_declarator_list m_fields;
};

/// A GLSL struct declaration.
class Declaration_struct : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Declaration_list::iterator       iterator;
    typedef Declaration_list::const_iterator const_iterator;

    static Kind const s_kind = DK_STRUCT;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the struct name if any.
    Name *get_name() { return m_name; }

    /// Get the struct name if any.
    Name const *get_name() const { return m_name; }

    /// Set the struct name.
    void set_name(Name *name);

    /// Get the fields.
    Declaration_list &get_fields() { return m_fields; }

    /// Get the first field.
    iterator begin() { return m_fields.begin(); }

    /// Get the end field.
    iterator end() { return m_fields.end(); }

    /// Get the first field.
    const_iterator begin() const { return m_fields.begin(); }

    /// Get the end field.
    const_iterator end() const { return m_fields.end(); }

    /// Add a struct field.
    void add(Declaration *field);

private:
    /// Constructor.
    ///
    /// \param loc  the location of this struct declaration.
    explicit Declaration_struct(
        Location const &loc);

private:
    /// The struct name or NULL if unnamed.
    Name *m_name;

    /// The fields of this struct.
    Declaration_list m_fields;
};

/// A GLSL interface declaration.
class Declaration_interface : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Declaration_list::iterator       iterator;
    typedef Declaration_list::const_iterator const_iterator;

    static Kind const s_kind = DK_INTERFACE;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type qualifier of this interface declaration.
    Type_qualifier &get_qualifier() { return m_qual; }

    /// Get the type qualifier of this interface declaration.
    Type_qualifier const &get_qualifier() const { return m_qual; }

    /// Get the interface block name.
    Name *get_name() { return m_name; }

    /// Get the interface block name.
    Name const *get_name() const { return m_name; }

    /// Set the interface block name.
    void set_name(Name *name);

    /// Get the fields.
    Declaration_list &get_fields() { return m_fields; }

    /// Get the first field.
    iterator begin() { return m_fields.begin(); }

    /// Get the end instance.
    iterator end() { return m_fields.end(); }

    /// Get the first field.
    const_iterator begin() const { return m_fields.begin(); }

    /// Get the end instance.
    const_iterator end() const { return m_fields.end(); }

    /// Add a field declaration.
    void add_field(Declaration *field);

    /// Get the identifier if any.
    Name *get_identifier() { return m_ident; }

    /// Set the identifier.
    void set_identifier(Name *ident);

    /// Get the array specifiers.
    Array_specifiers &get_array_specifiers() { return m_array_specifiers; }

private:
    /// Constructor.
    ///
    /// \param loc   the location of this interface declaration
    /// \param name  the interface block name
    explicit Declaration_interface(
        Location const &loc,
        Name           *name);

private:
    /// The type qualifier of this interface;
    Type_qualifier m_qual;

    /// The interface block name.
    Name *m_name;

    /// The interface fields.
    Declaration_list m_fields;

    /// The identifier if any.
    Name *m_ident;

    /// The array specifiers if any.
    Array_specifiers m_array_specifiers;
};

/// A GLSL instance name.
class Instance_name : public Ast_list_element<Instance_name>
{
    typedef Ast_list_element<Instance_name> Base;
    friend class mi::mdl::Arena_builder;
public:
    /// Get the location of this instance name.
    Location const &get_location() const { return m_name->get_location(); }

    /// Get the name of the instance.
    Name *get_name() { return m_name; }

    /// Get the name of the instance.
    Name const *get_name() const { return m_name; }

public:
    /// Constructor.
    ///
    /// \param name  the name of this instance
    explicit Instance_name(
        Name *name);
private:
    /// The name of this instance.
    Name *m_name;
};

typedef Ast_list<Instance_name> Instance_name_list;

/// A GLSL qualified declaration.
class Declaration_qualified : public Declaration
{
    typedef Declaration Base;
    friend class mi::mdl::Arena_builder;
public:
    typedef Instance_name_list::iterator       iterator;
    typedef Instance_name_list::const_iterator const_iterator;

    static Kind const s_kind = DK_QUALIFIER;

    /// Get the declaration kind.
    Kind get_kind() const GLSL_FINAL;

    /// Get the type qualifier of this qualified declaration.
    Type_qualifier &get_qualifier() { return m_qual; }

    /// Get the type qualifier of this qualified declaration.
    Type_qualifier const &get_qualifier() const { return m_qual; }

    /// Get the first instance.
    iterator begin() { return m_instances.begin(); }

    /// Get the end instance.
    iterator end() { return m_instances.end(); }

    /// Get the first instance.
    const_iterator begin() const { return m_instances.begin(); }

    /// Get the end instance.
    const_iterator end() const { return m_instances.end(); }

    /// Add an instance.
    void add_instance(Instance_name *inst);

private:
    /// Constructor.
    ///
    /// \param loc  the location of this qualified declaration
    explicit Declaration_qualified(
        Location const &loc);

private:
    /// The type qualifier of this declaration;
    Type_qualifier m_qual;

    /// The qualified instances.
    Instance_name_list m_instances;
};

/// Cast to declaration or return NULL if types do not match.
template<typename T>
T const *as(Declaration const *decl) {
    return (decl->get_kind() == T::s_kind) ? static_cast<T const *>(decl) : NULL;
}

/// Cast to declaration or return NULL if types do not match.
template<typename T>
T *as(Declaration *decl) {
    return (decl->get_kind() == T::s_kind) ? static_cast<T *>(decl) : NULL;
}

/// Check if a declaration is of a certain type.
template<typename T>
bool is(Declaration const *decl) {
    return decl->get_kind() == T::s_kind;
}

/// Cast an expression.
template<typename T>
T *cast(Declaration *decl) {
    GLSL_ASSERT(decl == NULL || is<T>(decl));
    return static_cast<T *>(decl);
}

/// The declaration factory.
class Decl_factory : public Interface_owned
{
    typedef Interface_owned Base;
public:
    /// Creates a new name.
    ///
    /// \param loc  the location of this name
    /// \param sym  the symbol of this name.
    Name *create_name(
        Location const &loc,
        Symbol         *sym);

    /// Creates a new layout qualifier id.
    ///
    /// \param loc         the location of this layout qualifier id
    /// \param identifier  if non-NULL, the identifier of this id
    /// \param expr        if non-NULL, the expression of this id
    Layout_qualifier_id *create_layout_qualifier_id(
        Location const &loc,
        Name           *identifier,
        Expr           *expr);

    /// Creates a new array specifier.
    ///
    /// \param size  if non-NULL, the array size expression
    Array_specifier *create_array_specifier(
        Location const &loc,
        Expr           *size);

    /// Creates a type name..
    ///
    /// \param loc  the location of this type name
    Type_name *create_type_name(
        Location const &loc);

    /// Create a new invalid declaration.
    ///
    /// \param loc the location of this declaration
    Declaration_invalid *create_invalid(
        Location const &loc);

    /// Create a new precision declaration.
    ///
    /// \param loc        the location of this declaration
    /// \param type_name  the type name
    Declaration_precision *create_precision(
        Location const &loc,
        Type_name      *name);

    /// Creates a new init declarator.
    ///
    /// \param loc  the location of this declarator
    Init_declarator *create_init_declarator(
        Location const &loc);

    /// Create a new variable declaration.
    ///
    /// \param name  the type name of this declaration
    Declaration_variable *create_variable(
        Type_name *name);

    /// Create a parameter declaration.
    ///
    /// \param name  the type name of this parameter declaration
    Declaration_param *create_param(
        Type_name *name);

    /// Creates a new function declaration.
    ///
    /// \param type_name  the return type of this function
    /// \param name       the identifier of this function
    Declaration_function *create_function(
        Type_name *type_name,
        Name      *name);

    /// Creates a field declarator.
    ///
    /// \param loc  the location of this field declarator
    Field_declarator *create_field(
        Location const &loc);

    /// Creates a field declaration.
    ///
    /// \param type_name  the type name
    Declaration_field *create_field_declaration(
        Type_name *type_name);

    /// Creates a struct declaration.
    ///
    /// \param loc      the location of this struct declaration.
    Declaration_struct *create_struct(
        Location const &loc);

    /// Creates an interface declaration.
    ///
    /// \param loc   the location of this interface declaration
    /// \param name  the interface block name
    Declaration_interface *create_interface(
        Location const &loc,
        Name           *name);

    /// Creates an instance.
    ///
    /// \param name  the name of this instance
    Instance_name *create_instance(
        Name *name);

    /// Creates a qualified declaration.
    ///
    /// \param loc  the location of this qualifier declaration
    Declaration_qualified *create_qualified(
        Location const &loc);

public:
    /// Constructor.
    ///
    /// \param arena  the memory arena to build on
    explicit Decl_factory(Memory_arena &arena);

private:
    // non copyable
    Decl_factory(Decl_factory const &) GLSL_DELETED_FUNCTION;
    Decl_factory &operator=(Decl_factory const &) GLSL_DELETED_FUNCTION;

private:
    /// The builder for declarations.
    Arena_builder m_builder;
};

}  // glsl
}  // mdl
}  // mi

#endif
