/******************************************************************************
 * Copyright (c) 2012-2025, NVIDIA CORPORATION. All rights reserved.
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

#include "pch.h"

#include <map>

#include <mi/base/ilogger.h>
#include <mi/base/plugin.h>
#include <mi/mdl/mdl_code_generators.h>
#include <mi/neuraylib/iplugin_api.h>

#include <base/system/main/access_module.h>
#include <base/system/main/module_registration.h>
#include <base/lib/log/i_log_assert.h>
#include <base/lib/log/i_log_logger.h>
#include <base/lib/config/config.h>
#include <base/lib/plug/i_plug.h>
#include <base/util/registry/i_config_registry.h>
#include <base/data/serial/i_serializer.h>
#include <base/system/stlext/i_stlext_no_unused_variable_warning.h>

#include "mdlnr.h"
#include "mdlnr_search_path.h"
#include "mdlnr_module.h"

#include <io/scene/mdl_elements/i_mdl_elements_utilities.h>
#include <mdl/compiler/compilercore/compilercore_assert.h>
#include <mdl/compiler/compilercore/compilercore_fatal.h>
#include <mdl/compiler/compilercore/compilercore_debug_tools.h>
#include <mdl/compiler/compilercore/compilercore_mdl.h>
#include <mdl/compiler/compilercore/compilercore_file_utils.h>
#include <mdl/compiler/compilercore/compilercore_code_cache.h>
#include <mdl/compiler/compilercore/compilercore_errors.h>
#include <mi/mdl/mdl_distiller_node_types.h>

// Enable the MDL debug allocator in DEBUG builds
#if defined(DEBUG)
#define USE_MDL_DEBUG_ALLOCATOR
#endif

namespace MI {
namespace MDLC {

class Assert_helper : public mi::mdl::IAsserter {
    virtual void assertfailed(
        char const   *exp,
        char const   *file,
        unsigned int line)
    {
        ::MI::LOG::report_assertion_failure(M_MDLC, exp, file, line);
    }
};

class Fatal_helper : public mi::mdl::IFatal_error {
    virtual void fatal(
        char const *msg)
    {
        ::MI::LOG::mod_log->fatal(M_MDLC, MI::LOG::ILogger::C_COMPILER, "%s", msg);
    }
};

class DebugOutput : public mi::mdl::IOutput_stream {
public:
    /// Write a character to the output stream.
    virtual void write_char(char c) {
        ::MI::LOG::mod_log->info(M_MDLC, LOG::ILogger::C_RENDER, "%c", c);
    }

    /// Write a string to the stream.
    virtual void write(const char *string) {
        ::MI::LOG::mod_log->info(M_MDLC, LOG::ILogger::C_RENDER, "%s", string);
    }

    /// Flush stream.
    virtual void flush() {}

    /// Remove the last character from output stream if possible.
    ///
    /// \param c  remove this character from the output stream
    ///
    /// \return true if c was the last character in the stream and it was successfully removed,
    /// false otherwise
    virtual bool unput(char c) {
        // unsupported
        return false;
    }

    // from IInterface: This object is not reference counted.
    virtual Uint32 retain() const { return 1; }
    virtual Uint32 release() const { return 1; }
    virtual const mi::base::IInterface* get_interface(mi::base::Uuid const &interface_id) const {
        return nullptr;
    }
    virtual mi::base::IInterface* get_interface(mi::base::Uuid const &interface_id) {
        return nullptr;
    }
    virtual mi::base::Uuid get_iid() const { return IID(); }
};

static Assert_helper g_assert_helper;
static Fatal_helper  g_fatal_helper;
static DebugOutput   g_debug_output;

/// An allocator based on new/delete.
class Allocator : public mi::base::Interface_implement<mi::base::IAllocator>
{
public:
    void* malloc(mi::Size size) { return new char[size]; }
    void free(void* memory) { delete[] (char*) memory; }
};

/// MDL object deserializer that wraps a deserializer.
class MDL_deserializer : public mi::mdl::Base_deserializer
{
public:
    /// Read a byte.
    virtual Byte read() { Uint8 c; m_deserializer->read(&c); return c; }

    /// Reads a DB::Tag 32bit encoding.
    virtual unsigned read_db_tag() {
        DB::Tag t;
        m_deserializer->read(&t);
        return t.get_uint();
    }

    /// Constructor.
    MDL_deserializer(mi::mdl::IAllocator *alloc, SERIAL::Deserializer *deserializer)
    : mi::mdl::Base_deserializer(alloc)
    {
        m_deserializer = deserializer;
    }

private:
    SERIAL::Deserializer *m_deserializer;
};

/// MDL object serializer that wraps a serializer.
class MDL_serializer : public mi::mdl::Base_serializer
{
public:
    /// Write a byte.
    ///
    /// \param b  the byte to write
    virtual void write(Byte b) { m_serializer->write(Uint8(b)); }

    /// Write a DB::Tag.
    ///
    /// \param tag  the DB::Tag encoded as 32bit
    virtual void write_db_tag(unsigned tag) {
        DB::Tag t(tag);
        m_serializer->write(t);
    }

    /// Constructor.
    MDL_serializer(SERIAL::Serializer *serializer) { m_serializer = serializer; }

private:
    SERIAL::Serializer *m_serializer;
};

// Registration of the module.
static SYSTEM::Module_registration<Mdlc_module_impl> s_module(M_MDLC,"MDLC");

// Allow link time detection.
Module_registration_entry *Mdlc_module::get_instance()
{
    return s_module.init_module(s_module.get_name());
}

Mdlc_module_impl::Mdlc_module_impl()
  : m_mdl(0)
  , m_code_cache(0)
  , m_implicit_cast_enabled(true)
  , m_expose_names_of_let_expressions(true)
  , m_module_wait_queue(0)
{
}

bool Mdlc_module_impl::init()
{
    mi::mdl::iasserter   = &g_assert_helper;
    mi::mdl::ifatal      = &g_fatal_helper;
    mi::mdl::i_debug_log = &g_debug_output;

    m_allocator = new Allocator();

#ifdef USE_MDL_DEBUG_ALLOCATOR
    m_allocator = new mi::mdl::dbg::DebugMallocAllocator(m_allocator.get());
#endif

    // retrieve MDL parameters
    SYSTEM::Access_module<CONFIG::Config_module> config_module(/*deferred=*/false);
    CONFIG::Config_registry const                &registry = config_module->get_configuration();

    bool varying = true;
    registry.get_value("mdl_material_ior_is_varying", varying);

    m_mdl = mi::mdl::initialize(varying, m_allocator.get());
    if (!m_mdl)
        return false;

    m_mdl->install_search_path(MDL_search_path::create(m_allocator.get()));

    mi::mdl::Options                             &options = m_mdl->access_options();

    int opt_level = 0;
    if (registry.get_value("mdl_opt_level", opt_level)) {
        options.set_option(
            mi::mdl::MDL::option_opt_level, std::to_string(opt_level).c_str());
    }

    // neuray always runs in "relaxed" mode for compatibility with old releases
    options.set_option(mi::mdl::MDL::option_strict, "false");

    // disable "deprecated entity" warning by default
    MDL_ASSERT(mi::mdl::DEPRECATED_ENTITY == 275);
    options.set_option(mi::mdl::MDL::option_warn, "275=off");


    {
        // register nvidia/distilling_support.mdl
        bool res = m_mdl->add_builtin_module(
            "::nvidia::distilling_support",
            (char const *)mi::mdl::mdl_module_distilling_support,
            sizeof(mi::mdl::mdl_module_distilling_support),
            /*is_encoded=*/true,
            /*is_native=*/false);

        MDL_ASSERT(res && "Registration of ::nvidia::distilling_support failed");
        STLEXT::no_unused_variable_warning_please(res);
    }

    mi::mdl::Allocator_builder builder(m_allocator.get());

    // 8MB cache size by default
    size_t cache_size = 8*1024*1024, v;
    if (registry.get_value("mdl_target_code_cache_size", v)) {
        cache_size = v;
    }

    m_code_cache = builder.create<mi::mdl::Code_cache>(m_allocator.get(), cache_size);

    m_module_wait_queue = new MDL::Mdl_module_wait_queue();


    return true;
}

