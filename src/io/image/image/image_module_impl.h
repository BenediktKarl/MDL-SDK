/***************************************************************************************************
 * Copyright (c) 2011-2025, NVIDIA CORPORATION. All rights reserved.
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

#ifndef IO_IMAGE_IMAGE_IMAGE_MODULE_IMPL_H
#define IO_IMAGE_IMAGE_IMAGE_MODULE_IMPL_H

#include "i_image.h"

#include <mi/base/handle.h>
#include <mi/base/lock.h>

#include <memory>
#include <vector>

#include <base/system/main/access_module.h>

namespace mi { class IMap; namespace base { class IPlugin_descriptor; } }

namespace MI {

namespace IDATA { class Factory; }
namespace PLUG { class Plug_module; }

namespace IMAGE {

/// Implementation class of the IMAGE module
class Image_module_impl : public Image_module
{
public:

    ~Image_module_impl();

    // methods of SYSTEM::IModule

    bool init();

    void exit();

    // methods of Image_module

    IMipmap* create_mipmap(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height,
        mi::Uint32 layers,
        bool is_cubemap,
        mi::Float32 gamma) const;

    IMipmap* create_mipmap(
        File_based,
        const std::string& filename,
        const char* selector,
        bool only_first_level,
        mi::Sint32* errors) const;

    IMipmap* create_mipmap(
        Container_based,
        mi::neuraylib::IReader* reader,
        const std::string& container_filename,
        const std::string& member_filename,
        const char* selector,
        bool only_first_level,
        mi::Sint32* errors) const;

    IMipmap* create_mipmap(
        Memory_based,
        mi::neuraylib::IReader* reader,
        const char* image_format,
        const char* selector,
        const char* mdl_file_path,
        bool only_first_level,
        mi::Sint32* errors) const;

    void create_mipmap(
        std::vector<mi::base::Handle<mi::neuraylib::ICanvas> >& mipmaps,
        const mi::neuraylib::ICanvas* base_canvas,
        mi::Float32 gamma) const;

    IMipmap* create_mipmap(
        std::vector<mi::base::Handle<mi::neuraylib::ICanvas> >& canvases,
        bool is_cubemap) const;

    IMipmap* create_dummy_mipmap();

    bool is_dummy_mipmap( const IMipmap* mipmap) const;

    mi::neuraylib::ICanvas* create_canvas(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height,
        mi::Uint32 layers,
        bool is_cubemap,
        mi::Float32 gamma) const;

    mi::neuraylib::ICanvas* create_canvas(
        File_based,
        const std::string& filename,
        const char* selector,
        mi::Uint32 miplevel,
        mi::Sint32* errors) const;

    mi::neuraylib::ICanvas* create_canvas(
        Container_based,
        mi::neuraylib::IReader* reader,
        const std::string& container_filename,
        const std::string& member_filename,
        const char* selector,
        mi::Uint32 miplevel,
        mi::Sint32* errors) const;

    mi::neuraylib::ICanvas* create_canvas(
        Memory_based,
        mi::neuraylib::IReader* reader,
        const char* image_format,
        const char* selector,
        const char* mdl_file_path,
        mi::Uint32 miplevel,
        mi::Sint32* errors) const;

    mi::neuraylib::ICanvas* create_canvas(
        const std::vector<mi::base::Handle<mi::neuraylib::ITile>>& tiles,
        mi::Float32 gamma = 0.0f) const;

    mi::neuraylib::ITile* create_tile(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height) const;

    IMipmap* copy_mipmap( const IMipmap* other, bool only_first_level) const;

    mi::neuraylib::ICanvas* copy_canvas( const mi::neuraylib::ICanvas* other) const;

    mi::neuraylib::ITile* copy_tile( const mi::neuraylib::ITile* other) const;

    mi::Sint32 copy_mipmap_data(
        const IMipmap* source, IMipmap* dest) const;

    mi::Sint32 copy_canvas_data(
        const mi::neuraylib::ICanvas* source, mi::neuraylib::ICanvas* dest) const;

    mi::Sint32 copy_tile_data(
        const mi::neuraylib::ITile* source, mi::neuraylib::ITile* dest) const;

    IMipmap* convert_mipmap(
        const IMipmap* mipmap, Pixel_type pixel_type, bool only_first_level) const;

    mi::neuraylib::ICanvas* convert_canvas(
        const mi::neuraylib::ICanvas* canvas, Pixel_type pixel_type) const;

    mi::neuraylib::ITile* convert_tile(
        const mi::neuraylib::ITile* tile, Pixel_type pixel_type) const;

    void adjust_gamma(
        mi::neuraylib::ITile* tile, mi::Float32 old_gamma, mi::Float32 new_gamma) const;

    void adjust_gamma( mi::neuraylib::ICanvas* canvas, mi::Float32 new_gamma) const;

    Pixel_type get_pixel_type_for_channel(
        Pixel_type pixel_type, const char* selector) const;

    IMipmap* extract_channel(
        const IMipmap* mipmap, const char* selector, bool only_first_level) const;

    mi::neuraylib::ICanvas* extract_channel(
        const mi::neuraylib::ICanvas* canvas, const char* selector) const;

    mi::neuraylib::ITile* extract_channel(
        const mi::neuraylib::ITile* tile, const char* selector) const;

    void serialize_mipmap(
        SERIAL::Serializer* serializer, const IMipmap* mipmap, bool only_first_level) const;

    IMipmap* deserialize_mipmap( SERIAL::Deserializer* deserializer) const;

    void serialize_canvas(
        SERIAL::Serializer* serializer, const mi::neuraylib::ICanvas* canvas) const;

    mi::neuraylib::ICanvas* deserialize_canvas( SERIAL::Deserializer* deserializer) const;

    void serialize_tile(
        SERIAL::Serializer* serializer, const mi::neuraylib::ITile* tile) const;

    mi::neuraylib::ITile* deserialize_tile(
        SERIAL::Deserializer* deserializer) const;

    bool export_canvas(
        const mi::neuraylib::ICanvas* canvas,
        const char* output_filename,
        const mi::IMap* export_options) const;

    bool export_mipmap(
        const IMipmap* mipmap,
        const char* output_filename,
        const mi::IMap* export_options) const;

    mi::neuraylib::IBuffer* create_buffer_from_canvas(
        const mi::neuraylib::ICanvas* canvas,
        const char* image_format,
        const char* pixel_type,
        const mi::IMap* export_options) const;

    mi::neuraylib::IImage_plugin* find_plugin_for_import(
        const char* extension, mi::neuraylib::IReader* reader) const;

    mi::neuraylib::IImage_plugin* find_plugin_for_export( const char* extension) const;

    void set_mdl_container_callback( IMdl_container_callback* mdl_container_callback);

    IMdl_container_callback* get_mdl_container_callback() const;

    mi::neuraylib::ICanvas* create_miplevel(
        const mi::neuraylib::ICanvas* prev_canvas, float gamma_override) const;

    mi::IMap* convert_legacy_options( mi::Uint32 quality, bool force_default_gamma) const;

    void dump() const;

private:
    /// Adjusts the gamma value of a canvas depending on export options.
    ///
    /// If requested in \p export_options, and the gamma value of the canvas is sufficiently
    /// different from \p export_default_gamma, then #adjust_gamma() is called on a copy of
    /// \p canvas with \p export_default_gamma as new gamma value, and the modified copy is
    /// returned. Otherwise, no copy is created, but \p canvas itself is returned.
    const mi::neuraylib::ICanvas* enforce_default_gamma(
        const mi::neuraylib::ICanvas* canvas,
        const mi::IMap* export_options,
        mi::Float32 export_default_gamma) const;

    /// Helper function to detect valid image plugin type names.
    ///
    /// \param type        Type of the plugin, should be MI_NEURAY_IMAGE_PLUGIN_TYPE.
    /// \param name        Name of the plugin, for diagnostics.
    /// \param filename    Filename of the DSO, for diagnostics.
    /// \return            \c true if the plugin type is known and supported, \c false otherwise.
    ///                    Logs a warning for supported but outdated image plugin types, and an
    ///                    error for unsupported image plugin types.
    static bool is_valid_image_plugin( const char* type, const char* name, const char* filename);

    /// Returns the conversion cost for pixel type conversion.
    ///
    /// The costs have been manually set such that pixel types are preferred in the following order:
    /// - Same pixel type.
    /// - Equivalent pixel types ("Color"/"Float32<4>" and "Rgb_fp"/"Float32<3>").
    /// - Add alpha channel (if not already present).
    /// - Increase bits per channel (smaller increase preferred).
    /// - Reduce bits per channel (smaller decrease preferred).
    /// - Drop channels (smaller channel count reduction preferred).
    static mi::Float32 get_conversion_cost( Pixel_type from, Pixel_type to);

    /// Returns the pixel type that matches a given pixel type best and is supported by a plugin.
    ///
    /// \param pixel_type   The pixel type of the given canvas or mipmap.
    /// \param plugin       The plugin that will be used for export.
    /// \return             The pixel type that should be used for export. It is one of the pixel
    ///                     types supported by the plugin (or \c nullptr in case of failure).
    static const char* find_best_pixel_type_for_export(
        const char* pixel_type,
        mi::neuraylib::IImage_plugin* plugin);

    /// Indicates whether the given canvas is a cubemap or not.
    static bool get_canvas_is_cubemap( const mi::neuraylib::ICanvas* canvas);

    /// Access to the PLUG module
    SYSTEM::Access_module<PLUG::Plug_module> m_plug_module;

    /// Lock for #m_plugins.
    mutable mi::base::Lock m_plugins_lock;

    using Plugin_vector = std::vector<mi::base::Handle<mi::base::IPlugin_descriptor>>;

    /// The registered image plugins. Needs #m_plugins_lock.
    Plugin_vector m_plugins;

    /// Callback to support lazy loading of images in MDL containers.
    mi::base::Handle<IMdl_container_callback> m_mdl_container_callback;

    /// The IData factory.
    std::unique_ptr<IDATA::Factory> m_factory;

};

} // namespace IMAGE

} // namespace MI

#endif // IO_IMAGE_IMAGE_IMAGE_MODULE_IMPL_H
