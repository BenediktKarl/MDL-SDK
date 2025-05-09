/******************************************************************************
 * Copyright (c) 2009-2025, NVIDIA CORPORATION. All rights reserved.
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

/** \file
 ** \brief
 **/

#include "pch.h"

#define MI_TEST_AUTO_SUITE_NAME "Regression Test Suite for prod/lib/neuray"
#define MI_TEST_IMPLEMENT_TEST_MAIN_INSTEAD_OF_MAIN

#include <base/system/test/i_test_auto_driver.h>
#include <base/system/test/i_test_auto_case.h>

#include <mi/base/handle.h>

#include <mi/neuraylib/factory.h>
#include <mi/neuraylib/idebug_configuration.h>
#include <mi/neuraylib/istring.h>
#include <mi/neuraylib/ineuray.h>

#include "test_shared.h"

void run_tests( mi::neuraylib::INeuray* neuray)
{
    MI_CHECK_EQUAL( 0, neuray->start());

    mi::base::Handle<mi::neuraylib::IDebug_configuration> debug_configuration(
        neuray->get_api_component<mi::neuraylib::IDebug_configuration>());

    mi::base::Handle<const mi::IString> result;

    MI_CHECK_EQUAL( 0, debug_configuration->set_option( "foo1=\"bar\""));
    result = debug_configuration->get_option( "foo1");
    MI_CHECK_EQUAL_CSTR( result->get_c_str(), "bar");

    MI_CHECK_EQUAL( 0, debug_configuration->set_option( "foo2=42"));
    result = debug_configuration->get_option( "foo2");
    MI_CHECK_EQUAL_CSTR( result->get_c_str(), "42");

    MI_CHECK_EQUAL( 0, debug_configuration->set_option( "foo3=0"));
    result = debug_configuration->get_option( "foo3");
    MI_CHECK_EQUAL_CSTR( result->get_c_str(), "0");

    MI_CHECK_EQUAL( 0, debug_configuration->set_option( "foo4=1"));
    result = debug_configuration->get_option( "foo4");
    MI_CHECK_EQUAL_CSTR( result->get_c_str(), "1");

    MI_CHECK_EQUAL( 0, debug_configuration->set_option( "foo5=0.5"));
    result = debug_configuration->get_option( "foo5");
    MI_CHECK_EQUAL_CSTR( result->get_c_str(), "0.5");

    MI_CHECK_EQUAL( -1, debug_configuration->set_option( "no_key_equals_value_syntax"));

    MI_CHECK_EQUAL( 0, neuray->shutdown());
}

MI_TEST_AUTO_FUNCTION( test_idebug_configuration )
{
    mi::base::Handle<mi::neuraylib::INeuray> neuray( load_and_get_ineuray());
    MI_CHECK( neuray);

    {
        run_tests( neuray.get());
        run_tests( neuray.get());
    }

    neuray = nullptr;
    MI_CHECK( unload());
}

MI_TEST_MAIN_CALLING_TEST_MAIN();

