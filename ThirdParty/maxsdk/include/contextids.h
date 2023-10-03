//
// Copyright 2010 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//===============================================================================================
//
// 3ds Max context-sensitive help items.
//
// The idh values in this file are most commonly used as arguments to the SetWindowContextHelpId(HWND, idh) function.
// That function associates the HWND and idh value it receives so that if F1 is pressed while the HWND is active, then the idh value is used to open the help at a relevant page.
// For example, 3ds Max's viewports call SetWindowContextHelpId() with their HWNDs and the idh_view_ports value. Because of this, when users click on one of the viewports and press F1, the help is opened at the "Configuring Viewports" page.
//
// The idh values in this file can also be used as arguments to the MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic() function in order to display a specific 3ds Max help topic.
// For example, a call to MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic(idh_file_save_as) causes the help to be opened at the "Save As" page.
//
// Internal note: Any change to this file will require HelpsysContextIDtoString.cpp to be updated too.
//

#pragma once

#define HOME_PAGE                              1000 // 3ds Max Online Help (contents page)
#define what_s_new_topic                       1001 // Help what's new

// Menubar items
#define idh_menubar                            3000 // Menubar

// Edit menu items
#define idh_edit_clone                         3350 // Clone
#define idh_edit_edit_named_selections         3412
#define idh_edit_track_view                    3459
#define idh_edit_schematic_view                3460
#define idh_spacing_tool                       3471

// Tools Menu
#define idh_tools_selection_floater            3479 // Selection floater

// MAXScript menu
#define idh_maxscript_debug_dialog             3486 // Open MAXScript debugger dialog

// Rendering menu items
#define idh_rendering_ireshade                 3815 // Interactive reshading
#define idh_radiosity                          3841 // Radiosity panel

// Schematic view menu items
#define idh_schematic_view_open_schematic_view 3910

#define idh_view_ports                         5600 // Viewports

// Status line and prompt area items
#define idh_status_panel                       5700
#define idh_sp_time_slider                     5710 // Time slider
#define idh_sp_status_line                     5730 // Status line
#define idh_sp_selection_lock                  5735
#define idh_sp_degrade_override                5745
#define idh_sp_oneclick                        5775
#define idh_sp_tti_mode                        5790 // Transform type-in absolute-relative toggle

// Time panel
#define idh_time_panel                         5800
#define idh_tp_animate                         5810
#define idh_tp_goto_start                      5820
#define idh_tp_previous                        5830
#define idh_tp_play                            5840
#define idh_tp_next                            5850
#define idh_tp_goto_end                        5860
#define idh_tp_key_mode                        5870
#define idh_tp_time_config                     5890
#define idh_tp_dflt_tangent                    5900

// View control panel (lower right)
#define idh_view_control                       8200
#define idh_vc_zoom                            8201
#define idh_vc_zoom_all                        8202
#define idh_vc_zoom_extents                    8203
#define idh_vc_zoom_ext_all                    8205
#define idh_vc_zoom_region                     8207
#define idh_vc_field_of_view                   8208
#define idh_vc_pan                             8209
#define idh_vc_arc_rotate                      8210
#define idh_vc_min_max_toggle                  8212
// Camera specific
#define idh_vc_dolly                           8213
#define idh_vc_perspective                     8214
#define idh_vc_roll                            8215
#define idh_vc_truck                           8216
#define idh_vc_pan_cam                         8218
// Spotlight specific
#define idh_vc_hotspot                         8219
#define idh_vc_falloff                         8220

// Command panel items
#define idh_command_panel                      6000 // Command panel

// Create items
#define idh_cp_create                          6100 // Create

// Modify items
#define idh_cp_modify                          6300 // Modify

// Hierarchy items
#define idh_cp_hierarchy                       6500 // Hierarchy

// Motion items
#define idh_cp_motion                          6600 // Motion

// Display items
#define idh_cp_display                         6700 // Display

// Dialog Box: Viewport Configuration
#define idh_dialog_viewport_configuration      7000 // Main box
// Render page
#define idh_vpconfig_render_page               7001 // Render page
// Layout page
#define idh_vpconfig_layout_page               7010
// Safeframe page
#define idh_vpconfig_safeframe_page            7020
// Degradation page
#define idh_vpconfig_degrade_page              7030 // Degradation page
// Statistics page
#define idh_vpconfig_statistics_page           7045

