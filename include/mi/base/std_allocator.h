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
/// \file mi/base/std_allocator.h
/// \brief Standard STL %allocator implementation.
///
/// The implementation is based on the #mi::base::IAllocator interface.
/// See \ref mi_base_iinterface.

#ifndef MI_BASE_STD_ALLOCATOR_H
#define MI_BASE_STD_ALLOCATOR_H

#include <mi/base/types.h>
#include <mi/base/iallocator.h>
#include <mi/base/default_allocator.h>

namespace mi {

namespace base {

/** \addtogroup mi_base_iallocator
@{
*/

/** An adaptor class template that implements a standard STL allocator.

    The implementation of the STL allocator is based on the #mi::base::IAllocator interface.

    \tparam T The value type of the allocator.

    \par Include File:
        <tt> \#include <mi/base/std_allocator.h></tt>
*/
template <class T>
class Std_allocator
{
    // Allocator interface used for memory management.
    IAllocator* m_alloc;
public:

    using value_type      = T;                ///< Value type allocated by this allocator.
    using pointer         = T*;               ///< Pointer type.
    using const_pointer   = const T*;         ///< Const pointer type.
    using reference       = T&;               ///< Reference type.
    using const_reference = const T&;         ///< Const reference type.
    using size_type       = std::size_t;      ///< Size type.
    using difference_type = std::ptrdiff_t;   ///< Difference type.

    /// Rebind helper struct to define a new class instance of this allocator template
    /// instantiated for the new value type \c T1.
    template <class T1> struct rebind
    {
        /// Rebind type, defines a new class instance of this allocator template
        /// instantiated for the new value type \c T1.
        using other = Std_allocator<T1>;
    };

    /// Default constructor.
    ///
    /// Uses #mi::base::Default_allocator.
    Std_allocator() noexcept
        : m_alloc( Default_allocator::get_instance()) {}

    /// Constructor.
    ///
    /// Constructor from an #mi::base::IAllocator interface.
    ///
    /// \param allocator An implementation of the #mi::base::IAllocator interface.
    ///        Can be \c nullptr in which case the #mi::base::Default_allocator
    ///        will be used.
    Std_allocator( base::IAllocator* allocator) noexcept
        : m_alloc( allocator ? allocator : Default_allocator::get_instance()) {}

    /// Copying constructor template for alike allocators of different value type.
    template <class T1>
    Std_allocator(const Std_allocator<T1>& other) noexcept
        : m_alloc( other.get_allocator()) {}

    /// Returns address of object \c x allocated through this allocator.
    pointer  address( reference x) const { return &x;}

    /// Returns const address of object \c x allocated through this allocator.
    const_pointer address(const_reference x) const { return &x; }

    /// Allocate uninitialized dynamic memory for \c n elements of type \c T.
    /// \return The pointer to the allocated memory. Can be \c nullptr if the
    ///         underlying IAllocator implementation fails to allocate the memory.
    T* allocate( size_type n, const void* = nullptr) noexcept
    {
        return reinterpret_cast<T*>( m_alloc->malloc( n * sizeof(value_type)));
    }

    /// Frees uninitialized  dynamic memory at location \p p that has previously
    /// been allocated with \c %allocate().
    /// \param p the memory to be freed. If \p p is equal to \c nullptr,
    ///          no operation is performed. However, note that according to
    //           the standard allocator concept \p p must not be \c nullptr.
    void deallocate(pointer p, size_type)
    {
        m_alloc->free( p);
    }

    /// Returns the maximum number of elements of type \c T that can be allocated
    /// using this allocator.
    size_type max_size() const noexcept  { return SIZE_MAX_VALUE / sizeof(value_type); }

    /// Calls the copy constructor of \c T on the location \p p with the argument
    /// \p value.
    void construct(pointer p, const_reference value) { new(p) T(value); }

    /// Calls the destructor of \c T on the location \p p.
    void destroy(pointer p) { p->~T(); }

    /// Returns the interface of the underlying allocator.
    IAllocator* get_allocator() const { return m_alloc; }

    /// Equality comparison.
    ///
    /// Returns \c true if the underlying IAllocator interface
    /// implementations are the same, because then these allocators can be used
    /// interchangeably for allocation and deallocation.
    template <class T2>
    bool operator== ( Std_allocator<T2> other) const noexcept
    {
        return m_alloc == other.get_allocator();
    }

    /// Inequality comparison.
    ///
    /// Returns \c false if the underlying IAllocator interface
    /// implementations are the same.
    template <class T2>
    bool operator!= ( Std_allocator<T2> other) const noexcept
    {
        return ! ((*this) == other);
    }
};

/*@}*/ // end group mi_base_iallocator

} // namespace base

} // namespace mi

#endif // MI_BASE_STD_ALLOCATOR_H
