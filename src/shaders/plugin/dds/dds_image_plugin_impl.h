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

#ifndef SHADERS_PLUGIN_DDS_DDS_IMAGE_PLUGIN_IMPL_H
#define SHADERS_PLUGIN_DDS_DDS_IMAGE_PLUGIN_IMPL_H

#include <mi/base/handle.h>
#include <mi/neuraylib/iimage_api.h>
#include <mi/neuraylib/iimage_plugin.h>

namespace MI {

namespace DDS {

class Image_plugin_impl : public mi::neuraylib::IImage_plugin
{
public:

    virtual ~Image_plugin_impl() { }

    // methods of mi::base::Plugin

    const char* get_name() const { return "dds"; }

    const char* get_type() const { return MI_NEURAY_IMAGE_PLUGIN_TYPE; }

    mi::Sint32 get_version() const { return 1; }

    const char* get_compiler() const { return "unknown"; }

    void release() { delete this; }

    // methods of mi::neuraylib::IImage_plugin

    bool init( mi::neuraylib::IPlugin_api* plugin_api);

    bool exit( mi::neuraylib::IPlugin_api* plugin_api);

    const char* get_file_extension( mi::Uint32 index) const;

    const char* get_supported_type( mi::Uint32 index) const;

    mi::neuraylib::Impexp_priority get_priority() const;

    bool supports_selectors() const { return false; }

    bool test( mi::neuraylib::IReader* reader) const;

    mi::neuraylib::IImage_file* open_for_writing(
        mi::neuraylib::IWriter* writer,
        const char* pixel_type,
        mi::Uint32 resolution_x,
        mi::Uint32 resolution_y,
        mi::Uint32 nr_of_layers,
        mi::Uint32 miplevels,
        bool is_cubemap,
        mi::Float32 gamma,
        const mi::IMap* export_options) const;

    mi::neuraylib::IImage_file* open_for_reading(
        mi::neuraylib::IReader* reader, const char* selector) const;

private:

    /// API component IImage_api.
    mi::base::Handle<mi::neuraylib::IImage_api> m_image_api;
};

} // namespace DDS

} // namespace MI

#endif // SHADERS_PLUGIN_DDS_DDS_IMAGE_PLUGIN_IMPL_H