void Mdlc_module_impl::exit()
{

    if (m_mdl) {
        m_mdl->release();
        m_mdl = nullptr;
    }
    if (m_code_cache) {
        m_code_cache->release();
        m_code_cache = nullptr;
    }
    if (m_module_wait_queue) {
        delete m_module_wait_queue;
        m_module_wait_queue = nullptr;
    }

#ifdef USE_MDL_DEBUG_ALLOCATOR
    mi::mdl::dbg::DebugMallocAllocator* dbg_allocator = static_cast<mi::mdl::dbg::DebugMallocAllocator*>( m_allocator.get());
    m_allocator.reset();
    delete dbg_allocator;
#else
    m_allocator.reset();
#endif
}

mi::mdl::IMDL *Mdlc_module_impl::get_mdl() const
{
    m_mdl->retain();
    return m_mdl;
}

void Mdlc_module_impl::serialize_module(
    SERIAL::Serializer *serializer, const mi::mdl::IModule *module)
{
    MDL_serializer mdl_serializer(serializer);

    m_mdl->serialize_module(module, &mdl_serializer, /*include_dependencies=*/false);
}

const mi::mdl::IModule *Mdlc_module_impl::deserialize_module(SERIAL::Deserializer *deserializer)
{
    MDL_deserializer mdl_deserializer(m_allocator.get(), deserializer);

    return m_mdl->deserialize_module(&mdl_deserializer);
}

