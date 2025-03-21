/******************************************************************************
 * Copyright (c) 2023-2025, NVIDIA CORPORATION. All rights reserved.
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
// Generated by mdltlc

#include "pch.h"

#include "043_mixer_error.h"

#include <mi/mdl/mdl_distiller_plugin_api.h>
#include <mi/mdl/mdl_distiller_plugin_helper.h>

using namespace mi::mdl;

namespace MI {
namespace DIST {

// Return the strategy to be used with this rule set.
Rule_eval_strategy Mixer_error::get_strategy() const {
    return RULE_EVAL_BOTTOM_UP;
}

// Return the name of the rule set.
char const * Mixer_error::get_rule_set_name() const {
    return "Mixer_error";
}

// Return the number of imports of this rule set.
size_t Mixer_error::get_target_material_name_count() const {
    return 0;
}

// Return the name of the import at index i.
char const *Mixer_error::get_target_material_name(size_t i) const {
    return nullptr;
}

// Run the matcher.
DAG_node const* Mixer_error::matcher(
    IRule_matcher_event *event_handler,
    IDistiller_plugin_api &e,
    DAG_node const *node,
    const mi::mdl::Distiller_options *options,
    Rule_result_code &result_code) const
{
    auto match_rule2 = [&] (DAG_node const *node, IDistiller_plugin_api::Match_properties &node_props) -> const DAG_node * { return node; };

// 043_mixer_error.mdltl:8
//RUID 127964
    auto match_rule1 = [&] (DAG_node const *node1, IDistiller_plugin_api::Match_properties &node_props1) -> const DAG_node * {
        DAG_node const *v_matched_bsdf = node1;
        // continued match for bsdf_mix_2(_, bsdf(), _, _)
        DAG_node const *node3 = e.get_remapped_argument(node1, 1);
        IDistiller_plugin_api::Match_properties node_props3;
        e.get_match_properties(node3, node_props3); 
        // match for bsdf()
        if (node_props3.sema != IDefinition::DS_INVALID_REF_CONSTRUCTOR || node_props3.type_kind != IType::TK_BSDF) {
            return match_rule2(node1, node_props1);
        }
        DAG_DbgInfo root_dbg_info = node1->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 1);
        return v_matched_bsdf;
    };
    (void)match_rule1;

// 043_mixer_error.mdltl:7
//RUID 123784
    auto match_rule0 = [&] (DAG_node const *node0, IDistiller_plugin_api::Match_properties &node_props0) -> const DAG_node * {
        DAG_node const *v_matched_bsdf = node0;
        // match for bsdf_mix_2(_ [[ x ~ _xx ]], _, _, bsdf())
        if (node_props0.sema != IDefinition::DS_INTRINSIC_DF_NORMALIZED_MIX || node_props0.arity != 2
         || node_props0.type_kind != IType::TK_BSDF) {
            return match_rule2(node0, node_props0);
        }
        DAG_node const *node2 = e.get_remapped_argument(node0, 0);
        IDistiller_plugin_api::Match_properties node_props2;
        e.get_match_properties(node2, node_props2); 
        if (!e.attribute_exists(node2, "x")) {
            return match_rule1(node0, node_props0);
        }
        const DAG_node *node3 = e.get_attribute(node2, "x"); (void)node3;
        DAG_node const *v__xx = node3; (void)v__xx;
        DAG_node const *node6 = e.get_remapped_argument(node0, 3);
        IDistiller_plugin_api::Match_properties node_props6;
        e.get_match_properties(node6, node_props6); 
        // match for bsdf()
        if (node_props6.sema != IDefinition::DS_INVALID_REF_CONSTRUCTOR || node_props6.type_kind != IType::TK_BSDF) {
            return match_rule1(node0, node_props0);
        }
        DAG_DbgInfo root_dbg_info = node0->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 0);
        DAG_node const *node_result_1 = v_matched_bsdf;
        DAG_node const *node_result_1_x = e.create_int_constant(1);
        e.set_attribute(node_result_1, "x",node_result_1_x);
        return node_result_1;
    };
    (void)match_rule0;

    IDistiller_plugin_api::Match_properties node_props;
    e.get_match_properties(node, node_props);
    return match_rule0(node, node_props);

}

bool Mixer_error::postcond(
    IRule_matcher_event *event_handler,
    IDistiller_plugin_api &e,
    DAG_node const *root,
    const mi::mdl::Distiller_options *options) const
{
    (void)e; (void)root; // no unused variable warnings
    bool result = true;
    if (!result && event_handler != NULL)
        fire_postcondition_event(*event_handler);
    return result;
}

void Mixer_error::fire_match_event(
    mi::mdl::IRule_matcher_event &event_handler,
    std::size_t id)
{
    Rule_info const &ri = g_rule_info[id];
    event_handler.rule_match_event("Mixer_error", ri.ruid, ri.rname, ri.fname, ri.fline);
}

void Mixer_error::fire_postcondition_event(
mi::mdl::IRule_matcher_event &event_handler)
{
    event_handler.postcondition_failed("Mixer_error");
}

void Mixer_error::fire_debug_print(
    mi::mdl::IDistiller_plugin_api &plugin_api,
    mi::mdl::IRule_matcher_event &event_handler,
    std::size_t idx,
    char const *var_name,
    DAG_node const *value)
{
    Rule_info const &ri = g_rule_info[idx];
    event_handler.debug_print(plugin_api, "Mixer_error", ri.ruid, ri.rname, ri.fname,
        ri.fline, var_name, value);
}


// Rule info table.
Mixer_error::Rule_info const Mixer_error::g_rule_info[2] = {
    { 123784, "bsdf_mix_2", "043_mixer_error.mdltl", 7 },
    { 127964, "bsdf_mix_2", "043_mixer_error.mdltl", 8 }
};


} // DIST
} // MI
// End of generated code