// Dialog Box: Path Configuration
#define idh_dialog_configure_paths             7050 // Main box
// General page
#define idh_paths_general_page                 7051 // General page
#define idh_paths_plugin_page                  7061 // Plugin page
#define idh_paths_bitmaps_page                 7062 // Bitmaps page

// Dialog Box: Preference Settings (Part 1)
#define idh_dialog_preference_settings         8000 // Main box
// General page
#define idh_pref_general_page                  8010
// Rendering page
#define idh_pref_rendering_page                8018
// IK page
#define idh_pref_ik_page                       8029
// Animation page
#define idh_pref_animation_page                8034
// Keyboard page
#define idh_pref_keyboard_page                 8042
// Files page
#define idh_pref_files_page                    8048
// Gamma page
#define idh_pref_gamma_page                    8054
// Radiosity page
#define idh_pref_radiosity_page                8058
// Viewports page
#define idh_pref_viewports_page                8060
// Interaction mode page
#define idh_pref_interaction_page              8064
// MAXScript page
#define idh_pref_maxscript_page                8066
// Containers page
#define idh_pref_containers_page               8067
// Gizmos page
#define idh_pref_tmgizmos_page                 8068
// Help page
#define idh_pref_help_page                     8069

// Dialog Box: Select by Name
#define idh_dialog_select_by_name              8070

// Scene Info dialog box
#define idh_dialog_scene_info                  8080

// Array dialog box
#define idh_dialog_array                       8092

// Preview dialog box
#define idh_dialog_preview                     8100

// Dialog Box: Preference Settings (Part 2)
// Security page
#define idh_pref_security_page                 8300

// Dialogs with no "control level" granularity
#define idh_dialog_about                       9000
#define idh_dialog_select_camera               9010
#define idh_dialog_collapse_warning            9020 // For both collapse all and collapse range
#define idh_dialog_file_reset                  9050
#define idh_dialog_object_prop                 9060

#define idh_dialog_object_general              9061 // General object properties
#define idh_dialog_object_radiosity            9062 // Radiosity object properties
#define idh_dialog_object_user                 9065 // User defined properties

#define idh_dialog_grid_snap_settings          9070
#define idh_dialog_group                       9080
#define idh_dialog_import_name_conflict        9090
#define idh_dialog_missing_dlls                9100
#define idh_dialog_merge                       9110
#define idh_dialog_merge_matlib                9112
#define idh_dialog_merge_matlib_dup            9113
#define idh_dialog_replace                     9115
#define idh_dialog_choose_view                 9130
#define idh_dialog_mtl_preview                 9140
#define idh_dialog_object_color                9150
#define idh_dialog_clone_options               9160
#define idh_dialog_plugin_help                 9170
#define idh_dialog_plugin_info                 9180 // Subdialog of scene info
#define idh_dialog_preview_cancel              9190
#define idh_dialog_rescale_time                9200
#define idh_dialog_time_config                 9210
#define idh_dialog_topo_warning                9220 // Topology warning
#define idh_dialog_units_setup                 9230
#define idh_dialog_viewport_background         9240
#define idh_dialog_customize_ui                9245
#define idh_dialog_createbrushpreset           9246

// UI customization tabbed dialog
#define idh_customize_toolbars                 9255 // Toolbar page
#define idh_customize_menu                     9260 // Main menus page
#define idh_customize_quadmenus                9265 // Quad menus page
#define idh_customize_colors                   9270 // Colors page
#define idh_customize_mouse                    9271 // Mouse page

// FileLink dialogs
#define idh_filelink_mgr_main                  9300
#define idh_filelink_mgr_attach_page           9301
#define idh_filelink_mgr_files_page            9302
#define idh_filelink_mgr_presets_page          9303
#define idh_filelink_dwg_settings              9310
#define idh_filelink_dwg_settings_basic        9311
#define idh_filelink_dwg_settings_adv          9312

#define idh_dialog_mtl_edit                    10001
#define idh_dialog_medit_opt                   10002
#define idh_dialog_put_medit                   10003
#define idh_dialog_mtlbrows                    10004
#define idh_dialog_render                      10005
#define idh_dialog_excl_list                   10006
#define idh_dialog_update_mtls                 10007
#define idh_dialog_sme                         10008 // Schematic material editor

// Developer IDs
#define idh_mcg                                10101
#define idh_maxscript                          10102
#define idh_python                             10103

// Desktop connected services IDs
#define idh_sharedviews                        10150
#define idh_dc_unwrap                          10155

#define idh_notify_main_ui                     10600 // Notify.exe main UI
#define idh_bmm_vfb                            10601 // BMM Virtual Frame Buffer Window

