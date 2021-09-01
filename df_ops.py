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
import os.path
import bpy
import ctypes
import time
import bmesh
import mathutils
import numpy
from bpy.app.handlers import persistent
import atexit
    
current_platform = sys.platform

#Is just for testing
#  V             V
#current_platform = "linux"

current_dir = os.path.dirname(os.path.realpath(__file__))

if (current_platform == "win32"):

    current_dir += "\lib\df_win\\"
    lib_file_name = "df_win.dll"
    
elif ((current_platform == "linux") or (current_platform == "linux2")):

    current_dir += "/lib/df_linux/"
    lib_file_name = "df_linux.so"
    
elif (current_platform == "darwin"):

    current_dir += "/lib/df_macos/"
    lib_file_name = "df_macos.dylib"

#Loads "dynamic library"
df_lib = ctypes.cdll.LoadLibrary(current_dir + lib_file_name)
df_lib.pass_lib_dir(ctypes.c_char_p(bytes(current_dir, 'utf-8')))


# Classes
#-------------------------------------------------------------------------------------------------------------#


class bmesh_type():

    bm_cpy = None
    obj = None
    mesh = None
    switched_mode = False
    original_mode = None

    def __init__(self, obj):

        self.obj = obj
        self.mesh = obj.data
        
        """ First checks if the current object's mode is object mode, if so, gets current mesh as bmesh structure using object mode
            specific process and continues as normal (This is checked first as most objects would pressumably be in object mode,
            and so checking if the mode is object mode, and skipping the extra check if so, would hopefully minimize the amount of
            condition checking done for the total amount of objects (even though it results in repeated code)). If not object mode,
            checks if is edit mode, if so, gets current mesh as bmesh structure in the process of doing so that is specific to edit
            mode. If is not edit mode (which would be the case if the mode is, for example, weight paint mode), then the object's mode is
            switched to object mode, the reason for this is purely precautionary and is done in order to avoid any unwanted behaviour 
            (of which I am not aware of) that could arise from using these modes, though it very well may be unnecessary """ 
        if (obj.mode == 'OBJECT'):
            
            #Internal references/ Aliases
            self.bm_cpy = bmesh.new()
            self.bm_cpy.from_mesh(self.mesh)
            
        else:
            
            if (obj.mode == 'EDIT'):
            
                self.bm_cpy = bmesh.from_edit_mesh(self.mesh)
                
            else:
                
                self.switched_mode = True
                
                #Stores the current mode so that it can be switched back to after the initialization process is complete
                self.original_mode = obj.mode
                bpy.ops.object.mode_set(mode = 'OBJECT')
                
                self.bm_cpy = bmesh.new()
                self.bm_cpy.from_mesh(self.mesh)

    def __del__(self):

        #Checks which mode is the current mode, and updates mesh accordingly
        if (self.obj.mode == 'EDIT'):
        
            bmesh.update_edit_mesh(self.mesh)
            
        else:
        
            self.bm_cpy.to_mesh(self.mesh)
            
            self.bm_cpy.free()
            
            #If mode was switched to object mode, swtiched back to original mode
            if (self.switched_mode == True):
            
                bpy.ops.object.mode_set(mode = self.original_mode)



class id_abstractor_type():

    id_type = 0
    layers_col = None
    assign_libfunc = None
    remove_libfunc = None
    expel_libfunc = None
    defrag_libfunc = None
    get_serchee_layers_libfunc = None

    def __init__(self, context, id_type):

        self.id_type = id_type
        if (id_type == 0):
            
            self.layers_col = context.scene.df_dfc_layers
            self.assign_libfunc = df_lib.call_df_assign_dfcs_to_dfc_layer
            self.remove_libfunc = df_lib.call_df_remove_dfcs_from_dfc_layer
            self.expel_libfunc = df_lib.call_df_expel_nonexistant_dfcs_from_layers
            self.defrag_libfunc = df_lib.call_df_defrag_dfc_ids
            self.get_serchee_layers_libfunc = df_lib.call_df_get_all_layers_with_dfc

        else:

            self.layers_col = context.scene.df_dfr_layers
            self.assign_libfunc = df_lib.call_df_assign_dfrs_to_dfr_layer
            self.remove_libfunc = df_lib.call_df_remove_dfrs_from_dfr_layer
            self.expel_libfunc = df_lib.call_df_expel_nonexistant_dfrs_from_layers
            self.defrag_libfunc = df_lib.call_df_defrag_dfr_ids
            self.get_serchee_layers_libfunc = df_lib.call_df_get_all_layers_with_dfr

    def get_ids_col_in_layer(self, context, layer):

        if (self.id_type == 0):

            return layer.dfc_ids

        else:

            return layer.dfr_ids

    def set_ids_indx_in_layer(self, context, layer, index):

        if (self.id_type == 0):

            layer.dfc_ids_indx = index

        else:

            layer.dfr_ids_indx = index

    def add_item_to_ids_col_in_layer(self, context, layer, id):

        if (self.id_type == 0):
            
            layer.dfc_ids_indx = len(layer.dfc_ids)
            layer.dfc_ids.add()
            layer.dfc_ids[layer.dfc_ids_indx].id = id
        
        elif (self.id_type == 1):

            layer.dfr_ids_indx = len(layer.dfr_ids)
            layer.dfr_ids.add()
            layer.dfr_ids[layer.dfr_ids_indx].id = id

    def get_id_in_obj(self, context, obj):

        if (self.id_type == 0):

            return obj.dfc_id

        else:

            return obj.dfr_id

    def set_id_in_obj(self, context, obj, id):

        if (self.id_type == 0):

            obj.dfc_id = id

        else:

            obj.dfr_id = id

    def get_next_id(self, context):

        df = context.scene.df

        if (self.id_type == 0):

            return df.df_next_dfc_id

        else:

            return df.df_next_dfr_id

    def set_next_id(self, context, id):

        df = context.scene.df

        if (self.id_type == 0):

            df.df_next_dfc_id = id

        else:

            df.df_next_dfr_id = id

    def set_ids_col_indx_to_end(self, context, layer):

        if (self.id_type == 0):

            layer.dfc_ids_indx = (len(layer.dfc_ids) - 1)

        else:

            layer.dfr_ids_indx = (len(layer.dfr_ids) - 1)


class rerun_item_type:

    sercher = None
    serchee = None
    serchee_layers = []


# Ctypes psuedo structs
#-------------------------------------------------------------------------------------------------------------#


class coord_xyz_type(ctypes.Structure):
    _fields_ = [("x", ctypes.c_double),
                ("y", ctypes.c_double),
                ("z", ctypes.c_double)]


class tri_info_type(ctypes.Structure):
    _fields_ = [("vert_0", ctypes.c_ulong),
                ("vert_1", ctypes.c_ulong),
                ("vert_2", ctypes.c_ulong),
                ("normal", coord_xyz_type),
                ("d", ctypes.c_double)]


class vert_info_type(ctypes.Structure):
    _fields_ = [("coord", coord_xyz_type),
                ("value", ctypes.c_float)]


class loop_info_type(ctypes.Structure):
    _fields_ = [("vert_index", ctypes.c_ulong),] 


class concave_entity_type(ctypes.Structure):
    _fields_ = [("coord", coord_xyz_type),
                ("rot", coord_xyz_type),
                ("scale", coord_xyz_type)]
                

class write_id_type(ctypes.Structure):
    _fields_ = [("index", ctypes.c_int),
                ("rand", ctypes.c_int)]


# Misc functions
#-------------------------------------------------------------------------------------------------------------#


def assign_ids_to_layer(context, objects, layer_indx, id_type):

    df = context.scene.df

    if not ((id_type == 0) or (id_type == 1)):

        return

    id_abstractor = id_abstractor_type(context, id_type)

    ids_type = ctypes.c_ulong * len(objects)
    ids = ids_type()
    ids_nxt_indx = ctypes.c_ulong(0)

    for obj in objects:

        if (id_abstractor.get_id_in_obj(context, obj) == 0):

            id_abstractor.set_id_in_obj(context, obj, id_abstractor.get_next_id(context)) 
            id_abstractor.set_next_id(context, (id_abstractor.get_next_id(context) + 1))
        
        exists = False
        for id in id_abstractor.get_ids_col_in_layer(context, id_abstractor.layers_col[layer_indx]):

            if (id.id == id_abstractor.get_id_in_obj(context, obj)):

                exists = True
                
        if (exists == False):

            id_abstractor.add_item_to_ids_col_in_layer(context, id_abstractor.layers_col[layer_indx], id_abstractor.get_id_in_obj(context, obj))
            ids[ids_nxt_indx.value] = id_abstractor.get_id_in_obj(context, obj)
            ids_nxt_indx.value += 1

    if (df.df_state_stashed):
        df_lib.call_df_unstash_state()
        df.df_state_stashed = False
    id_abstractor.assign_libfunc(ctypes.byref(ctypes.c_ulong(layer_indx)), ctypes.pointer(ids), ctypes.byref(ids_nxt_indx))
    if (df.df_stashing_enabled):
        df_lib.call_df_stash_state()
        df.df_state_stashed = True
    else:
        df.df_state_stashed = False


