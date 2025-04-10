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

/// \file
/// \brief Collection of useful tools.

#ifndef BASE_DATA_ATTR_TEST_UTILITIES_H
#define BASE_DATA_ATTR_TEST_UTILITIES_H

#include "i_attr_type.h"
#include "i_attr_types.h"

#include <string>
#include <cstring>

#include <mi/math/vector.h>
#include <base/system/stlext/i_stlext_no_unused_variable_warning.h>
#include <base/data/db/i_db_tag.h>

using Vector2 = mi::math::Vector<mi::Float32,2>;
using Vector3 = mi::math::Vector<mi::Float32,3>;
using Vector4 = mi::math::Vector<mi::Float32,4>;

namespace MI {
namespace ATTR {

class Type;

// Set the given \p data at the looked-up \p address to the given \p value.
template <typename T>
inline void set_value(
    const Type& root,                                   // the root's type
    char* data,                                         // the data block
    const char* address,                                // the name of the address
    const T& value)                                     // the given value
{
    char *ret_address;
    const Type* type = root.lookup(address, data, &ret_address);
    if(type) {
        Uint size = type->get_arraysize();
        STLEXT::no_unused_variable_warning_please( size);
        ASSERT(M_ATTR, size != 0); // dynamic arrays are note supported in this variant
        ASSERT(M_ATTR,
            type->sizeof_one()/size == sizeof(value) || type->get_typecode() == TYPE_CALL);
        const char* ptr = reinterpret_cast<const char*>(&value);
        memcpy(ret_address,ptr,sizeof(value));
    } else {
        ASSERT(M_ATTR, !"set_value<T> failed!");
    }
}

//--------------------------------------------------------------------------------------------------

// Set the given \p data at the looked-up \p address to the given \p value. Specialization for
// values of type const char*.
inline void set_value(
    const Type& root,           // the root's type
    char* data,                 // the data block
    const char* address,        // the name of the address
    const char* value)          // the given value
{
    char *ret_address;
    if(root.lookup(address, data, &ret_address)) {
        char* ptr = nullptr;
        if (value) {
            const size_t len = strlen(value) + 1;
            ptr = new char[len];
            memcpy(ptr, value, len);
        }
        char** data_ptr = reinterpret_cast<char**>(ret_address);
        free(*data_ptr);
        *data_ptr = ptr;
    } else {
        ASSERT(M_ATTR, !"set_value<string> failed!");
    }
}


//--------------------------------------------------------------------------------------------------

// Set the given \p data at the looked-up \p address to the given \p value. Specialization for
// values of type string.
template <>
inline void set_value<std::string>(
    const Type& root,                                   // the root's type
    char* data,                                         // the data block
    const char* address,                                // the name of the address
    const std::string& value)                           // the given value
{
    set_value(root,data,address,value.c_str());
}


//--------------------------------------------------------------------------------------------------

// Set the given \p data at the looked-up \p address to the given \p value. Specialization for
// values of type Tag.
template <>
inline void set_value<DB::Tag>(
    const Type& root,                                   // the root's type
    char* data,                                         // the data block
    const char* address,                                // the name of the address
    const DB::Tag& value)                               // the given value
{
    char *ret_address;
    if(root.lookup(address, data, &ret_address)) {
        Uint32 val = value.get_uint();
        const char* ptr = reinterpret_cast<const char*>(&val);
        memcpy(ret_address,ptr,sizeof(Uint32));
    } else {
        ASSERT(M_ATTR, !"set_value<Tag> failed!");
    }
}


//--------------------------------------------------------------------------------------------------

/// Set the given \p data at the looked-up \p address to the given \p value. Overload for
/// dynamic arrays. The attribute (or rather \p data) takes over owenership of value.m_value.
template<>
inline void set_value<ATTR::Dynamic_array>(
    const Type& root,                                   // the root's type
    char* data,                                         // the data block
    const char* address,                                // the name of the address
    const ATTR::Dynamic_array& value)                   // the given value
{
    char *ret_address;
    root.lookup(address, data, &ret_address);
    ATTR::Dynamic_array* array = (ATTR::Dynamic_array*)(ret_address);
    // This method cannot replace or resize existing dynamic arrays. Just freeing array->m_value
    // is not enough since it might contain strings, other dynamic arrays, etc.
    ASSERT(M_ATTR, !array->m_value);
    array->m_count = value.m_count;
    array->m_value = value.m_value;
}

//--------------------------------------------------------------------------------------------------

// Set the given \p data at the looked-up \p address to the given \p value (wrapper for
// std::string addresses).
template <typename T>
inline void set_value(
    const Type& root,                                   // the root's type
    char* data,                                         // the data block
    const std::string& address,                 // the name of the address
    const T& value)                                     // the given value
{
    set_value<T>(root,data,address.c_str(),value);
}


//--------------------------------------------------------------------------------------------------

/// Get the given \p data at the looked-up \p address as type T.
template<typename T>
inline T get_value(
    const Type& root,                                   // the root's type
    const char* data,                                   // the data block
    const char* address)                                // the name of the address
{
    if (!address)
       return *reinterpret_cast<const T *>(data);
    char *ret_address;
    const Type* type = root.lookup(address, const_cast<char *>(data), &ret_address);
    if (!type)
        return T(0);
    return *reinterpret_cast<T *>(ret_address);
}

/// Get the given \p data at the looked-up \p address as type T.
/// Specialization for type bool.
template<>
inline bool get_value<bool>(
    const Type& root,                           // the root's type
    const char* data,                           // the data block
    const char* address)                        // the name of the address
{
    if (!address)
       return !!*data;
    char *ret_address;
    const Type* type = root.lookup(address, const_cast<char *>(data), &ret_address);
    if (!type)
        return false;
    return !!*reinterpret_cast<char *>(ret_address);
}

/// Get the given \p data at the looked-up \p address as type T (wrapper for std::string
/// addresses).
template<typename T>
inline T get_value(
    const Type& root,                                   // the root's type
    const char* data,                                   // the data block
    const std::string& address)                 // the name of the address
{
    return get_value<T>(root,data,address.c_str());
}


}
}

#endif