#define idh_vp_exec_dialogue                   10620 // Video Post - Execute Dialogue
#define idh_vp_presets_dialogue                10621 // Video Post - Render Presets Dialogue (Predefined Output Resolutions)
#define idh_vp_edit_scene                      10622 // Video Post - Edit Scene Dialogue
#define idh_vp_edit_filter                     10623 // Video Post - Edit Filter Dialogue
#define idh_vp_edit_layer                      10624 // Video Post - Edit Layer Dialogue
#define idh_vp_edit_image_input                10625 // Video Post - Edit Image Input Dialogue
#define idh_vp_edit_image_output               10626 // Video Post - Edit Image Output Dialogue
#define idh_vp_edit_external                   10627 // Video Post - Edit External Dialogue
#define idh_vp_edit_loop                       10628 // Video Post - Edit Loop Dialogue
#define idh_vp_missing_maps                    10629 // Video Post - Missing Maps Dialogue

#define idh_dialog_trackview_ort               11000
#define idh_dialog_trackview_timepaste         11010
#define idh_dialog_trackview_trackpaste        11020
#define idh_dialog_trackview_reducekeys        11030
#define idh_dialog_trackview_filters           11040
#define idh_dialog_trackview_pick              11050
#define idh_dialog_trackview_choosecontrol     11060

#define idh_dialog_timeslide_createkey         11070

#define idh_dialog_pref_control_defaults       11080
#define idh_dialog_pref_select_class           11090

#define idh_dialog_snapshot                    11100
#define idh_dialog_align                       11110
#define idh_dialog_mirror                      11120
#define idh_dialog_normal_align                11130
#define idh_dialog_view_align                  11135

#define idh_dialog_render_params               11140
#define idh_dialog_render_presetsize           11160

#define idh_dialog_atmos                       11170
#define idh_dialog_atmos_create                11180

#define idh_dialog_medit_browser               11190
#define idh_dialog_medit_puttolib              11200

#define idh_dialog_buttonsets                  11210

#define idh_dialog_xform_typein                11220

#define idh_dialog_effect                      11230
#define idh_dialog_effect_create               11240

#define idh_dialog_obj_xref                    11250 // Object xref dialog
#define idh_dialog_scene_xref                  11260 // Scene xref dialog

#define idh_colorclip_floater                  11270 // Color clip board utility

// Layer property dialog
#define idh_dialog_layer_general               11540
#define idh_dialog_layer_radiosity             11560
#define idh_dialog_layer_prop                  11580

// Schematic view ids
#define idh_dialog_schematicview_filters       11600

#define idh_dialog_file_properties             11700
#define idh_ramplayer_window                   11710

// DWG/DXF import and export dialog ids
#define idh_dwgexp_options                     11750 // DWG/DXF Export options
#define idh_dwgimp_options                     11760 // DWG/DXF Import options (tabbed)
#define idh_dwgimp_geometry_page               11761
#define idh_dwgimp_layers_page                 11762
#define idh_dwgimp_rendering_page              11763

// Prs dialog ids
#define idh_3dsexp_export                      11800 // 3ds Export Dialog

#define idh_3dsimp_import                      11820 // 3ds Import Dialog

#define idh_system_unit_dialog                   11825 // "System Unit Setup" dialog (idh_pref_system_units is in this dialog but defined above)

#define idh_vrmlexp_export                     11854 // VRML97 Export Dialog

#define idh_vrmlimp_import                     11890 // VRML Import Dialog

#define idh_max2obj_export                     11892 // Guruware .obj Export Dialog

#define idh_obj2max_import                     11894 // Guruware .obj Import Dialog

#define idh_nurbs_editcrvonsrf                 12001 // Edit Curve On Surface Dialog
#define idh_nurbs_selectbyname                 12002 // Select Sub-Objects Dialog
#define idh_nurbs_edittexturesurf              12003 // Edit Texture Surface Dialog
#define idh_nurbs_makeloft                     12004 // Make Loft Dialog
#define idh_nurbs_selectbyid                   12005 // Select by material ID dialog
#define idh_nurbs_convertsurface               12006 // Convert Surface dialog
#define idh_nurbs_convertcurve                 12007 // Convert Curve dialog
#define idh_nurbs_detach                       12008 // Detach dialog
#define idh_nurbs_reparameterize               12009 // Reparameterize dialog
#define idh_nurbs_makepointsurf                12010 // Make point surface dialog
#define idh_nurbs_rebuildcvsurf                12011 // Rebuild CV Surface dialog
#define idh_nurbs_rebuildtexturesurf           12012 // Rebuild texture surface dialog
#define idh_nurbs_toolbox                      12013 // Toolbox dialog
#define idh_nurbs_advancedsurfapprox           12014 // Advanced surface approximation dialog
#define idh_nurbs_rebuildcvcurv                12015 // Rebuild CV Curv dialog

