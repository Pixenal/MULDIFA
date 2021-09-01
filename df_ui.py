""" 
-------------------------------------------------------------------------------------------------------------
    Copyright (C) 2021 Pixenal


    This file is part of blend_df_tool.

    blend_df_tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    blend_df_tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with blend_df_tool.  If not, see <https://www.gnu.org/licenses/>.
-------------------------------------------------------------------------------------------------------------
"""

import sys
import bpy
import bmesh


current_platform = sys.platform


# UI
#-------------------------------------------------------------------------------------------------------------#
    
    
#dfc_layers_uilist
class DF_UL_df_dfc_layers(bpy.types.UIList):

    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):
    
        df = context.scene.df
    
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
        
            row0 = layout.row(align = True)
            row0.prop(item, "name", text = "", emboss = False, icon = 'OUTLINER_OB_POINTCLOUD')
            
            
class DF_UL_df_dfr_layer_dfc_layers(bpy.types.UIList):

    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
    
        df = context.scene.df
        
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            
            #layout.()
            
            row0 = layout.row(align = True)
            
            row0.prop_search(item, "dfc_layer", context.scene, "df_dfc_layers",text = "", icon = 'OUTLINER_OB_POINTCLOUD')
            remove_props = row0.operator("df.df_remove_dfr_layer_dfc_layer", text = "", icon = "REMOVE")
            remove_props.dfr_layer_indx = context.scene.df_dfr_layers_indx
            remove_props.dfr_layer_dfc_layer_indx = index
            
            
            
            
class DF_UL_df_dfr_layers(bpy.types.UIList):

    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):
    
        df = context.scene.df
        
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
        
            col0 = layout.column(align = True)
            col0.prop(item, "name", text = "", emboss = False, icon = 'NODE_TEXTURE')


#Parent panel for the below sub panels
class df_parent_panel(bpy.types.Panel):

    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "DF Tools"


#Main Distance Field sub Panel
class DF_PT_df(df_parent_panel, bpy.types.Panel):

    bl_idname = "DF_PT_df"
    bl_label = "Distance Field"
    
    def draw(self, context):
    
        #Checks if current platform is Windows
        if ((current_platform == "win32") or
            (current_platform == "linux") or
            (current_platform == "linux2") or
            (current_platform == "darwin")):
        
            #If so, behave as normal
        
            #Alias definitions
            df = context.scene.df
            layout = self.layout
            
            row0 = layout.row()
            row0.scale_y = 1.4
            props0 = row0.operator("df.df_update", text = "Update")
            props0.enable_non_object_modes = True
            
            layout.prop(df, "df_interp_mode", text = '')
            layout.prop(df, "df_gamma")
            
            col0 = layout.column(align = True)
            col0.prop(df, "df_update_vertex_colors")
            col0.prop(df, "df_update_vertex_groups")
            
            col1 = layout.column(align = True)
            col1.prop(df, "df_update_on_frame")
            col1.prop(df, "df_clean_dfr_vert_layers")
        
        else:
        
            #If not, display compatability message
            
            layout = self.layout
            
            layout.label(text = "######################")
            layout.label(text = " Unsupported OS       ")
            layout.label(text = "######################")
        
        
