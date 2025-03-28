/******************************************************************************
 * Copyright (c) 2022-2025, NVIDIA CORPORATION. All rights reserved.
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

#include "024_attr.h"

#include <mi/mdl/mdl_distiller_plugin_api.h>
#include <mi/mdl/mdl_distiller_plugin_helper.h>

using namespace mi::mdl;

namespace MI {
namespace DIST {

// Return the strategy to be used with this rule set.
Rule_eval_strategy Attr::get_strategy() const {
    return RULE_EVAL_BOTTOM_UP;
}

// Return the name of the rule set.
char const * Attr::get_rule_set_name() const {
    return "Attr";
}

// Return the number of imports of this rule set.
size_t Attr::get_target_material_name_count() const {
    return 0;
}

// Return the name of the import at index i.
char const *Attr::get_target_material_name(size_t i) const {
    return nullptr;
}

// Run the matcher.
DAG_node const* Attr::matcher(
    IRule_matcher_event *event_handler,
    IDistiller_plugin_api &e,
    DAG_node const *node,
    const mi::mdl::Distiller_options *options,
    Rule_result_code &result_code) const
{
    auto match_rule6 = [&] (DAG_node const *node, IDistiller_plugin_api::Match_properties &node_props) -> const DAG_node * { return node; };

// 024_attr.mdltl:93
//RUID 558659
    auto match_rule5 = [&] (DAG_node const *node5, IDistiller_plugin_api::Match_properties &node_props5) -> const DAG_node * {

        // match for material(thin_walled, material_surface(diffuse_reflection_bsdf(tint, r)), _backface, ior, volume, geometry, hair)
        if (node_props5.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props5.type_kind != IType::TK_STRUCT || node_props5.struct_id != IType_struct::SID_MATERIAL) {
            return match_rule6(node5, node_props5);
        }
        DAG_node const *node7 = e.get_compound_argument(node5, 0);
        DAG_node const *v_thin_walled = node7; (void)v_thin_walled;
        DAG_node const *node9 = e.get_compound_argument(node5, 1);
        IDistiller_plugin_api::Match_properties node_props9;
        e.get_match_properties(node9, node_props9); 
        // match for material_surface(diffuse_reflection_bsdf(tint, r))
        if (node_props9.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props9.type_kind != IType::TK_STRUCT || node_props9.struct_id != IType_struct::SID_MATERIAL_SURFACE) {
            return match_rule6(node5, node_props5);
        }
        DAG_node const *node10 = e.get_compound_argument(node9, 0);
        IDistiller_plugin_api::Match_properties node_props10;
        e.get_match_properties(node10, node_props10); 
        // match for diffuse_reflection_bsdf(tint, r)
        if (node_props10.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule6(node5, node_props5);
        }
        DAG_node const *node11 = e.get_compound_argument(node10, 0);
        DAG_node const *v_tint = node11; (void)v_tint;
        DAG_node const *node13 = e.get_compound_argument(node10, 1);
        DAG_node const *v_r = node13; (void)v_r;
        DAG_node const *node17 = e.get_compound_argument(node5, 2);
        DAG_node const *v__backface = node17; (void)v__backface;
        DAG_node const *node19 = e.get_compound_argument(node5, 3);
        DAG_node const *v_ior = node19; (void)v_ior;
        DAG_node const *node21 = e.get_compound_argument(node5, 4);
        DAG_node const *v_volume = node21; (void)v_volume;
        DAG_node const *node23 = e.get_compound_argument(node5, 5);
        DAG_node const *v_geometry = node23; (void)v_geometry;
        DAG_node const *node25 = e.get_compound_argument(node5, 6);
        DAG_node const *v_hair = node25; (void)v_hair;
        if (!e.attribute_exists(node5, "_bsdf1")) {
            return match_rule6(node5, node_props5);
        }
        const DAG_node *node27 = e.get_attribute(node5, "_bsdf1"); (void)node27;
        // match for diffuse_reflection_bsdf(t1, r1)
        if (node_props27.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule6(node5, node_props5);
        }
        DAG_node const *node29 = e.get_compound_argument(node27, 0);
        DAG_node const *v_t1 = node29; (void)v_t1;
        DAG_node const *node31 = e.get_compound_argument(node27, 1);
        DAG_node const *v_r1 = node31; (void)v_r1;
        DAG_DbgInfo root_dbg_info = node5->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 5);
        DAG_node const *node_result_8 = e.create_call("material(bool,material_surface,material_surface,color,material_volume,material_geometry,hair_bsdf)",
                IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<7>::mk_args(e, m_node_types,
                    material, v_thin_walled, e.create_call("material_surface(bsdf,material_emission)",
                        IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<2>::mk_args(
                            e, m_node_types, material_surface, e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                                IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF,
                                Args_wrapper<4>::mk_args(e, m_node_types, diffuse_reflection_bsdf,
                                    v_t1, v_r1).args, 4, e.get_type_factory()->create_bsdf(), root_dbg_info)).args,
                        2, e.get_type_factory()->get_predefined_struct(IType_struct::SID_MATERIAL_SURFACE), root_dbg_info),
                    e.create_call("material_surface(bsdf,material_emission)", IDefinition::DS_ELEM_CONSTRUCTOR,
                        Args_wrapper<2>::mk_args(e, m_node_types, material_surface,
                            e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                                IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF,
                                Args_wrapper<4>::mk_args(e, m_node_types, diffuse_reflection_bsdf,
                                    v_tint, v_r).args, 4, e.get_type_factory()->create_bsdf(), root_dbg_info)).args,
                        2, e.get_type_factory()->get_predefined_struct(IType_struct::SID_MATERIAL_SURFACE), root_dbg_info),
                    v_ior, v_volume, v_geometry, v_hair).args, 7, e.get_type_factory()->
                get_predefined_struct(IType_struct::SID_MATERIAL), root_dbg_info);
        DAG_node const *node_result_8_is_thin_walled = v_thin_walled;
        e.set_attribute(node_result_8, "is_thin_walled",node_result_8_is_thin_walled);
        DAG_node const *node_result_8_global_tint = e.create_color_constant(2,2,2);
        e.set_attribute(node_result_8, "global_tint",node_result_8_global_tint);
        return node_result_8;
    };
    (void)match_rule5;

// 024_attr.mdltl:74
//RUID 968546
    auto match_rule4 = [&] (DAG_node const *node4, IDistiller_plugin_api::Match_properties &node_props4) -> const DAG_node * {

        // match for material(thin_walled, material_surface(diffuse_reflection_bsdf(_, r) [[ surface_value ~ _ ]]), backface, ior, volume, geometry, hair2)
        if (node_props4.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props4.type_kind != IType::TK_STRUCT || node_props4.struct_id != IType_struct::SID_MATERIAL) {
            return match_rule5(node4, node_props4);
        }
        DAG_node const *node6 = e.get_compound_argument(node4, 0);
        DAG_node const *v_thin_walled = node6; (void)v_thin_walled;
        DAG_node const *node8 = e.get_compound_argument(node4, 1);
        IDistiller_plugin_api::Match_properties node_props8;
        e.get_match_properties(node8, node_props8); 
        // match for material_surface(diffuse_reflection_bsdf(_, r) [[ surface_value ~ _ ]])
        if (node_props8.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props8.type_kind != IType::TK_STRUCT || node_props8.struct_id != IType_struct::SID_MATERIAL_SURFACE) {
            return match_rule5(node4, node_props4);
        }
        DAG_node const *node9 = e.get_compound_argument(node8, 0);
        IDistiller_plugin_api::Match_properties node_props9;
        e.get_match_properties(node9, node_props9); 
        // match for diffuse_reflection_bsdf(_, r)
        if (node_props9.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule5(node4, node_props4);
        }
        DAG_node const *node10 = e.get_compound_argument(node9, 1);
        DAG_node const *v_r = node10; (void)v_r;
        if (!e.attribute_exists(node9, "surface_value")) {
            return match_rule5(node4, node_props4);
        }
        const DAG_node *node12 = e.get_attribute(node9, "surface_value"); (void)node12;
        DAG_node const *node16 = e.get_compound_argument(node4, 2);
        DAG_node const *v_backface = node16; (void)v_backface;
        DAG_node const *node18 = e.get_compound_argument(node4, 3);
        DAG_node const *v_ior = node18; (void)v_ior;
        DAG_node const *node20 = e.get_compound_argument(node4, 4);
        DAG_node const *v_volume = node20; (void)v_volume;
        DAG_node const *node22 = e.get_compound_argument(node4, 5);
        DAG_node const *v_geometry = node22; (void)v_geometry;
        DAG_node const *node24 = e.get_compound_argument(node4, 6);
        DAG_node const *v_hair2 = node24; (void)v_hair2;
        if (!e.attribute_exists(node4, "some_color")) {
            return match_rule5(node4, node_props4);
        }
        DAG_DbgInfo root_dbg_info = node4->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 4);
        DAG_node const *node_result_6 = e.create_call("material(bool,material_surface,material_surface,color,material_volume,material_geometry,hair_bsdf)",
                IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<7>::mk_args(e, m_node_types,
                    material, v_thin_walled, e.create_call("material_surface(bsdf,material_emission)",
                        IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<2>::mk_args(
                            e, m_node_types, material_surface, e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                                IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF,
                                Args_wrapper<4>::mk_args(e, m_node_types, diffuse_reflection_bsdf,
                                    v_some_color, v_r).args, 4, e.get_type_factory()->create_bsdf(), root_dbg_info)).args,
                        2, e.get_type_factory()->get_predefined_struct(IType_struct::SID_MATERIAL_SURFACE), root_dbg_info),
                    v_backface, v_ior, v_volume, v_geometry, v_hair2).args, 7, e.get_type_factory()->
                get_predefined_struct(IType_struct::SID_MATERIAL), root_dbg_info);
        DAG_node const *node_result_6_global_value = v_surface_value;
        e.set_attribute(node_result_6, "global_value",node_result_6_global_value);
        return node_result_6;
    };
    (void)match_rule4;

// 024_attr.mdltl:51
//RUID 512932
    auto match_rule3 = [&] (DAG_node const *node3, IDistiller_plugin_api::Match_properties &node_props3) -> const DAG_node * {

        // match for material(_, material_surface(diffuse_reflection_bsdf(_, r)), backface, ior, volume, geometry, hair)
        if (node_props3.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props3.type_kind != IType::TK_STRUCT || node_props3.struct_id != IType_struct::SID_MATERIAL) {
            return match_rule4(node3, node_props3);
        }
        DAG_node const *node5 = e.get_compound_argument(node3, 1);
        IDistiller_plugin_api::Match_properties node_props5;
        e.get_match_properties(node5, node_props5); 
        // match for material_surface(diffuse_reflection_bsdf(_, r))
        if (node_props5.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props5.type_kind != IType::TK_STRUCT || node_props5.struct_id != IType_struct::SID_MATERIAL_SURFACE) {
            return match_rule4(node3, node_props3);
        }
        DAG_node const *node6 = e.get_compound_argument(node5, 0);
        IDistiller_plugin_api::Match_properties node_props6;
        e.get_match_properties(node6, node_props6); 
        // match for diffuse_reflection_bsdf(_, r)
        if (node_props6.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule4(node3, node_props3);
        }
        DAG_node const *node7 = e.get_compound_argument(node6, 1);
        DAG_node const *v_r = node7; (void)v_r;
        DAG_node const *node11 = e.get_compound_argument(node3, 2);
        DAG_node const *v_backface = node11; (void)v_backface;
        DAG_node const *node13 = e.get_compound_argument(node3, 3);
        DAG_node const *v_ior = node13; (void)v_ior;
        DAG_node const *node15 = e.get_compound_argument(node3, 4);
        DAG_node const *v_volume = node15; (void)v_volume;
        DAG_node const *node17 = e.get_compound_argument(node3, 5);
        DAG_node const *v_geometry = node17; (void)v_geometry;
        DAG_node const *node19 = e.get_compound_argument(node3, 6);
        DAG_node const *v_hair = node19; (void)v_hair;
        if (!e.attribute_exists(node3, "is_thin_walled")) {
            return match_rule4(node3, node_props3);
        }
        if (!e.attribute_exists(node3, "_some_color")) {
            return match_rule4(node3, node_props3);
        }
        const DAG_node *node21 = e.get_attribute(node3, "_some_color"); (void)node21;
        DAG_node const *v_c = node21; (void)v_c;
        DAG_DbgInfo root_dbg_info = node3->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 3);
        return e.create_call("material(bool,material_surface,material_surface,color,material_volume,material_geometry,hair_bsdf)",
            IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<7>::mk_args(e, m_node_types,
                material, v_is_thin_walled, e.create_call("material_surface(bsdf,material_emission)",
                    IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<2>::mk_args(e, m_node_types,
                        material_surface, e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                            IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF,
                            Args_wrapper<4>::mk_args(e, m_node_types, diffuse_reflection_bsdf,
                                v_c, v_r).args, 4, e.get_type_factory()->create_bsdf(), root_dbg_info)).args,
                    2, e.get_type_factory()->get_predefined_struct(IType_struct::SID_MATERIAL_SURFACE), root_dbg_info),
                v_backface, v_ior, v_volume, v_geometry, v_hair).args, 7, e.get_type_factory()->
            get_predefined_struct(IType_struct::SID_MATERIAL), root_dbg_info);
    };
    (void)match_rule3;

// 024_attr.mdltl:33
//RUID 158046
    auto match_rule2 = [&] (DAG_node const *node2, IDistiller_plugin_api::Match_properties &node_props2) -> const DAG_node * {

        // match for material(thin_walled, material_surface(diffuse_reflection_bsdf(_, r)), backface, ior, volume, geometry, hair)
        if (node_props2.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props2.type_kind != IType::TK_STRUCT || node_props2.struct_id != IType_struct::SID_MATERIAL) {
            return match_rule3(node2, node_props2);
        }
        DAG_node const *node4 = e.get_compound_argument(node2, 0);
        DAG_node const *v_thin_walled = node4; (void)v_thin_walled;
        DAG_node const *node6 = e.get_compound_argument(node2, 1);
        IDistiller_plugin_api::Match_properties node_props6;
        e.get_match_properties(node6, node_props6); 
        // match for material_surface(diffuse_reflection_bsdf(_, r))
        if (node_props6.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props6.type_kind != IType::TK_STRUCT || node_props6.struct_id != IType_struct::SID_MATERIAL_SURFACE) {
            return match_rule3(node2, node_props2);
        }
        DAG_node const *node7 = e.get_compound_argument(node6, 0);
        IDistiller_plugin_api::Match_properties node_props7;
        e.get_match_properties(node7, node_props7); 
        // match for diffuse_reflection_bsdf(_, r)
        if (node_props7.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule3(node2, node_props2);
        }
        DAG_node const *node8 = e.get_compound_argument(node7, 1);
        DAG_node const *v_r = node8; (void)v_r;
        DAG_node const *node12 = e.get_compound_argument(node2, 2);
        DAG_node const *v_backface = node12; (void)v_backface;
        DAG_node const *node14 = e.get_compound_argument(node2, 3);
        DAG_node const *v_ior = node14; (void)v_ior;
        DAG_node const *node16 = e.get_compound_argument(node2, 4);
        DAG_node const *v_volume = node16; (void)v_volume;
        DAG_node const *node18 = e.get_compound_argument(node2, 5);
        DAG_node const *v_geometry = node18; (void)v_geometry;
        DAG_node const *node20 = e.get_compound_argument(node2, 6);
        DAG_node const *v_hair = node20; (void)v_hair;
        if (!e.attribute_exists(node2, "some_color")) {
            return match_rule3(node2, node_props2);
        }
        DAG_DbgInfo root_dbg_info = node2->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 2);
        return e.create_call("material(bool,material_surface,material_surface,color,material_volume,material_geometry,hair_bsdf)",
            IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<7>::mk_args(e, m_node_types,
                material, v_thin_walled, e.create_call("material_surface(bsdf,material_emission)",
                    IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<2>::mk_args(e, m_node_types,
                        material_surface, e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                            IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF,
                            Args_wrapper<4>::mk_args(e, m_node_types, diffuse_reflection_bsdf,
                                v_some_color, v_r).args, 4, e.get_type_factory()->create_bsdf(), root_dbg_info)).args,
                    2, e.get_type_factory()->get_predefined_struct(IType_struct::SID_MATERIAL_SURFACE), root_dbg_info),
                v_backface, v_ior, v_volume, v_geometry, v_hair).args, 7, e.get_type_factory()->
            get_predefined_struct(IType_struct::SID_MATERIAL), root_dbg_info);
    };
    (void)match_rule2;

// 024_attr.mdltl:14
//RUID 611548
    auto match_rule1 = [&] (DAG_node const *node1, IDistiller_plugin_api::Match_properties &node_props1) -> const DAG_node * {

        // match for material(thin_walled, surface, backface, ior, volume, geometry, hair)
        if (node_props1.sema != IDefinition::DS_ELEM_CONSTRUCTOR || node_props1.type_kind != IType::TK_STRUCT || node_props1.struct_id != IType_struct::SID_MATERIAL) {
            return match_rule2(node1, node_props1);
        }
        DAG_node const *node3 = e.get_compound_argument(node1, 0);
        DAG_node const *v_thin_walled = node3; (void)v_thin_walled;
        DAG_node const *node5 = e.get_compound_argument(node1, 1);
        DAG_node const *v_surface = node5; (void)v_surface;
        DAG_node const *node7 = e.get_compound_argument(node1, 2);
        DAG_node const *v_backface = node7; (void)v_backface;
        DAG_node const *node9 = e.get_compound_argument(node1, 3);
        DAG_node const *v_ior = node9; (void)v_ior;
        DAG_node const *node11 = e.get_compound_argument(node1, 4);
        DAG_node const *v_volume = node11; (void)v_volume;
        DAG_node const *node13 = e.get_compound_argument(node1, 5);
        DAG_node const *v_geometry = node13; (void)v_geometry;
        DAG_node const *node15 = e.get_compound_argument(node1, 6);
        DAG_node const *v_hair = node15; (void)v_hair;
        DAG_DbgInfo root_dbg_info = node1->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 1);
        DAG_node const *node_result_0 = e.create_call("material(bool,material_surface,material_surface,color,material_volume,material_geometry,hair_bsdf)",
                IDefinition::DS_ELEM_CONSTRUCTOR, Args_wrapper<7>::mk_args(e, m_node_types,
                    material, v_thin_walled, v_surface, v_backface, v_ior, v_volume,
                    v_geometry, v_hair).args, 7, e.get_type_factory()->get_predefined_struct(
                IType_struct::SID_MATERIAL), root_dbg_info);
        DAG_node const *node_result_0_is_thin_walled = v_thin_walled;
        e.set_attribute(node_result_0, "is_thin_walled",node_result_0_is_thin_walled);
        return node_result_0;
    };
    (void)match_rule1;

// 024_attr.mdltl:62
//RUID 753537
    auto match_rule0 = [&] (DAG_node const *node0, IDistiller_plugin_api::Match_properties &node_props0) -> const DAG_node * {

        // match for diffuse_reflection_bsdf(c, r)
        if (node_props0.sema != IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF) {
            return match_rule1(node0, node_props0);
        }
        DAG_node const *node2 = e.get_compound_argument(node0, 0);
        DAG_node const *v_c = node2; (void)v_c;
        DAG_node const *node4 = e.get_compound_argument(node0, 1);
        DAG_node const *v_r = node4; (void)v_r;
        DAG_DbgInfo root_dbg_info = node0->get_dbg_info();
        (void) root_dbg_info;

        if (event_handler != nullptr)
            fire_match_event(*event_handler, 0);
        DAG_node const *node_result_3 = e.create_call("::df::diffuse_reflection_bsdf(color,float,color,string)",
                IDefinition::DS_INTRINSIC_DF_DIFFUSE_REFLECTION_BSDF, Args_wrapper<4>::mk_args(
                    e, m_node_types, diffuse_reflection_bsdf, v_c, v_r).args, 4,
                e.get_type_factory()->create_bsdf(), root_dbg_info);
        DAG_node const *node_result_3_surface_value = e.create_color_constant(1,1,1);
        e.set_attribute(node_result_3, "surface_value",node_result_3_surface_value);
        return node_result_3;
    };
    (void)match_rule0;

    IDistiller_plugin_api::Match_properties node_props;
    e.get_match_properties(node, node_props);
    return match_rule0(node, node_props);

}

bool Attr::postcond(
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

void Attr::fire_match_event(
    mi::mdl::IRule_matcher_event &event_handler,
    std::size_t id)
{
    Rule_info const &ri = g_rule_info[id];
    event_handler.rule_match_event("Attr", ri.ruid, ri.rname, ri.fname, ri.fline);
}

void Attr::fire_postcondition_event(
mi::mdl::IRule_matcher_event &event_handler)
{
    event_handler.postcondition_failed("Attr");
}

void Attr::fire_debug_print(
    mi::mdl::IDistiller_plugin_api &plugin_api,
    mi::mdl::IRule_matcher_event &event_handler,
    std::size_t idx,
    char const *var_name,
    DAG_node const *value)
{
    Rule_info const &ri = g_rule_info[idx];
    event_handler.debug_print(plugin_api, "Attr", ri.ruid, ri.rname, ri.fname, ri.fline,
        var_name, value);
}


// Rule info table.
Attr::Rule_info const Attr::g_rule_info[6] = {
    { 753537, "diffuse_reflection_bsdf", "024_attr.mdltl", 62 },
    { 611548, "material", "024_attr.mdltl", 14 },
    { 158046, "material", "024_attr.mdltl", 33 },
    { 512932, "material", "024_attr.mdltl", 51 },
    { 968546, "material", "024_attr.mdltl", 74 },
    { 558659, "material", "024_attr.mdltl", 93 }
};


} // DIST
} // MI
// End of generated code