def remove_ids_from_layer(context, objects, layer_indx, id_type):

    df = context.scene.df

    if not ((id_type == 0) or (id_type == 1)):

        return

    id_abstractor = id_abstractor_type(context, id_type)

    slcted_obj_amount = len(objects)
    ids_type = ctypes.c_ulong * slcted_obj_amount
    ids = ids_type()
    ids_nxt_indx = ctypes.c_ulong(0)

    for obj in objects:
            
        ids[ids_nxt_indx.value] = id_abstractor.get_id_in_obj(context, obj)
        ids_nxt_indx.value += 1
        index = -1
        counter = 0
        layer_ids = id_abstractor.get_ids_col_in_layer(context, id_abstractor.layers_col[layer_indx])
        for id in layer_ids:
        
            if (id.id == id_abstractor.get_id_in_obj(context, obj)):
                
                index = counter
                break
                
            counter += 1
                
        if (index != -1):
            
            layer_ids.remove(index)
            
    if (df.df_state_stashed):
        df_lib.call_df_unstash_state()
        df.df_state_stashed = False
    id_abstractor.remove_libfunc(ctypes.byref(ctypes.c_ulong(layer_indx)), ctypes.pointer(ids), ctypes.byref(ids_nxt_indx))
    if (df.df_stashing_enabled):
        df_lib.call_df_stash_state()
        df.df_state_stashed = True
    else:
        df.df_state_stashed = False
    
    indx_counter = 0
    while (indx_counter < ids_nxt_indx.value):

        id_abstractor.set_id_in_obj(context, objects[ids[indx_counter]], 0)
        indx_counter += 1


def get_name_dup_num(name):

    name_len = len(name)
    num = ""
    
    index_counter = name_len - 1
    while(index_counter >= 0):
    
        if (name[index_counter].isdigit()):
        
            num = "".join((name[index_counter], num))
        
        elif(name[index_counter] == "."):
        
            return int(num)
        
        else:
        
            return int(-1)
        
        index_counter -= 1
        
        
def generate_uniq_dup_name(collection, base_name):
        
        max_dup_num = -2
        for element in collection:
        
            if ((base_name)in(element.name)):
            
                dup_num = get_name_dup_num(element.name)
                if (dup_num > max_dup_num):
                
                    max_dup_num = dup_num
        
        max_dup_num += 1
                
        if (max_dup_num > 0):
        
            if (max_dup_num < 10):
            
                layer_name = ".00" + str(max_dup_num)
                
            elif (max_dup_num < 100):
            
                layer_name = ".0" + str(max_dup_num)
                
            else:
            
                layer_name = "." + str(max_dup_num)
                
            return base_name + layer_name
        
        elif (max_dup_num == 0):
        
            return base_name + ".001"
        
        else:

            return base_name


def expel_nonexistant_ids_python(context, id_type):

    df = context.scene.df

    id_abstractor = id_abstractor_type(context, id_type)

    def check_against_ids(context, id):

        for obj in context.scene.objects:

            obj_id = id_abstractor.get_id_in_obj(context, obj)
            if (obj_id == id.id):

                return True

        return False

    counter = 0
    for layer in id_abstractor.layers_col:
        counter1 = 0
        layer_ids = id_abstractor.get_ids_col_in_layer(context, layer)
        while (counter1 < len(layer_ids)):

            if (not check_against_ids(context, layer_ids[counter1])):

                layer_ids.remove(counter1)
                id_abstractor.set_ids_col_indx_to_end(context, layer)
                counter1 -= 1

            counter1 += 1
                
        counter += 1


def defrag_ids(context, existing_objs, existing_obj_ids, existing_obj_ids_next_index, layers_col, id_type):

    df = context.scene.df

    if not ((id_type == 0) or (id_type == 1)):

        return

    id_abstractor = id_abstractor_type(context, id_type)

    validate_undo_step(context)

    existing_obj_ids_buffer = existing_obj_ids.ctypes.data_as(ctypes.POINTER(ctypes.c_ulong))
    greatest_id = ctypes.c_int(0)
    if (df.df_state_stashed):
        df_lib.call_df_unstash_state()
        df.df_state_stashed = False
    id_abstractor.expel_libfunc(existing_obj_ids_buffer, ctypes.c_ulong(existing_obj_ids_next_index))
    expel_nonexistant_ids_python(context, id_type)
    id_abstractor.defrag_libfunc(existing_obj_ids_buffer, ctypes.byref(ctypes.c_ulong(existing_obj_ids_next_index)), ctypes.byref(greatest_id))
    if (df.df_stashing_enabled):
        df_lib.call_df_stash_state()
        df.df_state_stashed = True
    else:
        df.df_state_stashed = False

    layer_indx = 0
    for layer in layers_col:
        id_indx = 0
        layer_ids = id_abstractor.get_ids_col_in_layer(context, layer)
        for id in layer_ids:
            id_indx += 1
        layer_indx += 1

    index_counter  = 0
    while (index_counter < existing_obj_ids_next_index):

        layer_indx = 0
        for layer in layers_col:
            id_indx = 0
            layer_ids = id_abstractor.get_ids_col_in_layer(context, layer)
            for id in layer_ids:

                if (id.id == id_abstractor.get_id_in_obj(context, existing_objs[index_counter])):
                    
                    id.id = existing_obj_ids_buffer[index_counter]
                    break
                id_indx += 1
            layer_indx += 1
        id_abstractor.set_id_in_obj(context, existing_objs[index_counter], existing_obj_ids_buffer[index_counter])
        index_counter += 1

    layer_indx = 0
    for layer in layers_col:
        id_indx = 0
        layer_ids = id_abstractor.get_ids_col_in_layer(context, layer)
        for id in layer_ids:
            id_indx += 1
        layer_indx += 1

    if (greatest_id.value < df.df_max_id):

        id_abstractor.set_next_id(context, (greatest_id.value + 1))

    incrmt_undo_step(context)


def get_id_dup_serchee_layers(context, serchee_id, layers_col, id_type):

    df = context.scene.df

    if not ((id_type == 0) or (id_type == 1)):

        return

    id_abstractor = id_abstractor_type(context, id_type)

    serchee_layers_type = ctypes.c_ulong * len(layers_col)
    serchee_layers = serchee_layers_type()
    serchee_layers_nxt_indx = ctypes.c_ulong(0)

    if (df.df_state_stashed):
        df_lib.call_df_unstash_state()
        df.df_state_stashed = False
    id_abstractor.get_serchee_layers_libfunc(serchee_id, ctypes.pointer(serchee_layers), ctypes.byref(serchee_layers_nxt_indx))
    if (df.df_stashing_enabled):
        df_lib.call_df_stash_state()
        df.df_state_stashed = True
    else:
        df.df_state_stashed = False

    trimmed_serchee_layers_type = ctypes.c_ulong * serchee_layers_nxt_indx.value
    trimmed_serchee_layers = trimmed_serchee_layers_type()
    counter = 0
    while (counter < serchee_layers_nxt_indx.value):

        trimmed_serchee_layers[counter] = serchee_layers[counter]
        counter += 1

    return trimmed_serchee_layers


class rerun_item_type:

    sercher = None
    serchee = None
    serchee_layers = []


