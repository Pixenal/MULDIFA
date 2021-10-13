""" 
    Copyright (C) 2021 Pixenal


    This file is part of MULDIFA.

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
"""


bl_info = {
            "name": "MULDIFA",
            "description" : "A distance field implementation",
            "author": "Pixenal",
            "version": (1, 0),
            "blender": (2, 93, 0),
            "location": "View3D > Sidebar > MULDIFA",
            "category": "3D View"
}

import importlib

if ("bpy" in locals()):

    importlib.reload(df_props)
    importlib.reload(df_ops)
    importlib.reload(df_ui)
    
else:

    from . import df_props
    from . import df_ops
    from . import df_ui
    
import bpy


#Register
def register():

    df_props.register()
    df_ops.register()
    df_ui.register()

#Unregister
def unregister():

    df_props.unregister()
    df_ops.unregister()
    df_ui.unregister()