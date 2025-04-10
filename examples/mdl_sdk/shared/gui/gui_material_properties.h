/******************************************************************************
 * Copyright (c) 2020-2025, NVIDIA CORPORATION. All rights reserved.
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

 // examples/mdl_sdk/shared/gui_material_properties.h

#ifndef EXAMPLE_SHARED_GUI_MATERIAL_PROPERTIES_H
#define EXAMPLE_SHARED_GUI_MATERIAL_PROPERTIES_H

#include <functional>
#include <stack>
#include <map>
#include <vector>
#include <mi/mdl_sdk.h>
#include <mi/neuraylib/target_code_types.h>
#include <mi/neuraylib/argument_editor.h>
#include <mi/neuraylib/iscene_element.h>

#include "gui.h"
#include "imgui_internal.h"

namespace mi { namespace examples { namespace gui
{

/// Interface between the Gui_section_material and the renderer to provide the Gui
/// with all available resources that are then offered in a drop down menu.
class Section_material_resource_handler
{
public:
    virtual ~Section_material_resource_handler() = default;

    /// Get the number of resources (Textures, Light Profiles, or Measured BSDFs) that
    /// are currently available for this material. This includes all resources that
    /// can be selected without loading new files or recompiling the material.
    /// It includes the invalid resource with ID 0.
    virtual mi::Size get_available_resource_count(mi::neuraylib::IValue::Kind kind);

    /// Get the ID of the i`th resource that is currently available.
    virtual mi::Uint32 get_available_resource_id(
        mi::neuraylib::IValue::Kind kind,
        mi::Size index);

    /// Get the display name of the i`th resource that is currently available.
    virtual const char* get_available_resource_name(
        mi::neuraylib::IValue::Kind kind,
        mi::Size index);

    /// Get the ID of a currently available resource by database name or -1.
    virtual mi::Uint32 get_available_resource_id(
        mi::neuraylib::IValue::Kind kind,
        const char* db_name);
};


/// Interface between the Gui_section_material and the renderer to provide the Gui
/// with all available string constants.
class Section_material_string_handler
{
public:
    virtual ~Section_material_string_handler() = default;

    /// Get the number of string constants that are currently available for this material.
    /// It includes the invalid resource with ID 0.
    virtual mi::Size get_string_count() const = 0;

    /// Get the ID of the i`th string that is currently available.
    virtual mi::Uint32 get_string_id(mi::Size index) const = 0;

    /// Get the string value of the i`th string that is currently available
    virtual const char* get_string_name(mi::Size index) const = 0;

    /// Get the ID of a given string in case it is currently available. 0, otherwise.
    virtual mi::Uint32 get_string_id_by_name(const char* string_name) const = 0;

    /// Get length of the longest string present in the target code.
    /// Used to limit text input buffers.
    virtual size_t get_max_string_name_length() const = 0;
};

/// Indicates the extend or impact of the changes made during the last updates.
enum class Section_material_update_state
{
    /// Parameters have not been changed.
    No_change = 0,

    /// Class-compilation parameters changed. The Argument block changed and the renderer
    /// needs to act on that. E.g., by uploading the buffer to the GPU.
    Argument_block_change = 1,

    /// A parameter was changed while in instance compilation mode. The material needs to be
    /// recompiled. The same applied to class compilation mode when the expression graph,
    /// attached to a parameter was changed structurally. In this case the material has to be
    /// recompiled as well.
    Structural_change = 2,

    /// When a parameter changed in class-compilation mode with enabled folding optimizations,
    /// e.g., booleans or enums, it could be possible that the change needs a recompilation.
    Unknown_change = 3,
};

class Section_material final : public Section
{
public: // declare internal data structures interfaces

    /// Internal context information that is passed to the parameter representation.
    class IParameter_context
    {
    public:
        virtual ~IParameter_context() = default;
    };

    /// Controls are organized hierarchical. This is the base class for each node in the hierarchy.
    class IParameter_node
    {
    public:
        virtual ~IParameter_node() = default;
    };

    /// Constructor.
    explicit Section_material(
        Root* gui,
        const std::string& name,
        bool collapsed_by_default,
        mi::neuraylib::IMdl_evaluator_api* evaluator,
        mi::neuraylib::IMdl_factory* factory);

    /// Destructor
    ~Section_material();

    /// Removes the material properties from the control.
    void unbind_material();

    /// Assign the material to edit. This should be done only when the selected material changed
    /// and not on a per frame basis.
    void bind_material(
        mi::neuraylib::ITransaction* transaction,
        const char* material_instance_db_name,
        const mi::neuraylib::ICompiled_material* compiled_material,
        const mi::neuraylib::ITarget_code* target_code,
        const mi::neuraylib::ITarget_value_layout* argument_block_layout,
        char* argument_block,
        Section_material_resource_handler* resource_handler,
        Section_material_string_handler* string_handler);

    /// Updates the Gui section and renders the material parameters or a level deeper in the
    /// hierarchy.
    void update(mi::neuraylib::ITransaction* transaction) final;

    /// Get the update state of the last update calls (since reset_update_state was called)
    Section_material_update_state get_update_state() const;

    /// After handling the consequences of the update by the application, reset has to be called
    /// in order to get informed by further updates. This is useful to delay structural changes
    /// to reasonable interval. E.g., in instance compilation to the point where the user released
    /// a slider.
    void reset_update_state();

private:
    /// create a node for an expression to build up the parameter hierarchy.
    IParameter_node* create(
        mi::neuraylib::ITransaction* transaction,
        mi::neuraylib::IMdl_factory* factory,
        const char* name,
        IParameter_node* parenting_call,
        const mi::neuraylib::IExpression* expression,
        const mi::neuraylib::IExpression* default_value,
        const mi::neuraylib::IAnnotation_block* annos,
        IParameter_context* context);

    /// create a node for an (constant) value to build up the parameter hierarchy.
    IParameter_node* create(
        mi::neuraylib::ITransaction* transaction,
        mi::neuraylib::IMdl_factory* factory,
        const char* name,
        IParameter_node* parenting_call,
        const mi::neuraylib::IValue* value,
        const mi::neuraylib::IValue* default_value,
        const mi::neuraylib::IAnnotation_block* annos,
        IParameter_context* context);

    // iterate over all parameters and render their controls
    Section_material_update_state update_material_parameters(
        mi::neuraylib::ITransaction* transaction,
        IParameter_node* call);

    std::stack<IParameter_node*> m_call_history;
    std::unique_ptr<IParameter_node> m_material_parameters;
    std::unique_ptr<IParameter_context> m_material_parameters_context;
    mi::base::Handle<mi::neuraylib::IMdl_evaluator_api> m_evaluator;
    mi::base::Handle<mi::neuraylib::IMdl_factory> m_factory;
    Section_material_update_state m_update_state;
};

}}}
#endif
