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

/// Important abstract interfaces in the IMAGE module are
///
/// - mi::neuraylib::ITile              #include <mi/neuraylib/itile.h>
/// - mi::neuraylib::ICanvas            #include <mi/neuraylib/icanvas.h>
/// - MI::IMAGE::IMipmap                #include <io/image/image/i_image_mipmap.h>
///
/// The IMAGE module has implementations of all these interfaces, though you should never work
/// directly with the implementations, or make use of implementation details.
///
/// There are two convenience classes for canvases and mipmaps that allow to get/set single pixels
/// (not very efficient) and to read/write a rectangular region of a canvas or miplevel layer:
///
/// - MI::IMAGE::Access/Edit_canvas     #include <io/image/image/i_image_access_canvas.h>
/// - MI::IMAGE::Access/Edit_mipmap     #include <io/image/image/i_image_access_mipmap.h>
///
/// For low-level pixel conversion (and copying) there are the following two functions that operate
/// on raw memory pointers.
///
/// - MI::IMAGE::convert()              #include <io/image/image/i_image_pixel_conversion.h>
/// - MI::IMAGE::copy()                 #include <io/image/image/i_image_pixel_conversion.h>
///
/// For high-level pixel conversion (and copying) of tiles, canvases, and mipmaps you should use the
/// methods on MI::IMAGE::Image_module instead. The read/write methods for rectangular regions on
/// MI::IMAGE::Access/Edit_canvas and  MI::IMAGE::Access/Edit_mipmap also support pixel conversion.
///
/// The enum for all supported pixel types and various utility functions related to pixel types can
/// be found in io/image/image/i_image_utilities.h.
///
/// Furthermore, there are the following classes related to the IMAGE module, but not belonging to
/// the IMAGE module itself:
///
/// - MI::DBIMAGE::Image                #include <io/scene/dbimage/i_dbimage.h>
/// - MI::TEXTURE::Texture              #include <io/scene/texture/i_texture.h>
///
/// MI::DBIMAGE::Image wraps MI::IMAGE::IMipmap as DB element, and
/// MI::TEXTURE::Texture references MI::DBIMAGE::Image via tag.
///
/// Note about mipmaps: several methods related to mipmaps have a only_first_level flag that
/// controls whether higher miplevels are recomputed if needed or loaded/serialized/copied/converted
/// as if they were independent from the level below. Since none of our render modes currently cares
/// about higher miplevels the flags are enabled by default to save handling of the higher
/// miplevels. Note that they are still computed on demand if requested.

#ifndef IO_IMAGE_IMAGE_I_IMAGE_H
#define IO_IMAGE_IMAGE_I_IMAGE_H

#include "i_image_utilities.h"

#include <base/system/main/i_module.h>

#include <mi/base/handle.h>
#include <mi/base/interface_declare.h>

#include <string>
#include <vector>


namespace mi {

class IMap;

namespace neuraylib {
class IBuffer;
class ICanvas;
class IImage_plugin;
class IReader;
class ITile;
class IWriter;
}

}

namespace MI {

namespace SYSTEM { class Module_registration_entry; }
namespace SERIAL { class Serializer; class Deserializer; }

namespace IMAGE {

class IMdl_container_callback;
class IMipmap;

/// Public interface of the IMAGE module.
class Image_module : public SYSTEM::IModule
{
public:
    /// Returns the module registrations entry for the module.
    static SYSTEM::Module_registration_entry* get_instance();

    /// Returns the name of the module.
    static const char* get_name() { return "IMAGE"; }

    // Factory methods to create mipmaps, canvases, and tiles (including file-based ones)
    // ==================================================================================

    /// Creates a memory-based mipmap with given pixel type, width, height, and layers of the
    /// base level.
    ///
    /// \param pixel_type         The desired pixel type.
    /// \param width              The desired width.
    /// \param height             The desired height.
    /// \param layers             The desired number of layers (depth).
    /// \param is_cubemap         Flag that indicates whether this mipmap represents a cubemap.
    /// \param gamma              The desired gamma value. The special value 0.0 represents the
    ///                           default gamma which is 1.0 for HDR pixel types and 2.2 for LDR
    ///                           pixel types.
    /// \return                   The requested mipmap, or \c nullptr in case of invalid pixel type,
    ///                           width, height, layers, or cubemap flag, or memory allocation
    ///                           failures.
    virtual IMipmap* create_mipmap(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height,
        mi::Uint32 layers = 1,
        bool is_cubemap = false,
        mi::Float32 gamma = 0.0f) const = 0;