class DF_PT_df_layers(df_parent_panel, bpy.types.Panel):

    bl_idname = "DF_PT_df_layers"
    bl_label = "Layers"
    bl_parent_id = "DF_PT_df"
    
    def draw(self, context):
    
        #Alias definitions
        df = context.scene.df
        layout = self.layout
        
        row0 = layout.row()
        row0.scale_y = 1.4
        
        col0 = layout.column(align = True)
        
        col0.label(text = "Contributor Layers")
        
        col0_row0 = col0.row()
        
        col0_row0_col0 = col0_row0.column(align = True)
        col0_row0_col0.template_list("DF_UL_df_dfc_layers", "", context.scene, "df_dfc_layers", context.scene, "df_dfc_layers_indx")
        
        col0_row0_col0_row0 = col0_row0_col0.row(align = True)
        add_props = col0_row0_col0_row0.operator("df.df_assign_dfcs_to_dfc_layer", text = "Assign")
        add_props.dfc_layer_indx = context.scene.df_dfc_layers_indx
        remove_props = col0_row0_col0_row0.operator("df.df_remove_dfcs_from_dfc_layer", text = "Remove")
        remove_props.dfc_layer_indx = context.scene.df_dfc_layers_indx
        
        col0_row0_col0_row1 = col0_row0_col0.row(align = True)
        col0_row0_col0_row1.operator("df.df_select_dfcs_in_dfc_layer", text = "Select")
        col0_row0_col0_row1.operator("df.df_deselect_dfcs_in_dfc_layer", text = "Deselect")
        
        col0_row0_col1 = col0_row0.column(align = True)
        col0_row0_col1.operator("df.df_add_dfc_layer", text = "", icon = "ADD")
        col0_row0_col1.operator("df.df_remove_dfc_layer", text = "", icon = "REMOVE")
        
        col0.label(text = "")
        col0.label(text = "Recipient Layers")
        
        col0_row1 = col0.row()
        
        col0_row1_col0 = col0_row1.column(align = True)
        col0_row1_col0.template_list("DF_UL_df_dfr_layers", "", context.scene, "df_dfr_layers", context.scene, "df_dfr_layers_indx")
        
        col0_row1_col0_row0 = col0_row1_col0.row(align = True)
        add_props = col0_row1_col0_row0.operator("df.df_assign_dfrs_to_dfr_layer", text = "Assign")
        add_props.dfr_layer_indx = context.scene.df_dfr_layers_indx
        remove_props = col0_row1_col0_row0.operator("df.df_remove_dfrs_from_dfr_layer", text = "Remove")
        remove_props.dfr_layer_indx = context.scene.df_dfr_layers_indx
        
        col0_row1_col0_row1 = col0_row1_col0.row(align = True)
        col0_row1_col0_row1.operator("df.df_select_dfrs_in_dfr_layer", text = "Select")
        col0_row1_col0_row1.operator("df.df_deselect_dfrs_in_dfr_layer", text = "Deselect")
        
        col0_row1_col0 = col0_row1.column(align = True)
        col0_row1_col0.operator("df.df_add_dfr_layer", text = "", icon = "ADD")
        col0_row1_col0.operator("df.df_remove_dfr_layer", text = "", icon = "REMOVE")
        
        col0_row2 = col0.row()
    
        if (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)):

            col0_row2.template_list("DF_UL_df_dfr_layer_dfc_layers", "", context.scene.df_dfr_layers[context.scene.df_dfr_layers_indx], "dfc_layers", context.scene.df_dfr_layers[context.scene.df_dfr_layers_indx], "dfc_layers_indx")
    
            col0_row2_col0 = col0_row2.column(align = True)
            add_props = col0_row2_col0.operator("df.df_add_dfr_layer_dfc_layer", text = "", icon = "ADD")
            add_props.dfr_layer_indx = context.scene.df_dfr_layers_indx
        
#Cache Dir child panel
class DF_PT_df_cache(df_parent_panel, bpy.types.Panel):

    bl_idname = "DF_PT_df_cache_dir"
    bl_label = "Cache File"
    bl_parent_id = "DF_PT_df"
    
    def draw(self, context):
    
        df = context.scene.df
        layout = self.layout
        
        col0 = layout.column()
        
        layout.prop(df, "df_enable_cache", icon = 'DISK_DRIVE')
        layout.operator("df.df_make_df_cache_dir_rel")
        col0.enabled = df.df_enable_cache
        col0.prop(df, "df_cache_dir")
        
        
        
