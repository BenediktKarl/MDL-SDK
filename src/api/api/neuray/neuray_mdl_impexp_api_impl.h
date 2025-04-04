/***************************************************************************************************
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
 **************************************************************************************************/

/** \file
 ** \brief Header for the IMdl_impexp_api implementation.
 **/

#ifndef API_API_NEURAY_NEURAY_MDL_IMPEXP_API_IMPL_H
#define API_API_NEURAY_NEURAY_MDL_IMPEXP_API_IMPL_H

#include <mi/base/interface_implement.h>
#include <mi/neuraylib/imdl_impexp_api.h>

#include <boost/core/noncopyable.hpp>

namespace mi {

namespace mdl { class IModule; }

namespace neuraylib {
class IBsdf_isotropic_data;
class ICanvas;
class ILightprofile;
class INeuray;
class IMdl_execution_context;
class ITransaction;
class IWriter;
}  // neuraylib
}  // mi

namespace MI {

namespace MDL { class Execution_context; }
namespace MDLC { class Mdlc_module; }

namespace NEURAY {



class Mdl_impexp_api_impl final
  : public mi::base::Interface_implement<mi::neuraylib::IMdl_impexp_api>,
    public boost::noncopyable
{
public:
    /// Constructor of Mdl_impexp_api_impl.
    ///
    /// \param neuray      The neuray instance which contains this Mdl_impexp_api_impl
    Mdl_impexp_api_impl( mi::neuraylib::INeuray* neuray);

    /// Destructor of Mdl_impexp_api_impl.
    virtual ~Mdl_impexp_api_impl();

    // public API methods

    mi::Sint32 load_module(
        mi::neuraylib::ITransaction* transaction,
        const char* argument,
        mi::neuraylib::IMdl_execution_context* context) final;

    mi::Sint32 load_module_from_string(
        mi::neuraylib::ITransaction* transaction,
        const char* module_name,
        const char* module_source,
        mi::neuraylib::IMdl_execution_context* context) final;

    mi::Sint32 export_module(
        mi::neuraylib::ITransaction* transaction,
        const char* module_name,
        const char* filename,
        mi::neuraylib::IMdl_execution_context* context) final;

    mi::Sint32 export_module_to_string(
        mi::neuraylib::ITransaction* transaaction,
        const char* module_name,
        mi::IString* exported_module,
        mi::neuraylib::IMdl_execution_context* context) final;

    mi::Sint32 export_canvas(
        const char* filename,
        const mi::neuraylib::ICanvas* canvas,
        const mi::IMap* export_options) const final;

    mi::Sint32 deprecated_export_canvas(
        const char* filename,
        const mi::neuraylib::ICanvas* canvas,
        mi::Uint32 quality,
        bool force_default_gamma) const final;

    mi::Sint32 export_lightprofile(
        const char* filename,
        const mi::neuraylib::ILightprofile* lightprofile) const final;

    mi::Sint32 export_bsdf_data(
        const char* filename,
        const mi::neuraylib::IBsdf_isotropic_data* reflection,
        const mi::neuraylib::IBsdf_isotropic_data* transmission) const final;

    const mi::IString* frame_uvtile_marker_to_string(
        const char* marker, mi::Size f, mi::Sint32 u, mi::Sint32 v) const final;

    const mi::IString* get_mdl_module_name(
        const char* filename, mi::neuraylib::IMdl_impexp_api::Search_option option) const final;

    const mi::neuraylib::ISerialized_function_name* serialize_function_name(
        const char* definition_name,
        const mi::neuraylib::IType_list* argument_types,
        const mi::neuraylib::IType* return_type,
        mi::neuraylib::IMdle_serialization_callback* mdle_callback,
        mi::neuraylib::IMdl_execution_context* context) const final;

    const mi::neuraylib::IDeserialized_function_name* deserialize_function_name(
        mi::neuraylib::ITransaction* transaction,
        const char* function_name,
        mi::neuraylib::IMdle_deserialization_callback* mdle_callback,
        mi::neuraylib::IMdl_execution_context* context) const final;

    const mi::neuraylib::IDeserialized_function_name* deserialize_function_name(
        mi::neuraylib::ITransaction* transaction,
        const char* module_name,
        const char* function_name_without_module_name,
        mi::neuraylib::IMdle_deserialization_callback* mdle_callback,
        mi::neuraylib::IMdl_execution_context* context) const final;

    const mi::neuraylib::IDeserialized_module_name* deserialize_module_name(
        const char* module_name,
        mi::neuraylib::IMdle_deserialization_callback* mdle_callback,
        mi::neuraylib::IMdl_execution_context* context) const final;

    mi::neuraylib::IReader* create_reader( const mi::neuraylib::IBuffer* buffer) const final;

    mi::neuraylib::IReader* create_reader( const char* filename) const final;

    mi::neuraylib::IWriter* create_writer( const char* filename) const final;

    // internal methods

    /// Starts this API component.
    ///
    /// The implementation of INeuray::start() calls the #start() method of each API component.
    /// This method performs the API component's specific part of the library start.
    ///
    /// \return            0, in case of success, -1 in case of failure.
    mi::Sint32 start();

    /// Shuts down this API component.
    ///
    /// The implementation of INeuray::shutdown() calls the #shutdown() method of each API
    /// component. This method performs the API component's specific part of the library shutdown.
    ///
    /// \return           0, in case of success, -1 in case of failure
    mi::Sint32 shutdown();

    /// Used by #export_module() and #export_module_to_string() to handle the common part.
    ///
    /// Also used by Mdl_module_transformer_impl to handle the export.
    ///
    /// \param filename   The filename used by the writer, or \c nullptr for string-based exports.
    static mi::Sint32 export_module_common(
        mi::neuraylib::ITransaction* transaction,
        const char* module_name,
        const mi::mdl::IModule* module,
        mi::neuraylib::IWriter* writer,
        const char* filename,
        MDL::Execution_context* context);

    mi::neuraylib::INeuray* m_neuray;
};

} // namespace NEURAY

} // namespace MI

#endif // API_API_NEURAY_NEURAY_MDL_IMPEXP_API_IMPL_H
