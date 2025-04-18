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

 // examples/mdl_sdk/dxr/mdl_d3d12/mdl_sdk.h

#ifndef MDL_D3D12_MDL_SDK_H
#define MDL_D3D12_MDL_SDK_H

#include "common.h"
#include "example_shared.h"

namespace mi { namespace examples { namespace mdl_d3d12
{
    class Base_application;
    class Mdl_transaction;
    class Mdl_material_library;
    class Light_profile;
    class Bsdf_measurement;
    enum class Texture_dimension;

    // --------------------------------------------------------------------------------------------

    enum class Mdl_resource_kind
    {
        Texture, // includes 2D and 3D textures
        Light_profile,
        Bsdf_measurement,
        _Count
    };

    // --------------------------------------------------------------------------------------------

    /// Set of GPU texture resources that belong to one MDL texture resource
    struct Mdl_texture_set
    {
        struct Entry
        {
            explicit Entry()
                : resource(nullptr)
                , frame(0)
                , uvtile_u(0)
                , uvtile_v(0)
            {}

            Resource* resource; // texture or buffer
            int32_t frame;      // frame number
            int32_t uvtile_u;   // u-coordinate of the lower left corner of the tile
            int32_t uvtile_v;   // v-coordinate of the lower left corner of the tile
        };

        explicit Mdl_texture_set()
            : entries()
            , frame_first(0)
            , frame_last(0)
            , uvtile_u_min(0)
            , uvtile_u_max(0)
            , uvtile_v_min(0)
            , uvtile_v_max(0)
        {
        }

        std::vector<Entry> entries; // tile resources

        int32_t frame_first;        // first frame number
        int32_t frame_last;         // last frame number
        int32_t uvtile_u_min;       // u-coordinate of the tile most left bottom
        int32_t uvtile_u_max;       // v-coordinate of the tile most right top
        int32_t uvtile_v_min;       // u-coordinate of the tile most left bottom
        int32_t uvtile_v_max;       // v-coordinate of the tile most right top

        int32_t get_uvtile_width() const
        {
            return (uvtile_u_max + 1 - uvtile_u_min);
        }

        int32_t get_uvtile_height() const
        {
            return (uvtile_v_max + 1 - uvtile_v_min);
        }

        int32_t get_uvtile_count() const
        {
            return (frame_last + 1 - frame_first) *
                   (uvtile_u_max + 1 - uvtile_u_min) *
                   (uvtile_v_max + 1 - uvtile_v_min);
        }

        size_t compute_linear_uvtile_index(size_t entry_index)
        {
            int32_t f = entries[entry_index].frame - frame_first;
            int32_t v = entries[entry_index].uvtile_v - uvtile_v_min;
            int32_t u = entries[entry_index].uvtile_u - uvtile_u_min;
            return f * get_uvtile_width() * get_uvtile_height() + v * get_uvtile_width() + u;
        }
    };

    // --------------------------------------------------------------------------------------------

    struct Mdl_resource_assignment
    {
        explicit Mdl_resource_assignment(Mdl_resource_kind kind)
            : kind(kind)
            , resource_name("")
            , runtime_resource_id(0)
            , texture_data(nullptr)
        {
        }

        // type of resource
        Mdl_resource_kind kind;

        // DB name of the resource
        std::string resource_name;

        // Texture dimension in case of texture resources.
        Texture_dimension dimension;

        // ID generated by the SDK or the ITarget_resource_callback.
        // Is passed to the HLSL mdl renderer runtime.
        uint32_t runtime_resource_id;

        // resource data (textures, buffers, etc.)
        union
        {
            Mdl_texture_set* texture_data;
            Light_profile* light_profile_data;
            Bsdf_measurement* mbsdf_data;
        };

        bool has_data() const { return !!texture_data; }
    };

    // --------------------------------------------------------------------------------------------

    /// Information passed to GPU for mapping id requested in the runtime functions to texture
    /// views of the corresponding type.
    struct Mdl_texture_info
    {
        // index into the tex2d, tex3d, ... buffers, depending on the type requested
        uint32_t gpu_resource_array_start;

        // number resources (e.g. uv-tiles) that belong to this resource
        uint32_t gpu_resource_array_size;

        // frame number of the first texture/uv-tile
        int32_t gpu_resource_frame_first;

