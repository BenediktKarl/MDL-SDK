/******************************************************************************
 * Copyright (c) 2019-2025, NVIDIA CORPORATION. All rights reserved.
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

// examples/mdl_sdk/dxr/mdl_d3d12/mdl_material.h

#ifndef MDL_D3D12_MDL_MATERIAL_H
#define MDL_D3D12_MDL_MATERIAL_H

#include "common.h"
#include "base_application.h"
#include "mdl_material_description.h"
#include "mdl_material_library.h"
#include "mdl_material_target.h"
#include "buffer.h"
#include "scene.h"
#include "shader.h"


namespace mi { namespace examples { namespace mdl_d3d12
{
    class Base_application;
    class Descriptor_heap;
    class Material;
    class Mdl_transaction;
    class Mdl_material_info;
    class Mdl_material_target;
    class Mdl_sdk;
    enum class Mdl_resource_kind;
    struct Mdl_resource_assignment;
    struct Mdl_texture_info;
    struct Mdl_light_profile_info;
    struct Mdl_mbsdf_info;
    enum class Texture_dimension;

    // --------------------------------------------------------------------------------------------

    class Mdl_material : public IMaterial
    {
        // --------------------------------------------------------------------

    public:
        struct Constants
        {
            // shared for all material compiled from the same MDL material
            // - none -

            // individual properties of the different material instances

            // id of the material in the scene
            int32_t material_id;

            // features of the material in terms of code paths to execute
            // (optimization of the entire material class)
            uint32_t features;

            // features of the individual material that are independent of MDL code,
            // e.g. single sided primitives (defined on the material to align with gltf)
            uint32_t flags;
        };

        // --------------------------------------------------------------------

        explicit Mdl_material(Base_application* app);
        virtual ~Mdl_material();

        uint32_t get_id() const { return m_material_id; }
        void set_name(const std::string& value) override { m_name = value; }
        const std::string& get_name() const override { return m_name; }
        // Flags get_flags() const override { return m_description.get_flags(); }

        /// use a material definition and a set of parameters to create an MDL compiled material
        /// that can be added to an Mdl_material_target.
        bool compile_material(const Mdl_material_description& description);

        /// create an MDL compiled material from the existing instance by reusing the existing
        /// definition and parameter list. This can be used after reloading modules.
        bool recompile_material(mi::neuraylib::IMdl_execution_context* context);

        const Mdl_material_description& get_material_desciption() const { return m_description; }
        const std::string& get_material_instance_db_name() const { return m_instance_db_name; }
        const std::string& get_material_compiled_db_name() const { return m_compiled_db_name; }
        const std::string& get_material_compiled_hash() const { return m_compiled_hash; }

        /// set the per target code constants in the materials constant buffer.
        void set_target_interface(
            Mdl_material_target* target,
            const Mdl_material_target_interface& target_data);

        /// reset the target code to none.
        void reset_target_interface();

        /// called by the Mdl_material_target after the target code was generated in order to
        /// allocate and update per material buffers for constants and textures.
        bool on_target_generated(D3DCommandList* command_list);

        /// get a descriptor table that describes the resource layout in the materials heap region.
        /// Note, non-static table has to be used when a target is created for each material.
        // const Descriptor_table get_descriptor_table();

        /// samplers used for mdl texture lookups
        static std::vector<D3D12_STATIC_SAMPLER_DESC> get_sampler_descriptions();

        /// get the identifier of the target code for this material.
        virtual const std::string& get_hash() const {
            return get_target_code()->get_compiled_material_hash(); }

        /// register scene data names that appear on the geometry data at the material for
        /// making them accessible in the shader.
        uint32_t register_scene_data_name(const std::string& name) override;

        /// get the target code that contains this material.
        Mdl_material_target* get_target_code() const { return m_target; }

        /// get the argument layout of the class compilation argument data block
        /// or NULL if there is no layout in case of instance compilation.
        const mi::neuraylib::ITarget_value_layout* get_argument_layout() const;

        /// get a pointer in argument block buffer or NULL if there is none.
        uint8_t* get_argument_data();

        /// Get the start index of the resources belonging to the target code of this material.
        /// The index can be used directly in the shader code.
        uint32_t get_target_resource_heap_index() const override
        {
            return m_target->get_resource_heap_index();
        }

        /// Get the start index of the resources belonging to this individual material.
        /// The index can be used directly in the shader code.
        uint32_t get_material_resource_heap_index() const override
        {
            return static_cast<uint32_t>(m_first_resource_heap_handle.get_heap_index());
        }

        /// after material parameters have been changed, they have to copied to the GPU.
        void update_material_parameters();

        /// Called by the target to register per material resources.
        size_t register_resource(
            Mdl_resource_kind kind,
            Texture_dimension dimension,
            const std::string& resource_name);

        /// get the resources used by this material
        const std::vector<Mdl_resource_assignment>& get_resources(Mdl_resource_kind kind) const;

        /// Called by the target to register per material string constants.
        uint32_t map_string_constant(const std::string& string_value);

        /// get the string constants used by this material
        const std::vector<Mdl_string_constant>& get_string_constants() const;

        /// Get the functions indices and argument layout index for the generated target code.
        const Mdl_material_target_interface& get_target_interface() const {
            return m_material_target_interface;
        }

    private:
        Base_application* m_app;
        Mdl_sdk* m_sdk;
        const uint32_t m_material_id;

        // Description including definition and parameter set of this material.
        Mdl_material_description m_description;

        // display name of the material in the scene.
        std::string m_name;

        /// database name of the material instance of this material.
        std::string m_instance_db_name;

        /// database name of the compiled material of this material.
        std::string m_compiled_db_name;

        /// the compiled material hash to detect materials that only differ in their parameters
        /// values.
        std::string m_compiled_hash;

        /// the compiled material hash stored when loading resources for this material.
        /// even when the target changed the compiled hash of a single material can stay constant.
        /// If that`s the case, also the resources are still valid. Loading them again is not
        /// Required.
        std::string m_resource_hash;

        /// contains the actual shader code for material.
        Mdl_material_target* m_target;

        /// constant buffer for function indices, material id and flags.
        Constant_buffer<Constants> m_constants;

        /// buffer that contains the material parameters, changing them during runtime is possible.
        Buffer* m_argument_block_buffer;
        std::vector<uint8_t> m_argument_block_data;

        /// index target code that describes the layout of the argument block data.
        mi::Size m_argument_layout_index;

        /// Contains functions indices and argument layout index for the generated target code.
        Mdl_material_target_interface m_material_target_interface;

        /// all material resources have views on the descriptor heap (all in one continuous block)
        Descriptor_heap_handle m_first_resource_heap_handle;    // first descriptor on the heap

        /// Mapping from runtime texture IDs to actual resource views in the shader
        Structured_buffer<Mdl_texture_info>* m_texture_infos;

        /// Information needed during runtime for light profile resources
        Structured_buffer<Mdl_light_profile_info>* m_light_profile_infos;

        /// Information needed during runtime for bsdf measurement resources
        Structured_buffer<Mdl_mbsdf_info>* m_mbsdf_infos;

        /// references to the resources used by the material
        /// resources are owned by the material library to avoid duplications
        std::map<Mdl_resource_kind, std::vector<Mdl_resource_assignment>> m_resources;

        /// list of string constants used by the material
        std::vector<Mdl_string_constant> m_string_constants;
    };

}}} // mi::examples::mdl_d3d12
#endif