def validate_ids(context, rerun_items, id_type):

    df = bpy.context.scene.df

    if not ((id_type == 0) or (id_type == 1)):

        return

    id_abstractor = id_abstractor_type(context, id_type)

    rerun_items_size = len(rerun_items)
    if (rerun_items_size > 0):

        validate_undo_step(context)
        counter = 0
        while (counter < rerun_items_size):

            id_abstractor.set_id_in_obj(context, rerun_items[counter].sercher, id_abstractor.get_next_id(context))
            id_abstractor.set_id_in_obj(context, rerun_items[counter].serchee, id_abstractor.get_next_id(context))
            id_abstractor.set_next_id(context, (id_abstractor.get_next_id(context) + 1))
            obj_list_wrapper = [rerun_items[counter].sercher,]
            counter1 = 0

            while (counter1 < len(rerun_items[counter].serchee_layers)):

                assign_ids_to_layer(context, obj_list_wrapper, rerun_items[counter].serchee_layers[counter1], id_type)
                counter1 += 1
            
            counter += 1

        incrmt_undo_step(context)

    obj_amount = len(bpy.context.scene.objects)

    existing_objs = numpy.empty(obj_amount, dtype = object)
    existing_obj_ids = numpy.empty(obj_amount, dtype = numpy.int_)
    existing_obj_ids_next_index = 0
    return_rerun_items = []
    return_rerun_items_nxt_indx = 0

    for obj in bpy.context.scene.objects:
    
        obj_id = id_abstractor.get_id_in_obj(context, obj)
        if (obj_id > 0):
            
            index_counter = 0
            while (index_counter < existing_obj_ids_next_index):
                
                if (obj_id == existing_obj_ids[index_counter]):

                    if (id_abstractor.get_next_id(context) < df.df_max_id):

                        id_to_apply = id_abstractor.get_next_id(context)
                        id_abstractor.set_next_id(context, (id_to_apply + 1))

                    elif (existing_obj_ids_next_index < (df.df_max_id - 1)):

                        return_rerun_items.append(rerun_item_type())
                        return_rerun_items[return_rerun_items_nxt_indx].sercher = obj
                        return_rerun_items[return_rerun_items_nxt_indx].serchee = existing_objs[index_counter]
                        return_rerun_items[return_rerun_items_nxt_indx].serchee_layers = get_id_dup_serchee_layers(context, id_abstractor.get_id_in_obj(context, existing_objs[index_counter]), id_abstractor.layers_col, id_type)
                        obj_list_wrapper = [obj, existing_objs[index_counter]]
                        counter1 = 0
                        while (counter1 < len(return_rerun_items[return_rerun_items_nxt_indx].serchee_layers)):

                            remove_ids_from_layer(context, obj_list_wrapper, return_rerun_items[return_rerun_items_nxt_indx].serchee_layers[counter1], id_type)
                            counter1 += 1

                        existing_objs = numpy.delete(existing_objs, index_counter)
                        existing_obj_ids = numpy.delete(existing_obj_ids, index_counter)
                        existing_obj_ids_next_index -= 1

                        return_rerun_items_nxt_indx += 1
                        break
                        
                    else:

                        id_to_apply = 0

                    id_abstractor.set_id_in_obj(context, obj, id_to_apply)

                    if (id_to_apply > 0):

                        validate_undo_step(context)
                        
                        serchee_layers = get_id_dup_serchee_layers(context, id_abstractor.get_id_in_obj(context, existing_objs[index_counter]), id_abstractor.layers_col, id_type)
                        obj_list_wrapper = [obj,]
                        counter = 0
                        while (counter < len(serchee_layers)):

                            assign_ids_to_layer(context, obj_list_wrapper, serchee_layers[counter], id_type)
                            counter += 1

                        incrmt_undo_step(context)

                index_counter += 1

            existing_objs[existing_obj_ids_next_index] = obj
            existing_obj_ids[existing_obj_ids_next_index] = id_abstractor.get_id_in_obj(context, obj)
            existing_obj_ids_next_index += 1 

    if ((id_abstractor.get_next_id(context) >= df.df_max_id) and existing_obj_ids_next_index < (df.df_max_id - 1)):

        defrag_ids(context, existing_objs, existing_obj_ids, existing_obj_ids_next_index, id_abstractor.layers_col, id_type)

    return return_rerun_items


def validate_dfc_dfr_ids():

    rerun_items = validate_ids(bpy.context, [], 0)
    while(True):
        
        if (len(rerun_items) > 0):
            
            rerun_items = validate_ids(bpy.context, rerun_items, 0)

        else:

            break
    
    rerun_items = validate_ids(bpy.context, [], 1)
    while(True):
        
        if (len(rerun_items) > 0):
            
            rerun_items = validate_ids(bpy.context, rerun_items, 1)

        else:

            break
            
            
def dfc_layer_name_to_indx(context, dfc_layer_name):

    index_counter = 0
    for layer in context.scene.df_dfc_layers:
    
        if (layer.name == dfc_layer_name):
        
            return index_counter
            
        index_counter += 1
            
    return 0
    
    
def validate_undo_step(context, delete_further_steps = True):

    df = context.scene.df

    df_lib.call_df_validate_undo_step(ctypes.c_int(df.df_undo_index), ctypes.c_bool(delete_further_steps))

def incrmt_undo_step(context):

    df = context.scene.df
    
    df.df_undo_index += 1
    df_lib.call_df_incrmt_undo_step(ctypes.c_int(df.df_undo_index))


# Operators
#-------------------------------------------------------------------------------------------------------------#
                                                        

class DF_OT_df_create_volume(bpy.types.Operator):
    """Creates a distance field volume, this must be done (and said volume must be initialized) before the distance field can be updated"""
    bl_idname = "df.df_create_volume"
    bl_label = "DF Create Volume"
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
        
        #Checks if a volume exists in the scene
        
        volume_exists = False
        
        for obj in context.scene.objects:
            if ("_df_volume_"in(obj.name)):
                volume_exists = True
        
        #If not, creates one
        if (volume_exists == False):
        
            bpy.ops.mesh.primitive_cube_add(size = 80)
            bpy.context.active_object.name = "_df_volume_"
            bpy.context.active_object.display_type = 'WIRE'
        
        return {'FINISHED'}


class DF_OT_df_toggle_init_returned_error(bpy.types.Operator):
    bl_idname = "df.df_toggle_init_returned_error"
    bl_label = "toggle_init_returned_error"
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
    
        df = context.scene.df
    
        if (df.df_init_returned_error == False):
        
            df.df_init_returned_error = True
            
        else:
            
            df.df_init_returned_error = False
        
        return {'FINISHED'}


class DF_OT_df_initialize(bpy.types.Operator):
    """Initializes volume, this must be done before the distance field can be updated"""
    bl_idname = "df.df_initialize"
    bl_label = "DF Initialize Volume"
    bl_options = {'REGISTER', 'UNDO'}
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
        
        #Creates alias
        df = context.scene.df
        
        validate_undo_step(context)
        
        #Loops through each object in the scene
        volume_amount = 0
        for obj in context.scene.objects:
        
            if ("_df_volume_"in(obj.name)):
            
                volume = obj
                volume_amount += 1
                
        """ If only one volume exists, then continue with initialization 
            using the alis to the volume object set during the above loop """
        if (volume_amount == 1):
            
            """ Checks if the volumes current mode is not object mode, if this is the case
            then the current mode is stored in "original_mode" and the mode is then
            switched to object mode (the current mode is stored so that it can be switched
            back to after the initialization process is complete).
            The reason for switching to object mode is to be able to get changes that may
            have been made to the mesh during edit mode (if the current mode is edit mode).
            For modes other than edit mode (such as modes weight paint or vertex paint),
            the reason for switching is purely precautionary and is done in order to avoid 
            any unwanted behaviour (of which I am not aware of) that could arise from using 
            these modes, though it very well may be unnecessary """
            switched_mode = False
            if (volume.mode != 'OBJECT'):
            
                switched_mode = True
                original_mode = volume.mode
                bpy.ops.object.mode_set(mode = 'OBJECT')
        
            #Gets current evaluated dependency graph
            depsgraph = context.evaluated_depsgraph_get()
            
            #Gets object after dependency graph evaluation (so that modifiers and such are applied)
            volume_eval = volume.evaluated_get(depsgraph)
            
            #Creates alias
            mesh_eval = volume_eval.data
            
            #Checks if vertex count is equal to 8, as the initialization function in df_lib expects this
            if (len(mesh_eval.vertices) == 8):
                
                #Creates ctypes array type
                verts_buffer_type = coord_xyz_type * 8
                
                #Creates object of the above type, ie, actually creates an array object
                verts_buffer = verts_buffer_type()
                
                #Loops through each vert and adds coordinates to array vert_buffer
                for vert in mesh_eval.vertices:
                
                    #Gets vert coordinates in world space
                    vert_coord = volume_eval.matrix_world @ vert.co
                    
                    verts_buffer[vert.index].x = vert_coord[0]
                    verts_buffer[vert.index].y = vert_coord[1]
                    verts_buffer[vert.index].z = vert_coord[2]
                    
                                
                
                #Calls function "call_df_initialize_volume" in dynamic library
                if (df.df_state_stashed):
                    df_lib.call_df_unstash_state()
                    df.df_state_stashed = False
                else:
                    df_lib.call_df_weak_unstash_volume_local()
                        
                initialize_return = df_lib.call_df_initialize_volume(ctypes.pointer(verts_buffer), ctypes.c_float(df.df_distance), ctypes.c_ushort(df.df_cmprt_size), ctypes.c_double(df.df_grid_spacing), ctypes.c_bool(False))
                
                incrmt_undo_step(context)
                
                if (df.df_stashing_enabled):
                    df_lib.call_df_stash_state()
                    df.df_state_stashed = True
                else:
                    df_lib.call_df_weak_stash_volume_local()
                        
                    df.df_state_stashed = False
                
                #If the function returns 0, was successful
                if (initialize_return == 0):
                    
                    """ Sets the values that the volume was initialized with, so that it can be quickly determined at the UI level if
                        the values becomes out of sync, in which case "-Volume outdated-" will be displayed """
                    df.df_distance_last = df.df_distance
                    df.df_cmprt_size_last = df.df_cmprt_size
                    df.df_grid_spacing_last = df.df_grid_spacing
                    
                    df.df_init_returned_error = False
                    df.df_volume_initialized = True
                    
                    """ If the function returns 0, than it is safe to assume that the volume was of correct scale,
                    and the memory estimate did not exceed the hard coded max, as this is checked for in the function """
                    df.df_mem_est_exceeded_max = False
                    df.df_volume_too_small = False
                    
                    """ Now that the volume has just been initialized, this property is now set to indicate that,
                        atleast at this exact point in time, the volume is in sync with the volume in scene as well
                        as with the properties that effect the volume """
                    df.df_volume_same = 1;
                elif (initialize_return == 3):
                    
                    df.df_init_returned_error = True
                    df.df_mem_est_exceeded_max = True
                    df.df_volume_initialized = False
                    df.df_volume_too_small = False
                    
                #If function returns 1, then initialization was not successful as the volume was too small
                elif (initialize_return == 1):
                
                    df.df_init_returned_error = True
                    df.df_volume_too_small = True
                    df.df_mem_est_exceeded_max = False
                    df.df_volume_initialized = False
                    
                    
            #If the object's mode was switched, switches it back to what it was origionally set to
            if (switched_mode == True):
                
                bpy.ops.object.mode_set(mode = original_mode)
                
        return {'FINISHED'}
    