#Volume child Panel    
class DF_PT_df_volume(df_parent_panel, bpy.types.Panel):

    bl_idname = "DF_PT_df_volume"
    bl_label = "Volume"
    bl_parent_id = "DF_PT_df"
    
    def draw(self, context):
    
        #Alias definitions
        df = context.scene.df
        layout = self.layout
        
        volume_button_icon = ''
        volume_button_text = ""
        volume_button_op = ""
        
        
        # CHECKS VOLUME VALIDITY
        #-------------------------------------------------------------------------------------------------------------#
        
        
        df_volume_8_verts = True
        volume_amount = 0

        #Gets how many volumes are currenty in scene as well as 
        for obj in context.scene.objects:
        
            if ("_df_volume_"in(obj.name)):
            
                switched_mode = False  
            
                depsgraph = bpy.context.evaluated_depsgraph_get()
                
                obj_eval = obj.evaluated_get(depsgraph)
                mesh_eval = obj_eval.data
                
                if (obj.mode == 'EDIT'):
                
                    bm_cpy = bmesh.from_edit_mesh(mesh_eval)
                    
                    if (len(bm_cpy.verts) != 8):
                
                        df_volume_8_verts = False
            
                if (len(mesh_eval.vertices) != 8):
                
                    df_volume_8_verts = False
            
                volume_amount += 1
        
        
        # LAYOUT CONDITIONS
        #-------------------------------------------------------------------------------------------------------------#
        
        
        
        
        
        # IF ESTIMATED MEMORY USEAGE EXCEEDED HARD CODED MAX
        #-------------------------------------------------------------------------------------------------------------#
        
        
        if (df.df_init_returned_error == True):
        
            col0 = layout.column(align = True)
        
            #If volume initialization was canceled due to pre allocation memory estimate exceeding the hard coded max, set label and icon accordingly
            if (df.df_mem_est_exceeded_max == True):

                col0.label(text = "The estimated memory usage")
                col0.label(text = "exceeded the max, so the")
                col0.label(text = "operation was aborted.")
                col0.label(text = "Please adjust settings") 
            
            # IF VOLUME IS TOO SMALL
            #-------------------------------------------------------------------------------------------------------------#
            
            #If volume is too small, set label and icon accordingly
            elif (df.df_volume_too_small == True):

                col0.label(text = "Volume was too small given")
                col0.label(text = "the current settings. Please")
                col0.label(text = "adjust volume size or settings")
                
            prop0 = layout.operator("df.df_toggle_init_returned_error", text = "OK")
        
        else:
            
            # IF MORE THAN 1 VOLUME EXISTS
            #-------------------------------------------------------------------------------------------------------------#
            
            #If more than 1 volume exists, grey out volume button and display error message
            if (volume_amount > 1):
            
                layout.label(text = "! Max of 1 volume !")
            
                row0 = layout.row()
                row0.scale_y = 2.0
            
                row0.enabled = False
                
                volume_button_icon = 'ERROR'
                volume_button_text = "Too Many Volumes"
                volume_button_op = "df.df_initialize"
            
            
            # ELSE IF VOLUME AMOUNT == 1 AND VOLUME DOESN'T HAVE 8 VERTS
            #-------------------------------------------------------------------------------------------------------------#
            
            #If volume doesn't have 8 verts, grey out volume button and display error message
            elif ((volume_amount == 1) and (df_volume_8_verts == False)):
                
                layout.label(text = "! volume must have 8 verts !")
            
                row0 = layout.row()
                row0.scale_y = 2.0
            
                row0.enabled = False
                
                volume_button_icon = 'ERROR'
                volume_button_text = "Invalid Volume"
                volume_button_op = "df.df_initialize"
                
            # ELSE IF VOLUME AMOUNT == 1
            #-------------------------------------------------------------------------------------------------------------#
            
            #If 1 volume exists, display volume button based on below conditions
            elif(volume_amount == 1):
                    
                    
                ######################################################## SUB-CONDITION # ELSE IF VOLUME ISN'T INITIALIZED
                    
                    
                #If volume is not initialized, set label and icon accordingly
                if (df.df_volume_initialized == False):
                
                        layout.label(text = "-Volume not initialized-")
                        volume_button_icon = 'CUBE'


                # SUB-CONDITION # ELSE
                #-------------------------------------------------------------------------------------------------------------#


                else:
                    
                    
                    # SUB-SUB-CONDITION # IF VOLUME IS UP TO DATE
                    #-------------------------------------------------------------------------------------------------------------#
                    
                    
                    #If volume initialized, check if the current volume in scene matches the distance field structure currently in memory
                    if ((df.df_distance_last == df.df_distance) and (df.df_cmprt_size_last == df.df_cmprt_size) and (df.df_grid_spacing_last == df.df_grid_spacing) and (df.df_volume_same == 1)):
                    
                        #If so, display set label and icon accordingly
                        layout.label(text = "-Volume is up to date-")
                        volume_button_icon = 'SNAP_VOLUME'
                        
                        
                    # SUB-SUB-CONDITION # IF VOLUME IS OUTDATED
                    #-------------------------------------------------------------------------------------------------------------#
                        
                        
                    else:
                    
                        #If not, set label and icon accordingly
                        layout.label(text = "-Volume outdated-")
                        volume_button_icon = 'CUBE'
                    
                
                row0 = layout.row()
                row0.scale_y = 2.0
                
                volume_button_text = "Initialize Volume"
                volume_button_op = "df.df_initialize"
                
            # ELSE ( OCCURS IF VOLUME AMOUNT == 0 )
            #-------------------------------------------------------------------------------------------------------------#
                
            else:
            
                #If there are no volumes in scene, show "Create Volume" button
            
                row0 = layout.row()
                row0.scale_y = 2.0
            
                volume_button_icon = 'CUBE'
                volume_button_text = "Create Volume"
                volume_button_op = "df.df_create_volume"

            #Displays volume button based on the result of the above if/elif/else statement
            row0.operator(volume_button_op, text = volume_button_text, icon = volume_button_icon)
            
            #Volume properties
            col0 = layout.column(align = True)
            col0.prop(df, "df_distance")
            col0.prop(df, "df_grid_spacing")
            col0.prop(df, "df_cmprt_size")
        
        
class DF_PT_df_memory(df_parent_panel, bpy.types.Panel):

    bl_idname = "DF_PT_df_memory"
    bl_label = "Memory"
    bl_parent_id = "DF_PT_df"
    bl_options = {'DEFAULT_CLOSED'}
    
    def draw(self, context):
    
        df = context.scene.df
        layout = self.layout
        
        col0 = layout.column()
        col0.prop(df, "df_stashing_enabled")


# Registration and Unregistration
#-------------------------------------------------------------------------------------------------------------#

           
if 	((current_platform == "win32") or
	(current_platform == "linux") or
	(current_platform == "linux2") or
    (current_platform == "darwin")):

    classes = [ DF_PT_df,
                DF_PT_df_layers,
                DF_UL_df_dfc_layers,
                DF_UL_df_dfr_layer_dfc_layers,
                DF_UL_df_dfr_layers,
                DF_PT_df_volume,
                DF_PT_df_cache]
                
           
else:
    
    classes = [df_PT_df]
    
#Register
def register():

    for cls in classes:
    
        bpy.utils.register_class(cls)
        

#Unregister
def unregister():

    for cls in classes:
    
        bpy.utils.unregister_class(cls)
    
    