#define idh_parameter_wiring_dialog            12500 // Parameter wiring dialog

#define idh_dialog_assembly_create             12520 // Create assembly dialog. No control level granularity

#define idh_radiosity_lighting_analysis        12540 // Radiosity Lighting Analysis dialog

// Help ids for restrictions that apply for actively linked objects
#define idh_linkedobj_no_unlink_mode           12563

// Help id for fix ambient utility
#define idh_fix_ambient                        12600

// Help id of the Clone & Align (Batch Insert) tool
#define idh_clone_align_tool                   12605

// Help ID for the DWF Exporter.
#define idh_dwf_export                         12610

// Help ID for Game Exporter.
#define idh_game_exporter                      12800

// Help ID for glTF Exporter
#define idh_gltf_exporter                      12850

// Help ID for FBX.
#define idh_fbx_help                           12900

// Unwrap Dialog IDs
#define idh_unwrap_edit                        13000
#define idh_unwrap_options                     13010
#define idh_unwrap_splinemap                   13020
#define idh_unwrap_peltmap                     13030
#define idh_unwrap_stitch                      13040
#define idh_unwrap_sketch                      13050
#define idh_unwrap_relax                       13060
#define idh_unwrap_renderuvw                   13070
#define idh_unwrap_flattenmap                  13080
#define idh_unwrap_normalmap                   13090
#define idh_unwrap_unfoldmap                   13100

// Lighting Analysis Assistant Dialog IDs
#define idh_dialog_lighting_analysis           14000 // Main box
#define idh_lightanal_lighting_page            14001 // Lighting page
#define idh_lightanal_materials_page           14001 // Materials page
#define idh_lightanal_analysis_page            14001 // Analysis page
#define idh_lightanal_general_page             14001 // General page

#define idh_viewcube_help                      15000
#define idh_steerwheel_help                    15001

#define id_massfx_modrb_prop                   15014
#define id_massfx_modrb_mat                    15015
#define id_massfx_modrb_meshes                 15016
#define id_massfx_modrb_advanced               15017
#define id_massfx_modrb_meshparam_sphere       15018
#define id_massfx_modrb_meshparam_box          15019
#define id_massfx_modrb_meshparam_capsule      15020
#define id_massfx_modrb_meshparam_convex       15021
#define id_massfx_modrb_meshparam_custom       15022
#define id_massfx_modrb_meshparam_composite    15023
#define id_massfx_skeleton_general             15024
#define id_massfx_uconstraints_connection      15026
#define id_massfx_uconstraints_transunit       15027
#define id_massfx_uconstraints_limits          15028
#define id_massfx_uconstraints_spring          15029
#define id_massfx_uconstraints_advanced        15030

#define topic_mesh_inspector                   15031 // About mesh validator
#define topic_startup_defaults                 15032
#define topic_particle_view                    15033

#define idh_security_tools                     15100 // Help for security tools - general
#define idh_security_tools_crp                 15101 // Help for security tools - CRP malicious script
#define idh_security_tools_adsl                15102 // Help for security tools - ADSL malicious script
#define idh_security_tools_alc                 15103 // Help for security tools - ALC malicious script

// Help for ATF importers
#define idh_atf_importers                      15200 // Parent topic
#define idh_atf_importer_solidworks            15201
#define idh_atf_importer_stl                   15202
#define idh_atf_importer_jt                    15203
#define idh_atf_importer_catiav4               15204
#define idh_atf_importer_catiav5               15205
#define idh_atf_importer_iges                  15206
#define idh_atf_importer_proe                  15207
#define idh_atf_importer_nx                    15208
#define idh_atf_importer_alias                 15209
#define idh_atf_importer_step                  15210
#define idh_atf_importer_sketchup              15211

#define idh_fluids_simulation                  16000 // Help for fluids simulation

// Security settings
#define idh_safe_scene_script_execution        17000
#define idh_scene_embedded_hyperlink           17001

//
// Internal note: Any change to this file will require HelpsysContextIDtoString.cpp to be updated too.
//