class DF_OT_df_update(bpy.types.Operator):
    """Updates the distance field, as well as recipients"""
    bl_idname = "df.df_update"
    bl_label = "DF Update"
    bl_options = {'REGISTER', 'UNDO'}
    
    enable_non_object_modes : bpy.props.BoolProperty()
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return (df.df_volume_initialized) and (df.df_assertion_code == 0)
    
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        validate_dfc_dfr_ids()
        expel_nonexistant_ids_python(context, 0)
        expel_nonexistant_ids_python(context, 1)
    
        #Gets current evaluated dependency graph
        depsgraph = context.evaluated_depsgraph_get()
        if (df.df_volume_initialized == True):
            
            dfc_ids_type = ctypes.c_ulong * len(context.scene.objects)
            dfc_ids = dfc_ids_type()
            ignored_dfcs_list = []
            ignored_dfcs_nxt_indx = 0
            dfc_vert_amount_total = ctypes.c_ulong(0)
            dfc_amount = ctypes.c_ulong(0)
            for obj in context.scene.objects:
            
                if (obj.dfc_id > 0):
                
                    if ((obj.mode != 'OBJECT') and (not self.enable_non_object_modes)):
                    
                        ignored_dfcs_list.append(obj.dfc_id)
                        ignored_dfcs_nxt_indx += 1
                        
                    else:
                    
                        obj_eval = obj.evaluated_get(depsgraph)
                        mesh_eval = obj_eval.data
                        dfc_ids[dfc_amount.value] = obj.dfc_id
                        dfc_vert_amount_total.value += len(mesh_eval.vertices)
                        dfc_amount.value += 1
            
            ignored_dfcs_type = ctypes.c_ulong * ignored_dfcs_nxt_indx
            ignored_dfcs = ignored_dfcs_type()
            counter = 0
            for dfc in ignored_dfcs_list:
            
                ignored_dfcs[counter] = ignored_dfcs_list[counter]
                counter += 1
                  
            #Calls call_df_pre_update, this does things that need to be done once per update operation, and not once per object
            if (df.df_state_stashed):
                df_lib.call_df_unstash_state()
                df.df_state_stashed = False
            else:
                df_lib.call_df_weak_unstash_volume_local()

            df_lib.call_df_pre_update(ctypes.pointer(dfc_ids), ctypes.byref(dfc_amount), ctypes.byref(dfc_vert_amount_total), ctypes.pointer(ignored_dfcs), ctypes.c_ulong(ignored_dfcs_nxt_indx))
            
            # Updates distance field structures
            #-------------------------------------------------------------------------------------------------------------#
            
            dfc_index = ctypes.c_ulong(0)
            for obj in context.scene.objects:
            
                if (obj.dfc_id > 0):
                
                    """ Checks if the objects current mode is not object mode, if this is the case
                    then the current mode is stored in "original_mode" and the mode is then
                    switched to object mode (the current mode is stored so that it can be switched
                    back to after the initialization process is complete).
                    The reason for switching to object mode is to be able to get changes that may
                    have been made to the mesh during edit mode (if the current mode is edit mode).
                    For modes other than edit mode (such as modes weight paint or vertex paint),
                    the reason for switching is purely precautionary and is done in order to avoid 
                    any unwanted behaviour (of which I am not aware of) that could arise from using 
                    these modes, though it very well may be unnecessary """
                    switched_mode = False
                    if (obj.mode != 'OBJECT'):
                    
                        if (self.enable_non_object_modes == False):
                        
                            continue
                            
                        else:
                        
                            switched_mode = True
                            original_mode = obj.mode
                            bpy.ops.object.mode_set(mode = 'OBJECT')
                
                    #Gets object after dependency graph evaluation (so that modifiers and such are applied)
                    obj_eval = obj.evaluated_get(depsgraph)
                    #Creates alias
                    mesh_eval = obj_eval.data
                    #Triangulates mesh so that triangles can be gotten instead of polygons (this is requiried otherwise .loop_triangles will be empty)
                    mesh_eval.calc_loop_triangles()
                    
                    #Gets amount of triangles in mesh
                    tri_amount = ctypes.c_ulong(len(mesh_eval.loop_triangles))
                    vert_amount = ctypes.c_ulong(len(mesh_eval.vertices))

                    #Create ctypes array types (these are only types, not objects)
                    verts_buffer_type = coord_xyz_type * vert_amount.value
                    tris_buffer_type = tri_info_type * tri_amount.value
                    
                    #Creates array objects
                    verts_buffer = verts_buffer_type()
                    tris_buffer = tris_buffer_type()
                    
                    
                    for vert in mesh_eval.vertices:
                    
                        #Gets coordinates of current vertex in world space
                        vert_coord = obj_eval.matrix_world @ vert.co
                        verts_buffer[vert.index].x = vert_coord[0]
                        verts_buffer[vert.index].y = vert_coord[1]
                        verts_buffer[vert.index].z = vert_coord[2]
                    
                    #Loops through all triangles in mesh
                    for tri in mesh_eval.loop_triangles:
                        
                        #Adds the indices of the current tri's vertices to the current tri's entry in the tri buffer
                        tris_buffer[tri.index].vert_0 = tri.vertices[0]
                        tris_buffer[tri.index].vert_1 = tri.vertices[1]
                        tris_buffer[tri.index].vert_2 = tri.vertices[2]
                        
                    bounds_buffer_type = coord_xyz_type * 8
                    bounds_buffer = bounds_buffer_type()
                    
                    vert_index = 0
                    for vert in obj_eval.bound_box:
                    
                        vert_coord = obj_eval.matrix_world @ mathutils.Vector(vert)
                        bounds_buffer[vert_index].x = vert_coord.x
                        bounds_buffer[vert_index].y = vert_coord.y
                        bounds_buffer[vert_index].z = vert_coord.z
                        vert_index += 1
                    
                    df_lib.call_df_add_dfc_to_cache(ctypes.pointer(verts_buffer), ctypes.byref(vert_amount), ctypes.pointer(tris_buffer), ctypes.byref(tri_amount), ctypes.pointer(bounds_buffer), ctypes.byref(dfc_index), ctypes.byref(ctypes.c_bool(False)))

                    #Calls function "call_df_update" in dynamic library
                    df_lib.call_df_update(ctypes.byref(ctypes.c_ulong(obj.dfc_id)), ctypes.byref(dfc_index))
                    
                    #If the object's mode was switched, switches it back to what it was origionally set to
                    if (switched_mode == True):
                    
                        bpy.ops.object.mode_set(mode = original_mode)
                    dfc_index.value += 1
                    
        incrmt_undo_step(context)
                        
        bpy.ops.df.df_update_recipients()
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df_lib.call_df_weak_stash_volume_local()
            df.df_state_stashed = False
        return {'FINISHED'}
            
            
