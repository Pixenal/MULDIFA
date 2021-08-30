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
import os
import bpy.types
import bpy.props
import bpy.utils


current_platform = sys.platform

#Is just for testing
#current_platform = "linux"


# Handler Functions
#-------------------------------------------------------------------------------------------------------------#

def df_cache_dir_update(self, context):

    df = context.scene.df
    if (not df.df_cache_dir_skip_handler):
    
        print("UPDATING REL !!!!!!!!!!!!!!!!!!!!!!!!!!")
        df.df_cache_dir_is_rel = False


# PROPERTY GROUP
#-------------------------------------------------------------------------------------------------------------#


class df_dfc_id(bpy.types.PropertyGroup):

    name : bpy.props.StringProperty(default = "New id")
    id : bpy.props.IntProperty(default = 0)
    
class df_dfr_id(bpy.types.PropertyGroup):

    name : bpy.props.StringProperty(default = "New id")
    id : bpy.props.IntProperty(default = 0)

class df_dfc_layer_properties(bpy.types.PropertyGroup):

    name : bpy.props.StringProperty(default = "New Layer")
    dfc_ids_indx : bpy.props.IntProperty(default = 0)
    
class df_dfr_layer_dfc_layer_properties(bpy.types.PropertyGroup):

    name : bpy.props.StringProperty(default = "New Layer")
    
class df_dfr_layer_properties(bpy.types.PropertyGroup):

    name : bpy.props.StringProperty(default = "New Layer")
    dfr_ids_indx : bpy.props.IntProperty(default = 0)
    dfc_layers_indx : bpy.props.IntProperty(default = 0)


class df_properties(bpy.types.PropertyGroup):

    #Integral error codes may be better for some of these than multiple bool properties

    #Why are you prefixing them with df if they're already within a property group called df_properties?

    df_distance : bpy.props.FloatProperty(name = "Distance", default = 10, min = 0, max = 100000, description = "How far an objects influence extends")
    df_cmprt_size : bpy.props.IntProperty(name = "Compartment Size", default = 4, min = 2, soft_max = 32, description = "The Distance Field Volume is split into compartments for optimization, defines how many voxels per compartment, lower values generally give faster update times but slower initializing of volume times")
    df_grid_spacing : bpy.props.FloatProperty(name = "Cell Size", default = 1, min = .001, soft_min = .2, soft_max = 8, description = "The size of each grid cell, determines the resolution of the distance field, greatly effects compute times for both updating of the distance field and initialization of the volume")
    df_update_vertex_colors : bpy.props.BoolProperty(name = "Update Vertex Colors", default = True)
    df_update_vertex_groups : bpy.props.BoolProperty(name = "Update Vertex Groups", default = False)
    df_distance_last : bpy.props.FloatProperty()
    df_cmprt_size_last : bpy.props.IntProperty()
    df_grid_spacing_last : bpy.props.FloatProperty()
    df_volume_pos_x_last : bpy.props.FloatProperty()
    df_volume_pos_y_last : bpy.props.FloatProperty()
    df_volume_pos_z_last : bpy.props.FloatProperty()
    df_volume_initialized : bpy.props.BoolProperty(default = False)
    df_single_volume : bpy.props.BoolProperty(default = False)
    df_volume_same : bpy.props.IntProperty(default = 0)
    df_volume_too_small : bpy.props.BoolProperty(default = False)
    df_mem_est_exceeded_max : bpy.props.BoolProperty(default = False)
    df_next_dfc_id : bpy.props.IntProperty(default = 1)
    df_max_id : bpy.props.IntProperty(default = 10)
    df_next_dfr_id : bpy.props.IntProperty(default = 1)
    df_write_id_index : bpy.props.IntProperty(default = 0)
    df_write_id_rand : bpy.props.IntProperty(default = 0)
    df_cache_dir : bpy.props.StringProperty(name = "Cache Dir", subtype = 'DIR_PATH', update = df_cache_dir_update)
    df_cache_dir_is_rel : bpy.props.BoolProperty(default = False)
    df_cache_dir_skip_handler : bpy.props.BoolProperty(default = False)
    df_enable_cache : bpy.props.BoolProperty(name = "Enable Cache", default = False)
    df_state_stashed : bpy.props.BoolProperty(name = "State Stashed", default = False)
    df_stashing_enabled : bpy.props.BoolProperty(name = "Enable Stashing", default = False)
    df_init_returned_error : bpy.props.BoolProperty(default = False)
    df_assertion_code : bpy.props.IntProperty(default = 0)
    df_interp_mode : bpy.props.EnumProperty(name = "Interp Mode", default = 0, items = [('0', 'TriCubic', ''), ('1', 'TriLinear', ''), ('2', 'Nearest', '')])
    df_gamma : bpy.props.FloatProperty(name = "Gamma", default = 1.0)
    df_update_on_frame : bpy.props.BoolProperty(name = "Update on Frame Change", default = False)
    df_undo_index : bpy.props.IntProperty(default = 0)



# Registration
#-------------------------------------------------------------------------------------------------------------#


classes = [ df_dfc_id,
            df_dfr_id,
            df_dfc_layer_properties,
            df_dfr_layer_properties,
            df_dfr_layer_dfc_layer_properties,
            df_properties]

#Register
def register():

    if 	((current_platform == "win32") or
    	(current_platform == "linux") or
    	(current_platform == "linux2") or
        (current_platform == "darwin")):
    
        for cls in classes:
        
            bpy.utils.register_class(cls)
            
        bpy.types.Object.dfc_id = bpy.props.IntProperty(name = "dfc ID", default = 0)
        bpy.types.Object.dfr_id = bpy.props.IntProperty(name = "dfr ID", default = 0)
        bpy.types.Scene.df_dfc_layers = bpy.props.CollectionProperty(type = df_dfc_layer_properties)
        bpy.types.Scene.df_dfc_layers_indx = bpy.props.IntProperty(default = 0)
        df_dfc_layer_properties.dfc_ids = bpy.props.CollectionProperty(type = df_dfc_id)
        df_dfr_layer_dfc_layer_properties.dfc_layer = bpy.props.StringProperty()
        bpy.types.Scene.df_dfr_layers = bpy.props.CollectionProperty(type = df_dfr_layer_properties)
        bpy.types.Scene.df_dfr_layers_indx = bpy.props.IntProperty(default = 0)
        df_dfr_layer_properties.dfr_ids = bpy.props.CollectionProperty(type = df_dfr_id)
        df_dfr_layer_properties.dfc_layers = bpy.props.CollectionProperty(type = df_dfr_layer_dfc_layer_properties)
        bpy.types.Scene.df = bpy.props.PointerProperty(type = df_properties)
    
    
#Unregister
def unregister():

    if 	((current_platform == "win32") or
    	(current_platform == "linux") or
    	(current_platform == "linux2")):
    
        for cls in classes:
        
            bpy.utils.unregister_class(cls)
            
        del bpy.types.Scene.df
