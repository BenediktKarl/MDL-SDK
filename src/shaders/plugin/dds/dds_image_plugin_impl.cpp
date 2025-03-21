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

#include "pch.h"

#include "dds_image_plugin_impl.h"
#include "dds_image_file_reader_impl.h"
#include "dds_image_file_writer_impl.h"
#include "dds_utilities.h"

#include <mi/neuraylib/iplugin_api.h>
#include <mi/neuraylib/ireader.h>
#include <mi/neuraylib/iwriter.h>
#include <mi/neuraylib/ilogging_configuration.h>
#include <base/system/version/i_version.h>

#include <string>

namespace MI {

namespace DDS {

bool Image_plugin_impl::init( mi::neuraylib::IPlugin_api* plugin_api)
{
    assert( plugin_api);

    mi::base::Handle<mi::neuraylib::ILogging_configuration> logging_configuration(
        plugin_api->get_api_component<mi::neuraylib::ILogging_configuration>());
    if( logging_configuration)
        g_logger = logging_configuration->get_forwarding_logger();

    std::string message = "Plugin \"";
    message += get_name();
    message += "\" (build " + std::string( VERSION::get_platform_version());
    message += ", " + std::string( VERSION::get_platform_date());
    message += ") initialized";
    log( mi::base::MESSAGE_SEVERITY_INFO, message.c_str());

    m_image_api = plugin_api->get_api_component<mi::neuraylib::IImage_api>();

    return true;
}

bool Image_plugin_impl::exit( mi::neuraylib::IPlugin_api* plugin_api)
{
    m_image_api.reset();
    g_logger.reset();
    return true;
}

const char* Image_plugin_impl::get_file_extension( mi::Uint32 index) const
{
    if( index == 0)
        return "dds";
    return nullptr;
}

const char* Image_plugin_impl::get_supported_type( mi::Uint32 index) const
{
    if( index == 0) return "Rgba";
    if( index == 1) return "Rgb";
    return nullptr;
}

bool Image_plugin_impl::test( mi::neuraylib::IReader* reader) const
{
    const mi::Sint64 N = 4;
    char buffer[N];
    mi::Sint64 bytes_read = reader->read( buffer, sizeof( buffer));
    if( bytes_read != N)
        return false;

    return buffer[0] == 'D'
        && buffer[1] == 'D'
        && buffer[2] == 'S'
        && buffer[3] == ' ';
}

mi::neuraylib::Impexp_priority Image_plugin_impl::get_priority() const
{
    return mi::neuraylib::IMPEXP_PRIORITY_WELL_DEFINED;
}

mi::neuraylib::IImage_file* Image_plugin_impl::open_for_writing(
    mi::neuraylib::IWriter* writer,
    const char* pixel_type,
    mi::Uint32 resolution_x,
    mi::Uint32 resolution_y,
    mi::Uint32 nr_of_layers,
    mi::Uint32 miplevels,
    bool is_cubemap,
    mi::Float32 gamma,
    const mi::IMap* export_options) const
{
    if( !writer || !pixel_type)
        return nullptr;

    if( !writer->supports_absolute_access())
        return nullptr;

    // Invalid canvas properties
    if( resolution_x == 0 || resolution_y == 0 || nr_of_layers == 0 || miplevels == 0
        || gamma <= 0.0f)
        return nullptr;

    if( is_cubemap && nr_of_layers != 6)
        return nullptr;

    // DDS cannot handle the combination of these features
    if( nr_of_layers > 1 && miplevels > 1)
        return nullptr;

    return new Image_file_writer_impl(
        writer, pixel_type, resolution_x, resolution_y, nr_of_layers, miplevels, is_cubemap, gamma);
}

mi::neuraylib::IImage_file* Image_plugin_impl::open_for_reading(
    mi::neuraylib::IReader* reader, const char* selector) const
{
    if( !reader)
        return nullptr;

    if( !reader->supports_absolute_access())
        return nullptr;

    return new Image_file_reader_impl( m_image_api.get(), reader);
}

/// Factory to create an instance of Image_plugin_impl.
extern "C"
MI_DLL_EXPORT
mi::base::Plugin* mi_plugin_factory(
    mi::Sint32 index,         // index of the plugin
    void* context)            // context given to the library, ignore
{
    if( index >= 1)
        return nullptr;
    return new Image_plugin_impl();
}

} // namespace DDS

} // namespace MI