class DF_OT_df_update_recipients(bpy.types.Operator):   
    bl_idname = "df.df_update_recipients"
    bl_label = "DF Update Recipients"
    bl_options = {'REGISTER', 'UNDO'}
    """ Writes, if enabled, the updated state of the distance field structure into vertex colors and/or vertex groups of objects that belong to a recipient layer """
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute (self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        depsgraph = context.evaluated_depsgraph_get()
        if (df.df_volume_initialized == True):
            
            """ First checks if either update_vertex_colors or update_vertex_groups are actually set to True """
            if ((df.df_update_vertex_colors == True) or (df.df_update_vertex_groups == True)):
                
                dfrs_total_list = []
                dfrs_total_nxt_indx = 0
                
                for obj in context.scene.objects:
                    
                    if (obj.dfr_id != 0):
                    
                        dfrs_total_list.append(obj.dfr_id)
                        dfrs_total_nxt_indx += 1
                
                dfrs_total_type = ctypes.c_ulong * dfrs_total_nxt_indx
                dfrs_total = dfrs_total_type()
                
                counter = 0
                for dfr in dfrs_total_list:
                
                    dfrs_total[counter] = dfrs_total_list[counter]
                    counter += 1

                df_lib.call_df_pre_update_recipients(ctypes.pointer(dfrs_total), ctypes.c_ulong(dfrs_total_nxt_indx))

                """ Loops through each object in the scene, if an object is marked as a distance field recipient,
                    then update its vertex colors or/and vertex groups """
                
                dfr_layer_indx = 0
                for dfr_layer in context.scene.df_dfr_layers:
                
                    layer_size = df_lib.call_df_get_dfr_layer_size(ctypes.byref(ctypes.c_ulong(dfr_layer_indx)))
                    dfr_ids_type = ctypes.c_ulong * layer_size
                    dfr_ids = dfr_ids_type()
                    df_lib.call_df_get_all_dfrs_in_dfr_layer(ctypes.byref(ctypes.c_ulong(dfr_layer_indx)), ctypes.pointer(dfr_ids))
                    indx_counter_layer_cont = 0
                    while (indx_counter_layer_cont < layer_size):
                        
                        in_layer = False
                        for obj in context.scene.objects:

                            if (obj.dfr_id == dfr_ids[indx_counter_layer_cont]):
                            
                                in_layer = True
                                break
                    
                        if (in_layer == True):
                            
                            # Sets up structures
                            #-------------------------------------------------------------------------------------------------------------#
                            
                            obj_bmesh = bmesh_type(obj)
                                    
                            #Gets amount of verts in bmesh    
                            vert_amount = ctypes.c_ulong(len(obj_bmesh.bm_cpy.verts))
                                
                            #Creates verts buffer array type, and then creates an object if said type
                            verts_buffer_type = vert_info_type * vert_amount.value
                            verts_buffer = verts_buffer_type()
                            
                            #Loops through each vertex in bmesh and adds its coords to the verts buffer
                            for vert in obj_bmesh.bm_cpy.verts:
                                
                                #Gets coordinates of current vertex in world space
                                vert_coord = obj.matrix_world @ vert.co
                                
                                verts_buffer[vert.index].coord.x = vert_coord[0]
                                verts_buffer[vert.index].coord.y = vert_coord[1]
                                verts_buffer[vert.index].coord.z = vert_coord[2]
                                
                            height_values_buffer_type = ctypes.c_float * vert_amount.value
                            height_values_buffer = height_values_buffer_type()
                                
                            if (df.df_update_vertex_colors == True):
                            
                                #Gets amount of loops in bmesh
                                loop_amount = ctypes.c_ulong(0)
                                for face in obj_bmesh.bm_cpy.faces:
                                    loop_amount.value += len(face.loops)
                                
                                #Creates loops buffer array type, and then creates an object if said type
                                loops_buffer_type = loop_info_type * loop_amount.value
                                loops_buffer = loops_buffer_type()
                                
                                #Adds each loop's respective vert index into said loops equivalent entry in the loops buffer
                                loop_index = 0
                                for face in obj_bmesh.bm_cpy.faces:
                                    
                                    for loop in face.loops:
                                        
                                        loops_buffer[loop_index].vert_index = loop.vert.index
                                
                                        loop_index += 1
                                
                            dfc_layers_nxt_indx = ctypes.c_ulong(len(dfr_layer.dfc_layers))
                            dfc_layers_type = ctypes.c_ulong * dfc_layers_nxt_indx.value
                            dfc_layers = dfc_layers_type()
                                
                            dfc_layer_indx = 0
                            for dfc_layer in dfr_layer.dfc_layers:
                            
                                dfc_layers[dfc_layer_indx] = dfc_layer_name_to_indx(context, dfc_layer.dfc_layer)
                                dfc_layer_indx += 1
                            
                            # Gets values of verts from distance field structure
                            #-------------------------------------------------------------------------------------------------------------#
                            
                            """ Calls function "call_df_get_vert_value" in df_lib, this writes a value to each vert in "verts_buffer" lerped from the values of  
                                the grid points surrounding said vert, more precisely the 8 grid points that make up the vertices of the grid cell that the vert sits within """
                            df_lib.call_df_update_recipient(ctypes.pointer(dfc_layers), ctypes.byref(dfc_layers_nxt_indx), ctypes.pointer(verts_buffer), ctypes.byref(vert_amount), ctypes.pointer(height_values_buffer), ctypes.c_int(int(df.df_interp_mode)), ctypes.c_float(df.df_gamma))
                            
                            # Updates vertex colors
                            #-------------------------------------------------------------------------------------------------------------#
                            
                            #First checks if updating vertex colors is enabled
                            if (df.df_update_vertex_colors == True):
                            
                                #Attempts to get vert color layer called _df_, if it doesn't exist, returns 0
                                df_vert_color_layer = obj_bmesh.bm_cpy.loops.layers.color.get(dfr_layer.name, 0)
                            
                                #Checks if the above method returned 0
                                if (df_vert_color_layer == 0):
                                    
                                    #If so, creates a new vertex color layer called _df_
                                    df_vert_color_layer = obj_bmesh.bm_cpy.loops.layers.color.new(dfr_layer.name)
                            
                                """ Sets each loop's vertex color to the value stored in said loop's respective vert's entry in the verts buffer
                                    (vertex colors are stored per loop, not per vertex) """
                                loop_index = 0
                                for face in obj_bmesh.bm_cpy.faces:
                                    
                                    for loop in face.loops:
                                    
                                        #Creates an alias to the current loops respective vert index
                                        loop_vert_index = loops_buffer[loop_index].vert_index
                                    
                                        #Creates an alias to the value stored within the vert's entry in the verts buffer
                                        value = verts_buffer[loop_vert_index].value
                                        
                                        #Sets the loops vertex color to equal the above value (the 1 at the end is the alpha)
                                        loop[df_vert_color_layer] = (value, value, value, 1.0)
                                
                                        loop_index += 1
                             
                            # Updates vertex groups
                            #-------------------------------------------------------------------------------------------------------------#
                            
                            #First checks if updating vertex groups is enabled
                            if (df.df_update_vertex_groups == True):
                            
                                #Checks if a vertex group called _df_ already exists, if not, creates one
                                df_vert_group_exists = False
                                for vert_group in obj.vertex_groups:
                                
                                    if (vert_group.name == dfr_layer.name):
                                        
                                        df_vert_group_exists = True
                                        
                                if (df_vert_group_exists == False):
                                
                                    obj.vertex_groups.new(name = dfr_layer.name)
                                    
                                #Gets bmesh's deform layer (afaik vertex groups are stored wihin deform layers)
                                df_deform = obj_bmesh.bm_cpy.verts.layers.deform.verify()
                                
                                """ Gets vertex group layer (this is different to the deform layer in bmesh (not that
                                    the below statment does not use bmesh)) """
                                df_vert_group = obj.vertex_groups.get(dfr_layer.name)
                                    
                                #Loops through each vert and sets it's vert group weight said verts entry in the verts buffer
                                for vert in obj_bmesh.bm_cpy.verts:
                                    
                                    """ In order to specify that you wish to write to a specific vertex's vertex group entry (when using bmesh),
                                        you need to specify both the deform layer and the vertex group, I wasn't able to find clear documentation
                                        on exactly why this is the case, however from what I can gather I think that each vertex's deform layer is,
                                        atleast in an informal sense, a 2D array, where the first dimension equates to an individual deform layer,
                                        and each deform layer contains an array of vertex groups, though this could be an incorrect interpretation,
                                        will research more on why this is """
                                    vert[df_deform][df_vert_group.index] = verts_buffer[vert.index].value
                                
                            # Clean up
                            #-------------------------------------------------------------------------------------------------------------#
                            
                            del obj_bmesh
                                    
                        indx_counter_layer_cont += 1
                              
                    dfr_layer_indx += 1
                            
        incrmt_undo_step(context)
                            
        return {'FINISHED'}
        
        
class  DF_OT_df_add_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_add_dfc_layer"
    bl_label = "Add Layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        layer_name = generate_uniq_dup_name(context.scene.df_dfc_layers, "New Layer")
             
        context.scene.df_dfc_layers_indx = len(context.scene.df_dfc_layers);             
        new_layer = context.scene.df_dfc_layers.add()
        new_layer.name = layer_name
            
        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        df_lib.call_df_add_dfc_layer()
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
            
        incrmt_undo_step(context)
        
        return {'FINISHED'}
        

class DF_OT_df_remove_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_remove_dfc_layer"
    bl_label = "Remove Layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (len(context.scene.df_dfc_layers) > 0) and (df.df_assertion_code == 0)

    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
    
        context.scene.df_dfc_layers.remove(context.scene.df_dfc_layers_indx)

        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        dfc_ids_size = df_lib.call_df_get_dfc_ids_size()
        expelled_dfcs_type = ctypes.c_ulong * dfc_ids_size
        expelled_dfcs = expelled_dfcs_type()
        expelled_dfcs_nxt_indx = ctypes.c_ulong(0)
        
        df_lib.call_df_remove_dfc_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfc_layers_indx)), ctypes.pointer(expelled_dfcs), ctypes.byref(expelled_dfcs_nxt_indx))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
            
        indx_counter = 0
        while (indx_counter < expelled_dfcs_nxt_indx.value):
        
            for obj in context.scene.objects:

                if (obj.dfc_id == expelled_dfcs[indx_counter]):
                
                    obj.dfc_id = 0
                    break
                    
            indx_counter += 1
        
        if (context.scene.df_dfc_layers_indx > 0):
        
            context.scene.df_dfc_layers_indx -= 1;
            
        incrmt_undo_step(context)
        
        return {'FINISHED'}

        