        // coordinate of the left bottom most uv-tile (also bottom left corner)
        int32_t gpu_resource_uvtile_u_min;
        int32_t gpu_resource_uvtile_v_min;

        // in case of uv-tiled textures,  required to calculate a linear index
        // (u + v * width + f * width * height)
        uint32_t gpu_resource_uvtile_width;
        uint32_t gpu_resource_uvtile_height;
    };

    // --------------------------------------------------------------------------------------------

    /// Information passed to the GPU for each BSDF measurement resource.
    struct Mdl_light_profile_info
    {
        // angular resolution of the grid and its inverse
        mi::Uint32_2_struct angular_resolution;
        mi::Float32_2_struct inv_angular_resolution;

        // starting angles of the grid
        mi::Float32_2_struct theta_phi_start;

        // angular step size and its inverse
        mi::Float32_2_struct theta_phi_delta;
        mi::Float32_2_struct theta_phi_inv_delta;

        // factor to rescale the normalized data
        // also represents the maximum candela value of the data
        float candela_multiplier;

        // power (radiant flux)
        float total_power;

        // index into the textures_2d array
        // -  texture contains normalized data sampled on grid
        uint32_t eval_data_index;

        // index into the buffers
        // - CDFs for sampling a light profile
        uint32_t sample_data_index;
    };

    /// Information passed to the GPU for each BSDF measurement resource.
    struct Mdl_mbsdf_info
    {
        // 32-bit booleans to specify if reflection and/or transmission data is present
        uint32_t has_data[2];

        // indices for the evaluation, sample and albedo data (for both parts)
        // - evaluation data texture consists of the bsdf measurement samples
        // - sample data buffer holds the CDFs for sampling
        // - albedo data buffer holds max albedos for each theta (isotropic)
        uint32_t eval_data_index[2];
        uint32_t sample_data_index[2];
        uint32_t albedo_data_index[2];

        // the maximum albedo values for both parts, used for limiting the multiplier
        float max_albedo[2];

        // vertical and horizontal angular resolutions for both parts, needed for texel access
        uint32_t angular_resolution_theta[2];
        uint32_t angular_resolution_phi[2];

        // number of channels per measurement sample in each part
        // 1 for intensity data, 3 for RGB color data
        uint32_t num_channels[2];
    };

    // --------------------------------------------------------------------------------------------

    struct Mdl_string_constant
    {
        // DB name of the resource
        std::string value;

        // ID generated by the SDK or the ITarget_resource_callback.
        // Is passed to the HLSL mdl renderer runtime.
        uint32_t runtime_string_id;
    };

    // --------------------------------------------------------------------------------------------

    class Mdl_sdk
    {
    public:
        /// A set of options that controls the MDL SDK and the code generation in global manner.
        /// These options can be bound to the GUI to be adjusted by the user at runtime.
        struct Options
        {
            bool use_class_compilation;
            bool fold_all_bool_parameters;
            bool fold_all_enum_parameters;

            bool enable_shader_cache; // note, this is not really an SDK option but fits here

            bool distilling_support_enabled;
            std::string distill_target;
        };

        // ----------------------------------------------------------------------------------------

        explicit Mdl_sdk(Base_application* app);
        virtual ~Mdl_sdk();

        // true if the MDL SDK was initialized correctly
        bool is_valid() const { return m_valid; }

        /// logs errors, warnings, infos, ... and returns true in case the was NO error
        /// meant to be called using the SRC macro: 'log_messages(context, SRC)'
        bool log_messages(
            const std::string& message,
            const mi::neuraylib::IMdl_execution_context* context,
            const std::string& file = "",
            int line = 0);

        mi::neuraylib::INeuray& get_neuray() { return *m_neuray; }
        mi::neuraylib::IMdl_configuration& get_config() { return *m_config; }
        mi::neuraylib::IDatabase& get_database() { return *m_database; }
        mi::neuraylib::IMdl_evaluator_api& get_evaluator() { return *m_evaluator_api; }
        mi::neuraylib::IMdl_factory& get_factory() { return *m_mdl_factory; }
        mi::neuraylib::IImage_api& get_image_api() { return *m_image_api; }
        mi::neuraylib::IMdl_impexp_api& get_impexp_api() { return *m_mdl_impexp_api; }
        mi::neuraylib::IMdl_backend& get_backend() { return *m_hlsl_backend; }
        mi::neuraylib::IMdl_distiller_api& get_distiller() { return *m_distiller_api; }

