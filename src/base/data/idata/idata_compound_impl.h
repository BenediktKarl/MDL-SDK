/***************************************************************************************************
 * Copyright (c) 2010-2025, NVIDIA CORPORATION. All rights reserved.
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
 ** \brief Header for the ICompound implementations.
 **/

#ifndef BASE_DATA_IDATA_IDATA_COMPOUND_IMPL_H
#define BASE_DATA_IDATA_IDATA_COMPOUND_IMPL_H

#include <mi/neuraylib/ibbox.h>
#include <mi/neuraylib/icolor.h>
#include <mi/neuraylib/icompound.h>
#include <mi/neuraylib/imatrix.h>
#include <mi/neuraylib/ispectrum.h>
#include <mi/neuraylib/ivector.h>

#include <string>
#include <vector>

#include <boost/core/noncopyable.hpp>

// see documentation of mi::base::Interface_merger
#include <mi/base/config.h>
#ifdef MI_COMPILER_MSC
#pragma warning( disable : 4505 )
#endif

#include <mi/base/interface_implement.h>
#include <mi/base/interface_merger.h>
#include <mi/base/handle.h>

#include "idata_interfaces.h"

namespace MI {

namespace DB { class Transaction; }

namespace IDATA {

class Factory;

/// Default/proxy implementation of interfaces derived from ICompound
///
/// The default implementation owns the memory used to store the actual value. The proxy
/// implementation does not own the memory used to store the actual value. In contrast to INumber
/// where two implementations Number_impl and Number_impl_proxy are used, there is only one
/// implementation class Compound_class for ICompound for both use cases. (One strong reason for
/// this is the enormous code bloat, another reason is the array-like access to the actual data
/// which makes it easy to handle both use cases in one class).
///
/// The constructor creates an instance in default mode. The first call of set_pointer_and_owner()
/// switches the instance permanently to proxy mode.
///
/// Note that only a fixed set of types is permitted for the template parameters I and T.
/// Hence we use explicit template instantiation in the corresponding .cpp file.
template<typename I, typename T, mi::Size ROWS, mi::Size COLUMNS>
class Compound_impl
  : public mi::base::Interface_merger<mi::base::Interface_implement<I>, IProxy>,
    public boost::noncopyable
{
public:

    /// Constructor
    ///
    /// All values are initialized to T( 0).
    Compound_impl( const Factory* factory);

    ~Compound_impl();

    // public API methods (IData)

    const char* get_type_name() const final;

    // public API methods (IData_compound)

    const char* get_key( mi::Size index) const final;

    bool has_key( const char* key) const final;

    const mi::base::IInterface* get_value( const char* key) const final;

    mi::base::IInterface* get_value( const char* key) final;

    const mi::base::IInterface* get_value( mi::Size index) const final;

    mi::base::IInterface* get_value( mi::Size index) final;

    mi::Sint32 set_value( const char* key, mi::base::IInterface* value) final;

    mi::Sint32 set_value( mi::Size index, mi::base::IInterface* value) final;

    // public API methods (ICompound)

    mi::Size get_number_of_rows() const final;

    mi::Size get_number_of_columns() const final;

    mi::Size get_length() const final;

    const char* get_element_type_name() const final;

    bool get_value( mi::Size row, mi::Size column, bool& value) const final;

    bool get_value( mi::Size row, mi::Size column, mi::Sint32& value) const final;

    bool get_value( mi::Size row, mi::Size column, mi::Uint32& value) const final;

    bool get_value( mi::Size row, mi::Size column, mi::Float32& value) const final;

    bool get_value( mi::Size row, mi::Size column, mi::Float64& value) const final;

    bool set_value( mi::Size row, mi::Size column, bool value) final;

    bool set_value( mi::Size row, mi::Size column, mi::Sint32 value) final;

    bool set_value( mi::Size row, mi::Size column, mi::Uint32 value) final;

    bool set_value( mi::Size row, mi::Size column, mi::Float32 value) final;

    bool set_value( mi::Size row, mi::Size column, mi::Float64 value) final;

    void get_values( bool* values) const final;

    void get_values( mi::Sint32* values) const final;

    void get_values( mi::Uint32* values) const final;

    void get_values( mi::Float32* values) const final;

    void get_values( mi::Float64* values) const final;

    void set_values( const bool* values) final;

    void set_values( const mi::Sint32* values) final;

    void set_values( const mi::Uint32* values) final;

    void set_values( const mi::Float32* values) final;

    void set_values( const mi::Float64* values) final;

    // internal methods (of IProxy)

    void set_pointer_and_owner( void* pointer, const mi::base::IInterface* owner) final;

    void release_referenced_memory() final;

protected:
    /// Storage.
    ///
    /// Owned by this instance if \p m_owner is invalid, otherwise by that owner.
    T* m_storage;

    /// The type name of the compound itself.
    std::string m_type_name;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    ///
    /// Note that this generic implementation always returns false.
    virtual bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    ///
    /// Note that this generic implementation always returns false.
    virtual bool index_to_key( mi::Size index, std::string& key) const;

    /// The class factory.
    const Factory* m_factory = nullptr;

    /// Owner of the storage.
    ///
    /// The class uses reference counting on the owner to ensure that the pointer to the storage
    /// is valid. An invalid handle indicates that the instance is in default mode, not proxy mode
    /// (in other words the instance itself owns the storage).
    mi::base::Handle<const mi::base::IInterface> m_owner;

    /// Caches the last return value of get_key().
    mutable std::string m_cached_key;
};

/// Default/proxy implementation of vector-like interfaces derived from ICompound
template<typename I, typename T, mi::Size ROWS>
class Vector_impl
  : public Compound_impl<I,T,ROWS,1>
{
public:

    using Base = Compound_impl<I,T,ROWS,1>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Vector_impl( const Factory* factory);

    // public API methods

    mi::math::Vector_struct<T,ROWS> get_value() const final;

    void get_value( mi::math::Vector_struct<T,ROWS>& value) const final;

    void set_value( const mi::math::Vector_struct<T,ROWS>& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};


/// Default/proxy implementation of matrix-like interfaces derived from ICompound
template<typename I, typename T, mi::Size ROWS, mi::Size COLUMNS>
class Matrix_impl
  : public Compound_impl<I,T,ROWS,COLUMNS>
{
public:

    using Base = Compound_impl<I,T,ROWS,COLUMNS>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Matrix_impl( const Factory* factory);

    // public API methods

    mi::math::Matrix_struct<T,ROWS,COLUMNS> get_value() const final;

    void get_value( mi::math::Matrix_struct<T,ROWS,COLUMNS>& value) const final;

    void set_value( const mi::math::Matrix_struct<T,ROWS,COLUMNS>& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};

/// Default/proxy implementation of IColor
class Color_impl
  : public Compound_impl<mi::IColor,mi::Float32,4,1>
{
public:

    using Base = Compound_impl<mi::IColor,mi::Float32,4,1>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Color_impl( const Factory* factory);

    // public API methods

    mi::Color_struct get_value() const final;

    void get_value( mi::Color_struct& value) const final;

    void set_value( const mi::Color_struct& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};

/// Default/proxy implementation of IColor3
class Color3_impl
  : public Compound_impl<mi::IColor3,mi::Float32,3,1>
{
public:

    using Base = Compound_impl<mi::IColor3,mi::Float32,3,1>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Color3_impl( const Factory* factory);

    // public API methods

    mi::Color_struct get_value() const final;

    void get_value( mi::Color_struct& value) const final;

    void set_value( const mi::Color_struct& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};

/// Default/proxy implementation of ISpectrum
class Spectrum_impl
  : public Compound_impl<mi::ISpectrum,mi::Float32,3,1>
{
public:

    using Base = Compound_impl<mi::ISpectrum,mi::Float32,3,1>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Spectrum_impl( const Factory* factory);

    // public API methods

    mi::Spectrum_struct get_value() const final;

    void get_value( mi::Spectrum_struct& value) const final;

    void set_value( const mi::Spectrum_struct& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};

/// Default/proxy implementation of IBbox3
class Bbox3_impl
  : public Compound_impl<mi::IBbox3,mi::Float32,2,3>
{
public:

    using Base = Compound_impl<mi::IBbox3,mi::Float32,2,3>;

    static mi::base::IInterface* create_instance(
        const Factory* factory,
        DB::Transaction* transaction,
        mi::Uint32 argc,
        const mi::base::IInterface* argv[]);

    Bbox3_impl( const Factory* factory);

    // public API methods

    mi::Bbox3_struct get_value() const final;

    void get_value( mi::Bbox3_struct& value) const final;

    void set_value( const mi::Bbox3_struct& value) final;

    using Base::get_value;

    using Base::set_value;

private:
    /// Converts a given key to the corresponding index.
    ///
    /// If \p key is a valid key, return \c true and the corresponding index in \c index.
    /// Otherwise return \c false.
    bool key_to_index( const char* key, mi::Size& index) const;

    /// Converts a given index to the corresponding key.
    ///
    /// If \p index is a valid index, return \c true and the key in \c key.
    /// Otherwise return \c false.
    bool index_to_key( mi::Size index, std::string& key) const;
};

// The typedefs here are only for convenience for class registration.

using Boolean_2_impl = Vector_impl<mi::IBoolean_2, bool, 2>;
using Boolean_3_impl = Vector_impl<mi::IBoolean_3, bool, 3>;
using Boolean_4_impl = Vector_impl<mi::IBoolean_4, bool, 4>;

using Sint32_2_impl = Vector_impl<mi::ISint32_2, mi::Sint32, 2> ;
using Sint32_3_impl = Vector_impl<mi::ISint32_3, mi::Sint32, 3> ;
using Sint32_4_impl = Vector_impl<mi::ISint32_4, mi::Sint32, 4> ;

using Uint32_2_impl = Vector_impl<mi::IUint32_2, mi::Uint32, 2> ;
using Uint32_3_impl = Vector_impl<mi::IUint32_3, mi::Uint32, 3> ;
using Uint32_4_impl = Vector_impl<mi::IUint32_4, mi::Uint32, 4> ;

using Float32_2_impl = Vector_impl<mi::IFloat32_2, mi::Float32, 2>;
using Float32_3_impl = Vector_impl<mi::IFloat32_3, mi::Float32, 3>;
using Float32_4_impl = Vector_impl<mi::IFloat32_4, mi::Float32, 4>;

using Float64_2_impl = Vector_impl<mi::IFloat64_2, mi::Float64, 2>;
using Float64_3_impl = Vector_impl<mi::IFloat64_3, mi::Float64, 3>;
using Float64_4_impl = Vector_impl<mi::IFloat64_4, mi::Float64, 4>;

using Boolean_2_2_impl = Matrix_impl<mi::IBoolean_2_2, bool, 2, 2>;
using Boolean_2_3_impl = Matrix_impl<mi::IBoolean_2_3, bool, 2, 3>;
using Boolean_2_4_impl = Matrix_impl<mi::IBoolean_2_4, bool, 2, 4>;
using Boolean_3_2_impl = Matrix_impl<mi::IBoolean_3_2, bool, 3, 2>;
using Boolean_3_3_impl = Matrix_impl<mi::IBoolean_3_3, bool, 3, 3>;
using Boolean_3_4_impl = Matrix_impl<mi::IBoolean_3_4, bool, 3, 4>;
using Boolean_4_2_impl = Matrix_impl<mi::IBoolean_4_2, bool, 4, 2>;
using Boolean_4_3_impl = Matrix_impl<mi::IBoolean_4_3, bool, 4, 3>;
using Boolean_4_4_impl = Matrix_impl<mi::IBoolean_4_4, bool, 4, 4>;

using Sint32_2_2_impl = Matrix_impl<mi::ISint32_2_2, mi::Sint32, 2, 2> ;
using Sint32_2_3_impl = Matrix_impl<mi::ISint32_2_3, mi::Sint32, 2, 3> ;
using Sint32_2_4_impl = Matrix_impl<mi::ISint32_2_4, mi::Sint32, 2, 4> ;
using Sint32_3_2_impl = Matrix_impl<mi::ISint32_3_2, mi::Sint32, 3, 2> ;
using Sint32_3_3_impl = Matrix_impl<mi::ISint32_3_3, mi::Sint32, 3, 3> ;
using Sint32_3_4_impl = Matrix_impl<mi::ISint32_3_4, mi::Sint32, 3, 4> ;
using Sint32_4_2_impl = Matrix_impl<mi::ISint32_4_2, mi::Sint32, 4, 2> ;
using Sint32_4_3_impl = Matrix_impl<mi::ISint32_4_3, mi::Sint32, 4, 3> ;
using Sint32_4_4_impl = Matrix_impl<mi::ISint32_4_4, mi::Sint32, 4, 4> ;

using Uint32_2_2_impl = Matrix_impl<mi::IUint32_2_2, mi::Uint32, 2, 2> ;
using Uint32_2_3_impl = Matrix_impl<mi::IUint32_2_3, mi::Uint32, 2, 3> ;
using Uint32_2_4_impl = Matrix_impl<mi::IUint32_2_4, mi::Uint32, 2, 4> ;
using Uint32_3_2_impl = Matrix_impl<mi::IUint32_3_2, mi::Uint32, 3, 2> ;
using Uint32_3_3_impl = Matrix_impl<mi::IUint32_3_3, mi::Uint32, 3, 3> ;
using Uint32_3_4_impl = Matrix_impl<mi::IUint32_3_4, mi::Uint32, 3, 4> ;
using Uint32_4_2_impl = Matrix_impl<mi::IUint32_4_2, mi::Uint32, 4, 2> ;
using Uint32_4_3_impl = Matrix_impl<mi::IUint32_4_3, mi::Uint32, 4, 3> ;
using Uint32_4_4_impl = Matrix_impl<mi::IUint32_4_4, mi::Uint32, 4, 4> ;

using Float32_2_2_impl = Matrix_impl<mi::IFloat32_2_2, mi::Float32, 2, 2>;
using Float32_2_3_impl = Matrix_impl<mi::IFloat32_2_3, mi::Float32, 2, 3>;
using Float32_2_4_impl = Matrix_impl<mi::IFloat32_2_4, mi::Float32, 2, 4>;
using Float32_3_2_impl = Matrix_impl<mi::IFloat32_3_2, mi::Float32, 3, 2>;
using Float32_3_3_impl = Matrix_impl<mi::IFloat32_3_3, mi::Float32, 3, 3>;
using Float32_3_4_impl = Matrix_impl<mi::IFloat32_3_4, mi::Float32, 3, 4>;
using Float32_4_2_impl = Matrix_impl<mi::IFloat32_4_2, mi::Float32, 4, 2>;
using Float32_4_3_impl = Matrix_impl<mi::IFloat32_4_3, mi::Float32, 4, 3>;
using Float32_4_4_impl = Matrix_impl<mi::IFloat32_4_4, mi::Float32, 4, 4>;

using Float64_2_2_impl = Matrix_impl<mi::IFloat64_2_2, mi::Float64, 2, 2>;
using Float64_2_3_impl = Matrix_impl<mi::IFloat64_2_3, mi::Float64, 2, 3>;
using Float64_2_4_impl = Matrix_impl<mi::IFloat64_2_4, mi::Float64, 2, 4>;
using Float64_3_2_impl = Matrix_impl<mi::IFloat64_3_2, mi::Float64, 3, 2>;
using Float64_3_3_impl = Matrix_impl<mi::IFloat64_3_3, mi::Float64, 3, 3>;
using Float64_3_4_impl = Matrix_impl<mi::IFloat64_3_4, mi::Float64, 3, 4>;
using Float64_4_2_impl = Matrix_impl<mi::IFloat64_4_2, mi::Float64, 4, 2>;
using Float64_4_3_impl = Matrix_impl<mi::IFloat64_4_3, mi::Float64, 4, 3>;
using Float64_4_4_impl = Matrix_impl<mi::IFloat64_4_4, mi::Float64, 4, 4>;

} // namespace IDATA

} // namespace MI

#endif // BASE_DATA_IDATA_IDATA_COMPOUND_IMPL_H