class DF_OT_df_assign_dfcs_to_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_assign_dfcs_to_dfc_layer"
    bl_label = "Assign DFCs to DFC layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    dfc_layer_indx : bpy.props.IntProperty()

    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfc_layers_indx < len(context.scene.df_dfc_layers)) and (df.df_assertion_code == 0)
        
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        objects = context.selected_objects
        assign_ids_to_layer(context, objects, self.dfc_layer_indx, 0)
    
        incrmt_undo_step(context)
    
        return {'FINISHED'}
        
class DF_OT_df_remove_dfcs_from_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_remove_dfcs_from_dfc_layer"
    bl_label = "Remove DFCs to DFC layer"
    bl_options = {'REGISTER', 'UNDO'}
       
    dfc_layer_indx : bpy.props.IntProperty()
       
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfc_layers_indx < len(context.scene.df_dfc_layers)) and(df.df_assertion_code == 0)
        
    def execute(self, context):
    
        df = context.scene.df

        validate_undo_step(context)

        objects = context.selected_objects
        remove_ids_from_layer(context, objects, self.dfc_layer_indx, 0)

        incrmt_undo_step(context)
        
        return {'FINISHED'}
        
class DF_OT_df_select_dfcs_in_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_select_dfcs_in_dfc_layer"
    bl_label = "Select DFCs in DFC layer"
    
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfc_layers_indx < len(context.scene.df_dfc_layers)) and (df.df_assertion_code == 0)
    
    def execute(self, context):
    
        df = context.scene.df

        validate_undo_step(context, False)
        
        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        layer_size = df_lib.call_df_get_dfc_layer_size(ctypes.byref(ctypes.c_ulong(context.scene.df_dfc_layers_indx)))
        dfc_ids_type = ctypes.c_ulong * layer_size
        dfc_ids = dfc_ids_type()
    
        df_lib.call_df_get_all_dfcs_in_dfc_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfc_layers_indx)), ctypes.pointer(dfc_ids))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
        
        indx_counter = 0
        while (indx_counter < layer_size):
        
            for obj in context.scene.objects:
            
                if (obj.dfc_id == dfc_ids[indx_counter]):
                
                    obj.select_set(True)
                    break
                    
            indx_counter += 1
    
        return {'FINISHED'}
        
        
class DF_OT_df_deselect_dfcs_in_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_deselect_dfcs_in_dfc_layer"
    bl_label = "Deselect DFCs in DFC layer"
    
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfc_layers_indx < len(context.scene.df_dfc_layers)) and (df.df_assertion_code == 0)
    
    def execute(self, context):
    
        df = context.scene.df
    
        validate_undo_step(context, False)

        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        layer_size = df_lib.call_df_get_dfc_layer_size(ctypes.byref(ctypes.c_ulong(context.scene.df_dfc_layers_indx)))
        dfc_ids_type = ctypes.c_ulong * layer_size
        dfc_ids = dfc_ids_type()
    
        df_lib.call_df_get_all_dfcs_in_dfc_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfc_layers_indx)), ctypes.pointer(dfc_ids))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
        
        indx_counter = 0
        while (indx_counter < layer_size):
        
            for obj in context.scene.objects:
            
                if (obj.dfc_id == dfc_ids[indx_counter]):
                
                    obj.select_set(False)
                    break
                    
            indx_counter += 1
    
        return {'FINISHED'}
        
        
class DF_OT_df_add_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_add_dfr_layer"
    bl_label = "Add DFR Layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        layer_name = generate_uniq_dup_name(context.scene.df_dfr_layers, "New Layer")
             
        context.scene.df_dfr_layers_indx = len(context.scene.df_dfr_layers);             
        context.scene.df_dfr_layers.add()
        new_layer = context.scene.df_dfr_layers[context.scene.df_dfr_layers_indx]
        new_layer.name = layer_name
        
        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        df_lib.call_df_add_dfr_layer()
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
            
        incrmt_undo_step(context)
        
        return {'FINISHED'}
        
        
class DF_OT_df_remove_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_remove_dfr_layer"
    bl_label = "Remove DFR Layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)) and (df.df_assertion_code == 0)
        
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)

        """ The below block of code removes vertex color and group layers, beloning to the recipient layer to be deleted, from any dfrs contained within said
            recipient layer (if deleting said color/ group layers is enabled, the option is disabled by default to avoid the potentially for accidental deletion
            of user authored layers that happen to share the same name) """

        if (df.df_clean_dfr_vert_layers):

            #Alias
            layer = context.scene.df_dfr_layers[context.scene.df_dfr_layers_indx]
            for id in layer.dfr_ids:

                for obj in context.scene.objects:

                    if (obj.dfr_id == id.id):

                        #Removes vertex color layer if exists
                        obj_bmesh = bmesh_type(obj)
                        vert_col_layer = obj_bmesh.bm_cpy.loops.layers.color.get(layer.name, 0)
                        if (vert_col_layer != 0):

                            obj_bmesh.bm_cpy.loops.layers.color.remove(vert_col_layer)
                        del obj_bmesh

                        #Removes vertex group layer if exists
                        vert_group = obj.vertex_groups.get(layer.name, 0)
                        if (vert_group != 0):

                            obj.vertex_groups.remove(vert_group)
    
        """ The below block of code removes the layer, then then expels any dfrs that were only wihin that layer (ie, expels any that do not have any other
            layers containing them) """

        context.scene.df_dfr_layers.remove(context.scene.df_dfr_layers_indx)
        
        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        dfr_ids_size = df_lib.call_df_get_dfr_ids_size()
        expelled_dfrs_type = ctypes.c_ulong * dfr_ids_size
        expelled_dfrs = expelled_dfrs_type()
        expelled_dfrs_nxt_indx = ctypes.c_ulong(0)
        
        df_lib.call_df_remove_dfr_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfr_layers_indx)), ctypes.pointer(expelled_dfrs), ctypes.byref(expelled_dfrs_nxt_indx))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
            
        indx_counter = 0
        while (indx_counter < expelled_dfrs_nxt_indx.value):
        
            for obj in context.scene.objects:

                if (obj.dfr_id == expelled_dfrs[indx_counter]):
                
                    obj.dfr_id = 0
                    break
                    
            indx_counter += 1
        
        if (context.scene.df_dfr_layers_indx > 0):
        
            context.scene.df_dfr_layers_indx -= 1;
            
        incrmt_undo_step(context)
    
        return {'FINISHED'}
        
        
class DF_OT_df_assign_dfrs_to_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_assign_dfrs_to_dfr_layer"
    bl_label = "Assign dfrs to dfr layer"
    bl_options = {'REGISTER', 'UNDO'}

    dfr_layer_indx : bpy.props.IntProperty()

    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)) and (df.df_assertion_code == 0)
        
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        objects = context.selected_objects
        assign_ids_to_layer(context, objects, self.dfr_layer_indx, 1)
            
        incrmt_undo_step(context)
    
        return {'FINISHED'}
        
class DF_OT_df_remove_dfrs_from_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_remove_dfrs_from_dfr_layer"
    bl_label = "Remove dfrs to dfr layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    dfr_layer_indx : bpy.props.IntProperty()
       
    @classmethod
    def poll(cls, context):
        
        df = bpy.context.scene.df
        
        return (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)) and (df.df_assertion_code == 0)
        
    def execute(self, context):
    
        df = context.scene.df
        
        validate_undo_step(context)
        
        objects = context.selected_objects
        remove_ids_from_layer(context, objects, self.dfr_layer_indx, 1)
        
        incrmt_undo_step(context)
        
        return {'FINISHED'}
        