        /// Updates the mdl search paths.
        /// These include the default admin and user space paths, the example search path,
        /// and if available the application folder.
        /// While the first two are best practice, the other ones make it easier to copy
        /// the example binaries without losing the referenced example MDL modules.
        /// Additionally, the current scene path is added to allow per scene materials and this
        /// why the search paths are reconfigured after loading a new scene.
        void reconfigure_search_paths();

        // Creates a new execution context. At least one per thread is required.
        // This means you can share the context for multiple calls from the same thread.
        // However, sharing is not required. Creating a context for each call is valid too but
        // slightly more expensive.
        // Use a neuray handle to hold the pointer returned by this function.
        mi::neuraylib::IMdl_execution_context* create_context();

        /// access point to the database
        Mdl_transaction& get_transaction() { return *m_transaction; }

        /// keeps all materials that are loaded by the application
        Mdl_material_library* get_library() { return m_library; }

        /// A set of options that controls the MDL SDK and the code generation in global manner.
        /// These options can be bound to the GUI to be adjusted by the user at runtime.
        Options& get_options() { return m_mdl_options; }

    private:
        Base_application* m_app;

        mi::base::Handle<mi::neuraylib::INeuray> m_neuray;
        mi::base::Handle<mi::neuraylib::IMdl_configuration> m_config;
        mi::base::Handle<mi::neuraylib::IDatabase> m_database;
        mi::base::Handle<mi::neuraylib::IImage_api> m_image_api;
        mi::base::Handle<mi::neuraylib::IMdl_factory> m_mdl_factory;
        mi::base::Handle<mi::neuraylib::IMdl_backend> m_hlsl_backend;
        mi::base::Handle<mi::neuraylib::IMdl_impexp_api> m_mdl_impexp_api;
        mi::base::Handle<mi::neuraylib::IMdl_evaluator_api> m_evaluator_api;
        mi::base::Handle<mi::neuraylib::IMdl_distiller_api> m_distiller_api;

        Mdl_transaction* m_transaction;
        Mdl_material_library* m_library;
        Options m_mdl_options;
        bool m_valid;
    };

    // --------------------------------------------------------------------------------------------

    class Mdl_transaction
    {
        // make sure there is only one transaction
        friend class Mdl_sdk;

        explicit Mdl_transaction(Mdl_sdk* sdk);
    public:
        virtual ~Mdl_transaction();

        // runs an operation on the database.
        // concurrent calls are executed in sequence using a lock.
        template<typename TRes>
        TRes execute(std::function<TRes(mi::neuraylib::ITransaction* t)> action)
        {
            std::lock_guard<std::mutex> lock(m_transaction_mtx);
            return action(m_transaction.get());
        }

        template<>
        void execute<void>(std::function<void(mi::neuraylib::ITransaction* t)> action)
        {
            std::lock_guard<std::mutex> lock(m_transaction_mtx);
            action(m_transaction.get());
        }

        // locked database access function
        template<typename TIInterface>
        const TIInterface* access(const char* db_name)
        {
            return execute<const TIInterface*>(
                [&](mi::neuraylib::ITransaction* t)
            {
                return t->access<TIInterface>(db_name);
            });
        }

        // locked database access function
        template<typename TIInterface>
        TIInterface* edit(const char* db_name)
        {
            return execute<TIInterface*>(
                [&](mi::neuraylib::ITransaction* t)
            {
                return t->edit<TIInterface>(db_name);
            });
        }

        // locked database create function
        template<typename TIInterface>
        TIInterface* create(const char* type_name)
        {
            return execute<TIInterface*>(
                [&](mi::neuraylib::ITransaction* t)
            {
                return t->create<TIInterface>(type_name);
            });
        }

        // locked database store function
        template<typename TIInterface>
        mi::Sint32 store(
            TIInterface* db_element,
            const char* name)
        {
            return execute<mi::Sint32>(
                [&](mi::neuraylib::ITransaction* t)
            {
                return t->store(db_element, name);
            });
        }

        // locked database commit function.
        // For that, all handles to neuray objects have to be released.
        // Initializes for further actions afterwards.
        void commit();

        mi::neuraylib::ITransaction* get() { return m_transaction.get(); }

    private:
        mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
        std::mutex m_transaction_mtx;
        Mdl_sdk* m_sdk;
    };

}}} // mi::examples::mdl_d3d12
#endif