void Mdlc_module_impl::serialize_code_dag(
    SERIAL::Serializer *serializer, const mi::mdl::IGenerated_code_dag *code)
{
    MDL_serializer mdl_serializer(serializer);

    m_mdl->serialize_code_dag(code, &mdl_serializer);
}

const mi::mdl::IGenerated_code_dag *Mdlc_module_impl::deserialize_code_dag(
    SERIAL::Deserializer *deserializer)
{
    MDL_deserializer mdl_deserializer(m_allocator.get(), deserializer);
    return m_mdl->deserialize_code_dag(&mdl_deserializer);
}

void Mdlc_module_impl::serialize_material_instance(
    SERIAL::Serializer *serializer,
    const mi::mdl::IMaterial_instance *instance)
{
    MDL_serializer mdl_serializer(serializer);

    m_mdl->serialize_material_instance(instance, &mdl_serializer);
}

const mi::mdl::IMaterial_instance *Mdlc_module_impl::deserialize_material_instance(
    SERIAL::Deserializer *deserializer)
{
    MDL_deserializer mdl_deserializer(m_allocator.get(), deserializer);
    return m_mdl->deserialize_material_instance(&mdl_deserializer);
}

// Serializes the lambda function to the given serializer.
void Mdlc_module_impl::serialize_lambda_function(
    SERIAL::Serializer *serializer, const mi::mdl::ILambda_function *lambda)
{
    MDL_serializer mdl_serializer(serializer);
    m_mdl->serialize_lambda(lambda, &mdl_serializer);
}

// Deserializes the lambda function from the given deserializer.
mi::mdl::ILambda_function *Mdlc_module_impl::deserialize_lambda_function(
    SERIAL::Deserializer *deserializer)
{
    MDL_deserializer mdl_deserializer(m_allocator.get(), deserializer);
    return m_mdl->deserialize_lambda(&mdl_deserializer);
}

mi::mdl::ICode_cache *Mdlc_module_impl::get_code_cache() const
{
    if (m_code_cache)
        m_code_cache->retain();
    return m_code_cache;
}

void Mdlc_module_impl::set_implicit_cast_enabled(bool value)
{
    m_implicit_cast_enabled = value;
}

bool Mdlc_module_impl::get_implicit_cast_enabled() const
{
    return m_implicit_cast_enabled;
}

void Mdlc_module_impl::set_expose_names_of_let_expressions(bool value)
{
    m_expose_names_of_let_expressions = value;
}

bool Mdlc_module_impl::get_expose_names_of_let_expressions() const
{
    return m_expose_names_of_let_expressions;
}

MDL::Mdl_module_wait_queue* Mdlc_module_impl::get_module_wait_queue() const
{
    return m_module_wait_queue;
}

bool Mdlc_module_impl::is_valid_mdl_core_plugin(
    const char* type, const char* name, const char* filename)
{
    return false;
}

} // namespace MDLC

} // namespace MI