class DF_OT_df_select_dfrs_in_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_select_dfrs_in_dfr_layer"
    bl_label = "Select dfrs in dfr layer"
    
    @classmethod
    def poll(cls, context):
    
        df = bpy.context.scene.df
        
        return (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)) and (df.df_assertion_code == 0)
    
    def execute(self, context):
    
        df = context.scene.df

        validate_undo_step(context, False)
    
        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        layer_size = df_lib.call_df_get_dfr_layer_size(ctypes.byref(ctypes.c_ulong(context.scene.df_dfr_layers_indx)))
        dfr_ids_type = ctypes.c_ulong * layer_size
        dfr_ids = dfr_ids_type()
    
        df_lib.call_df_get_all_dfrs_in_dfr_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfr_layers_indx)), ctypes.pointer(dfr_ids))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
        
        indx_counter = 0
        while (indx_counter < layer_size):
        
            for obj in context.scene.objects:
            
                if (obj.dfr_id == dfr_ids[indx_counter]):
                
                    obj.select_set(True)
                    break
                    
            indx_counter += 1
    
        return {'FINISHED'}
        
        
class DF_OT_df_deselect_dfrs_in_dfr_layer(bpy.types.Operator):

    bl_idname = "df.df_deselect_dfrs_in_dfr_layer"
    bl_label = "Deselect dfrs in dfr layer"
    
    @classmethod
    def poll(cls, context):
    
        df = bpy.context.scene.df
        
        return (context.scene.df_dfr_layers_indx < len(context.scene.df_dfr_layers)) and (df.df_assertion_code == 0)
    
    def execute(self, context):
    
        df = context.scene.df
    
        validate_undo_step(context, False)

        if (df.df_state_stashed):
            df_lib.call_df_unstash_state()
            df.df_state_stashed = False
        layer_size = df_lib.call_df_get_dfr_layer_size(ctypes.byref(ctypes.c_ulong(context.scene.df_dfr_layers_indx)))
        dfr_ids_type = ctypes.c_ulong * layer_size
        dfr_ids = dfr_ids_type()
    
        df_lib.call_df_get_all_dfrs_in_dfr_layer(ctypes.byref(ctypes.c_ulong(context.scene.df_dfr_layers_indx)), ctypes.pointer(dfr_ids))
        if (df.df_stashing_enabled):
            df_lib.call_df_stash_state()
            df.df_state_stashed = True
        else:
            df.df_state_stashed = False
        
        indx_counter = 0
        while (indx_counter < layer_size):
        
            for obj in context.scene.objects:
            
                if (obj.dfr_id == dfr_ids[indx_counter]):
                
                    obj.select_set(False)
                    break
                    
            indx_counter += 1
    
        return {'FINISHED'}
        
        
class DF_OT_df_add_dfr_layer_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_add_dfr_layer_dfc_layer"
    bl_label = "Add DFR Layer DFC layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    dfr_layer_indx : bpy.props.IntProperty()
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
    
    def execute(self, context):
    
        df = context.scene.df
        
        dfr_layer_dfc_layers = context.scene.df_dfr_layers[self.dfr_layer_indx].dfc_layers
        dfr_layer_dfc_layers_indx = context.scene.df_dfr_layers[self.dfr_layer_indx].dfc_layers_indx
        
        layer_name = generate_uniq_dup_name(dfr_layer_dfc_layers, "New Layer")
             
        dfr_layer_dfc_layers_indx = len(dfr_layer_dfc_layers);             
        dfr_layer_dfc_layers.add()
        new_layer = dfr_layer_dfc_layers[dfr_layer_dfc_layers_indx]
        new_layer.name = layer_name
        
        return {'FINISHED'}
        
        
class DF_OT_df_remove_dfr_layer_dfc_layer(bpy.types.Operator):

    bl_idname = "df.df_remove_dfr_layer_dfc_layer"
    bl_label = "Remove DFR Layer DFC layer"
    bl_options = {'REGISTER', 'UNDO'}
    
    dfr_layer_indx : bpy.props.IntProperty()
    dfr_layer_dfc_layer_indx : bpy.props.IntProperty()
        
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
    
        return df.df_assertion_code == 0;
        
    def execute(self, context):
    
        if (context.scene.df_dfr_layers[self.dfr_layer_indx].dfc_layers_indx < len(context.scene.df_dfr_layers[self.dfr_layer_indx].dfc_layers)):
        
            context.scene.df_dfr_layers[self.dfr_layer_indx].dfc_layers.remove(self.dfr_layer_dfc_layer_indx)
    
        return {'FINISHED'}
        
        
class DF_OT_df_make_df_cache_dir_rel(bpy.types.Operator):

    bl_idname = "df.df_make_df_cache_dir_rel"
    bl_label = "Make Path Relative"
    
    
    @classmethod
    def poll(cls, context):
    
        df = context.scene.df
        
        return (df.df_enable_cache and bpy.data.is_saved)
        
    def execute(self, context):
    
        df = context.scene.df
        
        if (not df.df_cache_dir_is_rel):
        
            df.df_cache_dir_skip_handler = True
            df.df_cache_dir = bpy.path.relpath(df.df_cache_dir)
            df.df_cache_dir_skip_handler = False
            df.df_cache_dir_is_rel = True
            
        return {'FINISHED'}


# Timer Functions
#-------------------------------------------------------------------------------------------------------------#


""" Periodically checks if the volume exists, as well as if the positions of it's vertices are different from
    what they were when the volume was initialized """
def periodical_volume_check():

    #Creates alias
    df = bpy.context.scene.df   
    
    #Gets current evaluated dependency graph
    depsgraph = bpy.context.evaluated_depsgraph_get()
    
    #Checks if volume vert's positions are outdated
    volume_amount = 0
    for obj in bpy.context.scene.objects:
        if ("_df_volume_"in(obj.name)):
        
            #Gets object after dependency graph evaluation (so that modifiers and such are applied)
            obj_eval = obj.evaluated_get(depsgraph)
            
            #Creats alias to mesh
            mesh_eval = obj_eval.data
            
            """ Checks if current volumes's mode is edit mode, if so, gets mesh in bmesh form using process
                specific to edit mode, otherwise, gets mesh in bmesh form using process specific to object mode. 
                The precautionary step done in the operators of switching modes, other than object and edit mode,
                to object mode isn't done here as it seems likely that constantly switching modes every short
                time interval could result in undesirable behaviour, and is likely to be more risky than just 
                leaving the mode set to what it current is """
            if (obj.mode == 'EDIT'):
            
                bm_cpy = bmesh.from_edit_mesh(mesh_eval)
                
            else:
                
                bm_cpy = bmesh.new()
                bm_cpy.from_mesh(mesh_eval)

            #Checks if current volume has 8 vertices, as the function in df_lib expects this
            if (len(bm_cpy.verts) == 8):
                
                #Creates a array type "verts_buffer_type" and creates an object of said type
                verts_buffer_type = coord_xyz_type * 8
                verts_buffer = verts_buffer_type()
                
                #Adds the coordinates of each vert to the verts buffer
                for vert in bm_cpy.verts:
                
                    #Gets current verts coords in world space
                    vert_coord = obj_eval.matrix_world @ vert.co
                    
                    verts_buffer[vert.index].x = vert_coord[0]
                    verts_buffer[vert.index].y = vert_coord[1]
                    verts_buffer[vert.index].z = vert_coord[2]
                    
                #Calls function "call_df_check_volume" in df_lib 
                df.df_volume_same = df_lib.call_df_check_volume(ctypes.pointer(verts_buffer))
            
            if (obj.mode != 'EDIT'):
                
                bm_cpy.free()
            
            volume_amount += 1
                    
                
    #If no volume can be found, cleans memory (removes dynamically allocated distance field related data structures from memory),
    #and sets "df_volume_initialized" to False (so that the UI panel can update accordingly)
    if (volume_amount == 0):
    
        if (df.df_volume_initialized == True):

            if (df.df_state_stashed):
                df_lib.call_df_unstash_state()
                df.df_state_stashed = False
            df_lib.call_df_clean()
            #df_lib.call_df_clean_special()
            
            df.df_volume_initialized = False
    
    return .1
    


# Handlers
#-------------------------------------------------------------------------------------------------------------#


""" The persistent function decorator prevents the handlers from being removed from the list of handler functions upon the loading
    of a new blend file """


@persistent    
def frame_change_post_handler(dummy):

    df = bpy.context.scene.df

    if ((bpy.ops.df.df_update.poll()) and (df.df_update_on_frame)):

        bpy.ops.df.df_update(enable_non_object_modes = False)


#Cleans memory (removes dynamically allocated distance field related data structures from memory) if a new blend file is loaded,
#also sets "df_volume_initialized" to False if so
@persistent
def df_load_pre_handler(dummy):

    df = bpy.context.scene.df
    
    df.df_assertion_code = 0
    
    #Calls function "call_df_clean_memory" in df_lib 
    if (df.df_state_stashed):
        df_lib.call_df_unstash_state()
        df.df_state_stashed = False
    else:
        df_lib.call_df_weak_unstash_volume_local()
    df_lib.call_df_clean()
    df_lib.call_df_clean_special()
        

""" Sets "df_volume_initialized" to False when a new file is loaded, this is done incase the current blend being loaded didn't have
    it's "df.df_volume_initialized" property set to false before it was unloaded """