    /// Creates a file-based mipmap that represents the given file on disk.
    ///
    /// \param filename           The file that shall be represented by this mipmap.
    /// \param selector           The selector (or \c nullptr).
    /// \param only_first_level   Indicates whether only the first (or all) miplevels should be
    ///                           read from the file.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -3: No image plugin found to handle the file.
    ///                           -  -5: Failure to open the file.
    ///                           -  -7: The image plugin failed to import the file.
    /// \return                   The requested mipmap, or a dummy mipmap with a 1x1 pink pixel in
    ///                           case of errors.
    virtual IMipmap* create_mipmap(
        File_based,
        const std::string& filename,
        const char* selector,
        bool only_first_level = true,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates an container-based mipmap obtained from a reader.
    ///
    /// \param reader             The reader to be used to obtain the mipmap. Needs to support
    ///                           absolute access.
    /// \param container_filename The resolved filename of the container itself.
    /// \param member_filename    The relative filename of the mipmap in the container.
    /// \param selector           The selector (or \c nullptr).
    /// \param only_first_level   Indicates whether only the first (or all) miplevels should be
    ///                           read from the reader.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -1: Invalid reader.
    ///                           -  -3: No image plugin found to handle the data.
    ///                           -  -6: The reader does not support absolute access.
    ///                           -  -7: The image plugin failed to import the data.
    ///                           - -10: Failure to apply the given selector.
    /// \return                   The requested mipmap, or a dummy mipmap with a 1x1 pink pixel in
    ///                           case of errors.
    virtual IMipmap* create_mipmap(
        Container_based,
        mi::neuraylib::IReader* reader,
        const std::string& container_filename,
        const std::string& member_filename,
        const char* selector,
        bool only_first_level = true,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates a memory-based mipmap obtained from a reader.
    ///
    /// \param reader             The reader to be used to obtain the mipmap. Needs to support
    ///                           absolute access.
    /// \param image_format       The image format of the buffer.
    /// \param selector           The selector (or \c nullptr).
    /// \param mdl_file_path      The resolved MDL file path (to be used for log messages only),
    ///                           or \c nullptr in other contexts.
    /// \param only_first_level   Indicates whether only the first (or all) miplevels should be
    ///                           read from the reader.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -1: Invalid reader.
    ///                           -  -3: No image plugin found to handle the data.
    ///                           -  -6: The reader does not support absolute access.
    ///                           -  -7: The image plugin failed to import the data.
    ///                           - -10: Failure to apply the given selector.
    /// \return                   The requested mipmap, or a dummy mipmap with a 1x1 pink pixel in
    ///                           case of errors.
    virtual IMipmap* create_mipmap(
        Memory_based,
        mi::neuraylib::IReader* reader,
        const char* image_format,
        const char* selector,
        const char* mdl_file_path = nullptr,
        bool only_first_level = true,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates a mipmap from the given canvas.
    ///
    /// This method creates the actual miplevels. See method below for wrapping them into an
    /// instance of IMipmap. Note that the \p base_canvas is \em not included in \p mipmaps.
    ///
    /// \param mipmaps            A vector to which the created miplevels are written.
    /// \param base_canvas        The canvas to create the miplevels from.
    /// \param gamma              An optional gamma value. If this value is different from
    ///                           zero it is used instead of the canvas gamma.
    virtual void create_mipmap(
        std::vector<mi::base::Handle<mi::neuraylib::ICanvas> >& mipmaps,
        const mi::neuraylib::ICanvas* base_canvas,
        mi::Float32 gamma = 0.0f) const = 0;

    /// Creates a mipmap from the given canvases.
    ///
    /// This method wraps the given canvases into a mipmap. See the method above for creating them.
    ///
    /// Note that the canvases are not copied, but shared. See copy_canvas() below if sharing is not
    /// desired.
    ///
    /// \param canvases           The array of canvases to create the mipmap from, starting with the
    ///                           base level.
    /// \param is_cubemap         Flag that indicates whether this mipmap represents a cubemap.
    /// \return                   The requested mipmap, or \c nullptr in case of invalid pointers in
    ///                           \c canvases.
    virtual IMipmap* create_mipmap(
        std::vector<mi::base::Handle<mi::neuraylib::ICanvas> >& canvases,
        bool is_cubemap = false) const = 0;

    /// Creates a dummy mipmap (1x1 pink pixel).
    virtual IMipmap* create_dummy_mipmap() = 0;

    /// Checks whether the mipmap is a dummy (1x1 pink pixel).
    virtual bool is_dummy_mipmap( const IMipmap* mipmap) const = 0;

    /// Creates a memory-based canvas with given pixel type, width, height, and layers.
    ///
    /// \param pixel_type         The desired pixel type.
    /// \param width              The desired width.
    /// \param height             The desired height.
    /// \param layers             The desired number of layers (depth).
    /// \param is_cubemap         Flag that indicates whether this canvas represents a cubemap.
    /// \param gamma              The desired gamma value. The special value 0.0 represents the
    ///                           default gamma which is 1.0 for HDR pixel types and 2.2 for LDR
    ///                           pixel types.
    /// \return                   The requested canvas, or \c nullptr in case of invalid pixel type,
    ///                           width, height, layers, or cubemap flag, or memory allocation
    ///                           failures.
    virtual mi::neuraylib::ICanvas* create_canvas(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height,
        mi::Uint32 layers = 1,
        bool is_cubemap = false,
        mi::Float32 gamma = 0.0f) const = 0;

    /// Creates a file-based canvas that represents the given file on disk.
    ///
    /// \param filename           The file that shall be represented by this canvas.
    /// \param selector           The selector (or \c nullptr).
    /// \param miplevel           The miplevel in the file that shall be represented by this canvas.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -3: No image plugin found to handle the file.
    ///                           -  -5: Failure to open the file.
    ///                           -  -7: The image plugin failed to import the file.
    ///                           - -10: Failure to apply the given selector.
    /// \return                   The requested canvas, or a dummy canvas with a 1x1 pink pixel in
    ///                           case of errors.
    virtual mi::neuraylib::ICanvas* create_canvas(
        File_based,
        const std::string& filename,
        const char* selector,
        mi::Uint32 miplevel = 0,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates a memory-based canvas obtained from a reader.
    ///
    /// \param reader             The reader to be used to obtain the canvas. Needs to support
    ///                           absolute access.
    /// \param container_filename The resolved filename of the container itself.
    /// \param member_filename    The relative filename of the canvas in the container.
    /// \param selector           The selector (or \c nullptr).
    /// \param miplevel           The miplevel in the reader that shall be represented by this
    ///                           canvas.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -1: Invalid reader.
    ///                           -  -3: No image plugin found to handle the data.
    ///                           -  -6: The reader does not support absolute access.
    ///                           -  -7: The image plugin failed to import the data.
    ///                           - -10: Failure to apply the given selector.
    /// \return                   The requested canvas, or a dummy canvas with a 1x1 pink pixel in
    ///                           case of errors.
    virtual mi::neuraylib::ICanvas* create_canvas(
        Container_based,
        mi::neuraylib::IReader* reader,
        const std::string& container_filename,
        const std::string& member_filename,
        const char* selector,
        mi::Uint32 miplevel = 0,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates a memory-based canvas that represents encoded pixel data (as buffer in memory).
    ///
    /// This method is similar to the method above, except that it does not decode the pixel data
    /// from a file on disk, but from a buffer in memory. The method is the counterpart of
    /// #create_buffer_from_canvas().
    ///
    /// \param reader             The reader to be used to obtain the canvas. Needs to support
    ///                           absolute access.
    /// \param image_format       The image format of the buffer.
    /// \param selector           The selector (or \c nullptr).
    /// \param mdl_file_path      The resolved MDL file path (to be used for log messages only),
    ///                           or \c nullptr in other contexts.
    /// \param miplevel           The miplevel in the buffer that shall be represented by this
    ///                           canvas.
    /// \param[out] errors        An optional pointer to an #mi::Sint32 to which an error code will
    ///                           be written. The error codes have the following meaning:
    ///                           -   0: Success.
    ///                           -  -1: Invalid reader.
    ///                           -  -3: No image plugin found to handle the data.
    ///                           -  -6: The reader does not support absolute access.
    ///                           -  -7: The image plugin failed to import the data.
    ///                           - -10: Failure to apply the given selector.
    /// \return                   The requested canvas, or a dummy canvas with a 1x1 pink pixel in
    ///                           case of errors.
    virtual mi::neuraylib::ICanvas* create_canvas(
        Memory_based,
        mi::neuraylib::IReader* reader,
        const char* image_format,
        const char* selector,
        const char* mdl_file_path = nullptr,
        mi::Uint32 miplevel = 0,
        mi::Sint32* errors = nullptr) const = 0;

    /// Creates a memory-based canvas with given array of tiles.
    ///
    /// \param tile               The array of tiles the canvas will be made of (in z-direction).
    ///                           Note that the tiles are not copied, but shared. See copy_tile()
    ///                           below if sharing is not desired.
    /// \param gamma              The desired gamma value. The special value 0.0 represents the
    ///                           default gamma which is 1.0 for HDR pixel types and 2.2 for LDR
    ///                           pixel types. Note that the pixel data itself is not changed.
    /// \return                   The requested canvas, or \c nullptr in case of invalid \c tile
    ///                           pointers.
    virtual mi::neuraylib::ICanvas* create_canvas(
        const std::vector<mi::base::Handle<mi::neuraylib::ITile>>& tiles,
        mi::Float32 gamma = 0.0f) const = 0;

    /// Creates a tile with given pixel type, width, and height.
    ///
    /// \param pixel_type         The desired pixel type.
    /// \param width              The desired width.
    /// \param height             The desired height.
    /// \return                   The requested tile, or \c nullptr in case of invalid pixel type,
    ///                           width, or height, or memory allocation failures.
    virtual mi::neuraylib::ITile* create_tile(
        Pixel_type pixel_type,
        mi::Uint32 width,
        mi::Uint32 height) const = 0;

    // Methods to create copies of mipmaps, canvases, and tiles
    // ========================================================

    /// Creates a copy of the passed mipmap.
    ///
    /// Note that the copy created by this method is always a memory-based mipmap (since there is
    /// no way to find out whether the mipmap is file-based or memory-based). If you want to create
    /// a copy of a file-based mipmap and know the file name, simply use #create_mipmap() instead.
    ///
    /// Note that by default only the first mipmap level will actually be copied. The other levels
    /// will be recomputed when needed.
    virtual IMipmap* copy_mipmap( const IMipmap* other, bool only_first_level = true) const = 0;

    /// Creates a copy of the passed canvas.
    ///
    /// Note that the copy created by this method is always a memory-based canvas (since there is
    /// no way to find out whether the canvas is file-based or memory-based). If you want to create
    /// a copy of a file-based canvas and know the file name, simply use #create_canvas() instead.
    virtual mi::neuraylib::ICanvas* copy_canvas( const mi::neuraylib::ICanvas* other) const = 0;

    /// Creates a copy of the passed tile.
    virtual mi::neuraylib::ITile* copy_tile( const mi::neuraylib::ITile* other) const = 0;

    /// Copies pixel data between two mipmaps.
    ///
    /// Source and destination have to agree on all properties (pixel type, width, height, etc.).
    ///
    /// \param source   Source of the pixel data.
    /// \param dest     Destination of the pixel data.
    /// return
    ///          -  0: Success.
    ///          - -1: Different properties.
    ///          - -2: Invalid pixel type.
    virtual mi::Sint32 copy_mipmap_data(
        const IMipmap* source, IMipmap* dest) const = 0;

    /// Copies pixel data between two canvasses.
    ///
    /// Source and destination have to agree on all properties (pixel type, width, height, etc.).
    ///
    /// \param source   Source of the pixel data.
    /// \param dest     Destination of the pixel data.
    /// return
    ///          -  0: Success.
    ///          - -1: Different properties.
    ///          - -2: Invalid pixel type.
    virtual mi::Sint32 copy_canvas_data(
        const mi::neuraylib::ICanvas* source, mi::neuraylib::ICanvas* dest) const = 0;

    /// Copies pixel data between two tiles.
    ///
    /// Source and destination have to agree on all properties (pixel type, width, height, etc.).
    ///
    /// \param source   Source of the pixel data.
    /// \param dest     Destination of the pixel data.
    /// return
    ///          -  0: Success.
    ///          - -1: Different properties.
    ///          - -2: Invalid pixel type.
    virtual mi::Sint32 copy_tile_data(
        const mi::neuraylib::ITile* source, mi::neuraylib::ITile* dest) const = 0;

    // Methods to convert pixel types of mipmaps, canvases, and tiles
    // ==============================================================

    /// Converts a mipmap to a given pixel type.
    ///
    /// Note that this method creates a copy if the passed-in mipmap already has the desired pixel
    /// type. (It cannot return the passed-in mipmap since this would require a const cast.) If
    /// performance is critical, you should compare pixel types yourself and skip the method call
    /// if it is not needed.)
    ///
    /// Note that by default only the first mipmap level will actually be converted. The other
    /// levels will be recomputed when needed.
    ///
    /// \param mipmap           The mipmap to convert.
    /// \param pixel_type       The desired pixel type.
    /// \param only_first_level Indicates whether only the first (or all) miplevels should be
    ///                         converted.
    /// \return                 The mipmap with the desired pixel type, or \c nullptr in case of
    ///                         invalid pixel types.
    virtual IMipmap* convert_mipmap(
        const IMipmap* mipmap,
        Pixel_type pixel_type,
        bool only_first_level = true) const = 0;

    /// Converts a canvas to a given pixel type.
    ///
    /// Note that this method creates a copy if the passed-in canvas already has the desired pixel
    /// type. (It cannot return the passed-in canvas since this would require a const cast.) If
    /// performance is critical, you should compare pixel types yourself and skip the method call
    /// if it is not needed.)
    ///
    /// \param canvas           The canvas to convert.
    /// \param pixel_type       The desired pixel type.
    /// \return                 The canvas with the desired pixel type, or \c nullptr in case of
    ///                         invalid pixel types.
    virtual mi::neuraylib::ICanvas* convert_canvas(
        const mi::neuraylib::ICanvas* canvas,
        Pixel_type pixel_type) const = 0;

    /// Converts a tile to a given pixel type.
    ///
    /// Note that this method creates a copy if the passed-in tile already has the desired pixel
    /// type. (It cannot return the passed-in tile since this would require a const cast.) If
    /// performance is critical, you should compare pixel types yourself and skip the method call
    /// if it is not needed.)
    ///
    /// \param tile             The tile to convert.
    /// \param pixel_type       The desired pixel type.
    /// \return                 The tile with the desired pixel type, or \c nullptr in case of
    ///                         invalid pixel types.
    virtual mi::neuraylib::ITile* convert_tile(
        const mi::neuraylib::ITile* tile,
        Pixel_type pixel_type) const = 0;

    // Methods to adjust gamma value of tiles and canvases
    // ===================================================

    /// Sets the gamma value of a tile and adjusts the pixel data accordingly.
    ///
    /// \note Gamma adjustments are always done in pixel type "Color" or "Rgb_fp". If necessary,
    ///       the pixel data is converted forth and back automatically (which needs temporary
    ///       buffers).
    ///
    /// \param tile             The tile whose pixel data is to adjust.
    /// \param new_gamma        The new gamma value.
    /// \param old_gamma        The old gamma value.
    virtual void adjust_gamma(
        mi::neuraylib::ITile* tile, mi::Float32 old_gamma, mi::Float32 new_gamma) const = 0;

    /// Sets the gamma value of a canvas and adjusts the pixel data accordingly.
    ///
    /// \note Gamma adjustments are always done in pixel type "Color" or "Rgb_fp". If necessary,
    ///       the pixel data is converted forth and back automatically (which needs temporary
    ///       buffers).
    ///
    /// \param canvas           The canvas whose pixel data is to adjust.
    /// \param new_gamma        The new gamma value.
    virtual void adjust_gamma( mi::neuraylib::ICanvas* canvas, mi::Float32 new_gamma) const = 0;

    // Methods to select "R", "G", "B", or "A" channel of mipmaps, canvases, and tiles
    // ===============================================================================

    /// Returns the pixel type of an RGBA channel.
    ///
    /// Invalid pixel type/selector combinations are:
    /// - \p pixel_type is not an RGB or RGBA pixel type
    /// - \p selector is not an RGBA channel selector
    ///
    /// \param pixel_type   The pixel type of the mipmap/canvas/tile.
    /// \param selector     The RGBA channel selector.
    /// \return             Returns PT_UNDEF for invalid pixel type/selector combinations.
    ///                     Otherwise, returns PT_SINT8 or PT_FLOAT32, depending on
    ///                     \p pixel_type.
    virtual Pixel_type get_pixel_type_for_channel(
        Pixel_type pixel_type, const char* selector) const = 0;

    /// Extracs an RGBA channel from a mipmap.
    ///
    /// \param mipmap           The mipmap to extract a channel from.
    /// \param selector         The RGBA channel selector.
    /// \param only_first_level Indicates whether only the first (or all) miplevels should be
    ///                         extracted.
    /// \return                 The extracted channel, or \c nullptr in case of invalid pixel type/
    ///                         channel selector combinations (see #get_pixel_type_for_channel()).
    virtual IMipmap* extract_channel(
        const IMipmap* mipmap, const char* selector, bool only_first_level = true) const = 0;

    /// Extracs an RGBA channel from a canvas.
    ///
    /// \param canvas           The canvas to extract a channel from.
    /// \param selector         The RGBA channel selector.
    /// \return                 The extracted channel, or \c nullptr in case of invalid pixel type/
    ///                         channel selector combinations (see #get_pixel_type_for_channel()).
    virtual mi::neuraylib::ICanvas* extract_channel(
        const mi::neuraylib::ICanvas* canvas, const char* selector) const = 0;

    /// Extracs an RGBA channel from a tile.
    ///
    /// \param tile             The tile to extract a channel from.
    /// \param selector         The RGBA channel selector.
    /// \return                 The extracted channel, or \c nullptr in case of invalid pixel type/
    ///                         channel selector combinations (see #get_pixel_type_for_channel()).
    virtual mi::neuraylib::ITile* extract_channel(
        const mi::neuraylib::ITile* tile, const char* selector) const = 0;

    // Methods to serialize/deserialize canvases and tiles
    // ===================================================

    /// Serializes a mipmap to the given serializer.
    ///
    /// Note that this method always serializes all the pixel data (since there is no way to find
    /// out whether the canvas is file-based or memory-based). If you want to serialize a
    /// file-based canvas and you know the file name, simply serialize the file name instead.
    ///
    /// Note that by default only the first mipmap level will actually be serialized. The other
    /// levels will be recomputed when needed.
    virtual void serialize_mipmap(
        SERIAL::Serializer* serializer,
        const IMipmap* mipmap,
        bool only_first_level = true) const = 0;

    /// Deserializes a mipmap from the given deserializer.
    virtual IMipmap* deserialize_mipmap(
        SERIAL::Deserializer* deserializer) const = 0;

    /// Serializes a canvas to the given serializer.
    ///
    /// Note that this method always serializes all the pixel data (since there is no way to find
    /// out whether the canvas is file-based or memory-based). If you want to serialize a
    /// file-based canvas and you know the file name, simply serialize the file name instead.
    virtual void serialize_canvas(
        SERIAL::Serializer* serializer, const mi::neuraylib::ICanvas* canvas) const = 0;

    /// Deserializes a canvas from the given deserializer.
    virtual mi::neuraylib::ICanvas* deserialize_canvas(
        SERIAL::Deserializer* deserializer) const = 0;

    /// Serializes a tile to the given serializer.
    virtual void serialize_tile(
        SERIAL::Serializer* serializer, const mi::neuraylib::ITile* tile) const = 0;

    /// Deserializes a tile from the given deserializer.
    virtual mi::neuraylib::ITile* deserialize_tile(
        SERIAL::Deserializer* deserializer) const = 0;

    // Methods to export mipmaps and canvases
    // ======================================

    /// Exports a canvas to an image file.
    ///
    /// \param image                 The image to export.
    /// \param output_filename       The filename for the exported image.
    /// \param export_options        See <mi/neuraylib/iimage_api.h> for supported export options.
    ///                              Can be \c nullptr.
    /// \return                      \c true in case of success, \c false in case of failure.

    virtual bool export_canvas(
        const mi::neuraylib::ICanvas* image,
        const char* output_filename,
        const mi::IMap* export_options = nullptr) const = 0;

    /// Exports a mipmap to an image file.
    ///
    /// \param image                 The image to export.
    /// \param output_filename       The filename for the exported image.
    /// \param export_options        See <mi/neuraylib/iimage_api.h> for supported export options.
    ///                              Can be \c nullptr.
    /// \return                      \c true in case of success, \c false in case of failure.
    virtual bool export_mipmap(
        const IMipmap* image,
        const char* output_filename,
        const mi::IMap* export_options = nullptr) const = 0;

    /// Creates a buffer with encoded image data from a canvas.
    ///
    /// This method is similar to the method #export_canvas(), except that it does not write the
    /// encoded pixel data to a file on disk, but creates a buffer in memory with the encoded pixel
    /// data. This method is the counterpart of #create_canvas(mi::neuraylib::IBuffer,...).
    ///
    /// \param canvas                The canvas to encode.
    /// \param image_format          The desired image format ("png", "jpg", etc.).
    /// \param pixel_type            The desired pixel type. Ignored if the plugin for the file
    ///                              format does not support the requested pixel type.
    /// \param export_options        See <mi/neuraylib/iimage_api.h> for supported export options.
    ///                              Can be \c nullptr.
    /// \return                      The encoded image, or \c nullptr in case of failure.
    virtual mi::neuraylib::IBuffer* create_buffer_from_canvas(
        const mi::neuraylib::ICanvas* canvas,
        const char* image_format,
        const char* pixel_type,
        const mi::IMap* export_options = nullptr) const = 0;

    // Misc methods
    // ============

    /// Finds a suitable plugin for import.
    ///
    /// \param extension   The extension of the file intended to import, may be \c nullptr.
    /// \param reader      The reader to use for the test buffer. Calls rewind() after the file
    ///                    header has been read. \c nullptr can be passed to skip this test.
    /// \return            One of the plugins of highest priority that support the extension
    ///                    \p extension and where mi::neuraylib::IImage_plugin::test() succeeds (if
    ///                    a reader was passed), or \c nullptr in case of failure.
    virtual mi::neuraylib::IImage_plugin* find_plugin_for_import(
        const char* extension, mi::neuraylib::IReader* reader) const = 0;

    /// Finds a suitable plugin for export.
    ///
    /// \param extension   The extension of the file intended to export, may not be \c nullptr.
    /// \return            One of the plugins of highest priority that support the extension
    ///                    \p extension, or \c nullptr in case of failure.
    virtual mi::neuraylib::IImage_plugin* find_plugin_for_export( const char* extension) const = 0;

    /// Sets the callback to support lazy loading of images in MDL containers and MDLE.
    ///
    /// Pass \c nullptr to clear the callback. Not thread-safe. Resetting during runtime causes
    /// errors for not yet loaded tiles.
    virtual void set_mdl_container_callback( IMdl_container_callback* mdl_container_callback) = 0;

    /// Returns the callback to support lazy loading of images in MDL containers and MDLE.
    ///
    /// ... or \c nullptr if no callback is set.
    virtual IMdl_container_callback* get_mdl_container_callback() const = 0;

    /// Creates the next miplevel from the given canvas.
    ///
    /// \param prev_canvas      The canvas to create a miplevel from.
    /// \param gamma_override   Canvas gamma override. If it is different from zero
    ///                         it is used instead of the canvas gamma.
    /// \return The canvas created for the next miplevel
    virtual mi::neuraylib::ICanvas* create_miplevel(
        const mi::neuraylib::ICanvas* prev_canvas, float gamma_override) const = 0;

    /// Converts the legacy export options into a map.
    ///
    /// This methods maps \p quality to \c "jpg:quality" and \c "exr:data_type", and
    /// \p force_default_gamma to \c "force_default_gamma".
    virtual mi::IMap* convert_legacy_options(
        mi::Uint32 quality, bool force_default_gamma) const = 0;

    // Methods for testing
    // ===================

    /// Dumps all registered plugins with their name, supported file extensions, and pixel types.
    ///
    /// For testing only.
    virtual void dump() const = 0;
};

/// Callback to support lazy loading of images in MDL containers and MDLE
class IMdl_container_callback : public
    mi::base::Interface_declare<0x039d55cf,0xd57f,0x4ef8,0x8e,0xb4,0xc7,0x2b,0x9e,0x77,0x02,0x96>
{
public:
    /// Returns a reader for a file in an MDL container, MDLE, or \c nullptr in case of failure.
    virtual mi::neuraylib::IReader* get_reader(
        const char* container_filename, const char* member_filename) = 0;
};

} // namespace IMAGE

} // namespace MI

#endif // IO_IMAGE_IMAGE_I_IMAGE_H