@persistent
def df_load_post_handler(dummy):
    
    df = bpy.context.scene.df
    
    df.df_undo_index = 0
    
    max_dfc_id_length = 0
    for dfc_layer in bpy.context.scene.df_dfc_layers:
        
        if (max_dfc_id_length < len(dfc_layer.dfc_ids)):
        
            max_dfc_id_length = len(dfc_layer.dfc_ids)
            
    max_dfr_id_length = 0
    for dfr_layer in bpy.context.scene.df_dfr_layers:
        
        if (max_dfr_id_length < len(dfr_layer.dfr_ids)):
        
            max_dfr_id_length = len(dfr_layer.dfr_ids)
    
    dfc_ids_type = ctypes.c_int * (max_dfc_id_length + 1)
    dfr_ids_type = ctypes.c_int * (max_dfr_id_length + 1)
    dfc_layers_type = ctypes.POINTER(dfc_ids_type) * (len(bpy.context.scene.df_dfc_layers) + 1)
    dfr_layers_type = ctypes.POINTER(dfr_ids_type) * (len(bpy.context.scene.df_dfr_layers) + 1)
    dfc_layers = dfc_layers_type()
    dfr_layers = dfr_layers_type()
    
    if (len(bpy.context.scene.df_dfc_layers) > 0):
    
        dfc_layers[0].contents = dfc_ids_type()
        dfc_layers[0].contents[0] = len(bpy.context.scene.df_dfc_layers)
        counter = 1
        for dfc_layer in bpy.context.scene.df_dfc_layers:

            dfc_layers[counter].contents = dfc_ids_type()
            dfc_layers[counter].contents[0] = len(dfc_layer.dfc_ids)
            
            counter_1 = 1
            for dfc_id in dfc_layer.dfc_ids:
                
                dfc_layers[counter].contents[counter_1] = dfc_id.id
                counter_1 += 1
            
            counter += 1
    
    else:
    
        dfc_layers[0].contents = dfc_ids_type()
        dfc_layers[0].contents[0] = 0
        
    if (len(bpy.context.scene.df_dfr_layers) > 0):
        
        dfr_layers[0].contents = dfr_ids_type()
        dfr_layers[0].contents[0] = len(bpy.context.scene.df_dfr_layers)
        counter = 1
        for dfr_layer in bpy.context.scene.df_dfr_layers:
        
            dfr_layers[counter].contents = dfr_ids_type()
            dfr_layers[counter].contents[0] = len(dfr_layer.dfr_ids)
            
            counter_1 = 1
            for dfr_id in dfr_layer.dfr_ids:
                
                dfr_layers[counter].contents[counter_1] = dfr_id.id
                counter_1 += 1
            
            counter += 1
            
    else:

        dfr_layers[0].contents = dfr_ids_type()
        dfr_layers[0].contents[0] = 0
            
    write_id = write_id_type()
    write_id.index = df.df_write_id_index
    write_id.rand = df.df_write_id_rand
    
    if (df.df_cache_dir_is_rel):
            
        df_cache_dir_abs = bpy.path.abspath(df.df_cache_dir)
        if (df_cache_dir_abs[-1] == "."):
            
            df_cache_dir_abs = df_cache_dir_abs[:-1]
            
    else:
        
        df_cache_dir_abs = df.df_cache_dir
    
    return_code = df_lib.call_df_new_blend_handler(ctypes.c_char_p(bytes(df_cache_dir_abs, 'utf-8')), ctypes.c_char_p(bytes(os.path.splitext(bpy.path.basename(bpy.data.filepath))[0], 'utf-8')), ctypes.byref(write_id), ctypes.pointer(dfc_layers), ctypes.pointer(dfr_layers), ctypes.c_bool(df.df_enable_cache))


    if (return_code == 1):

        #Cache file was found, but was invalid
        print("DF WARNING: Cache was invalid. This can occur if the cache is old or corrupt")
        df.df_volume_initialized = False

    elif (return_code == 2):
    
        #Cache file was loaded successfully
        df.df_volume_initialized = True

    elif (return_code == 3):

        #Cache is disabled
        df.df_volume_initialized = False

    elif (return_code == 4):

        #Cache file was not found
        print("DF WARNING: Cache file not found")
        df.df_volume_initialized = False
        
    if (df.df_stashing_enabled):
        df_lib.call_df_stash_state()
        df.df_state_stashed = True
    else:
        df_lib.call_df_weak_stash_volume_local()
        df.df_state_stashed = False
    
    
@persistent
def df_depsgraph_update_post_handler(dummy):

    df = bpy.context.scene.df

    validate_dfc_dfr_ids()
    
    
@persistent
def df_save_pre_handler(dummy):

    df = bpy.context.scene.df
    
    if (df.df_enable_cache == True):
    
        df.df_write_id_index = df_lib.call_df_get_write_id_index()
        df.df_write_id_rand = df_lib.call_df_get_write_id_rand()
        
        if (df.df_state_stashed):
        
            df_lib.call_df_stash_write_id()
		    
    
@persistent
def df_save_post_handler(dummy):

    df = bpy.context.scene.df
    
    validate_undo_step(bpy.context, False)
    
    if (df.df_enable_cache == True):
    
        if (not(df.df_state_stashed)):
        
            df_lib.call_df_weak_unstash_volume_local()
            df_lib.call_df_copy_to_buffer()
            
        if (df.df_cache_dir_is_rel):
        
            df_cache_dir_abs = bpy.path.abspath(df.df_cache_dir)
            if (df_cache_dir_abs[-1] == "."):
        
                df_cache_dir_abs = df_cache_dir_abs[:-1]
        
        else:
        
            df_cache_dir_abs = df.df_cache_dir
            
        df_lib.call_df_write_cache(ctypes.c_char_p(bytes(df_cache_dir_abs, 'utf-8')), ctypes.c_char_p(bytes(os.path.splitext(bpy.path.basename(bpy.data.filepath))[0], 'utf-8')))
        
        if (not(df.df_stashing_enabled)):
        
            df_lib.call_df_weak_stash_volume_local()
                        
        
# Registration
#-------------------------------------------------------------------------------------------------------------#


classes = [ DF_OT_df_create_volume,
            DF_OT_df_toggle_init_returned_error,
            DF_OT_df_initialize,
            DF_OT_df_update,
            DF_OT_df_update_recipients,
            DF_OT_df_add_dfc_layer,
            DF_OT_df_remove_dfc_layer,
            DF_OT_df_assign_dfcs_to_dfc_layer,
            DF_OT_df_remove_dfcs_from_dfc_layer,
            DF_OT_df_select_dfcs_in_dfc_layer,
            DF_OT_df_deselect_dfcs_in_dfc_layer,
            DF_OT_df_assign_dfrs_to_dfr_layer,
            DF_OT_df_remove_dfrs_from_dfr_layer,
            DF_OT_df_select_dfrs_in_dfr_layer,
            DF_OT_df_deselect_dfrs_in_dfr_layer,
            DF_OT_df_add_dfr_layer,
            DF_OT_df_remove_dfr_layer,
            DF_OT_df_add_dfr_layer_dfc_layer,
            DF_OT_df_remove_dfr_layer_dfc_layer,
            DF_OT_df_make_df_cache_dir_rel]
           

#Register
def register():

    if ((current_platform == "win32") or
    	(current_platform == "linux") or
    	(current_platform == "linux2") or
        (current_platform == "darwin")):
    
        for cls in classes:
        
            bpy.utils.register_class(cls)
            
        bpy.app.handlers.depsgraph_update_post.append(df_depsgraph_update_post_handler)
        bpy.app.handlers.frame_change_post.append(frame_change_post_handler)
        bpy.app.handlers.load_pre.append(df_load_pre_handler)
        bpy.app.handlers.load_post.append(df_load_post_handler)
        bpy.app.timers.register(periodical_volume_check, persistent = True)
        bpy.app.handlers.save_post.append(df_save_post_handler)
        bpy.app.handlers.save_pre.append(df_save_pre_handler)
        


#Unregister
def unregister():

    if ((current_platform == "win32") or
    	(current_platform == "linux") or
    	(current_platform == "linux2") or
        (current_platform == "darwin")):
    
        del bpy.types.Object.dfc_id
    
        for cls in classes:
        
            bpy.utils.unregister_class(cls)
            
        bpy.app.handlers.depsgraph_update_post.remove(df_depsgraph_update_post_handler)
        bpy.app.handlers.frame_change_post.remove(frame_change_post_handler)
       
        bpy.app.handlers.load_pre.remove(df_load_pre_handler)
        bpy.app.handlers.load_post.remove(df_load_post_handler)
        bpy.app.timers.unregister(periodical_volume_check)
        bpy.app.handlers.save_post.remove(df_save_post_handler)
        bpy.app.handlers.save_pre.remove(df_save_pre_handler)
    