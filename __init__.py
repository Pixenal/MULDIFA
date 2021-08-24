bl_info = {
            "name": "DF_Tool",
            "description" : "A distance field implementation",
            "author": "Pixenal",
            "bersion": (1, 0),
            "blender": (2, 91, 0),
            "location": "View3D > Sidebar > DF_Tool",
            "category": "3D View"
}

import importlib

if ("bpy" in locals()):

    importlib.reload(df_preop)
    importlib.reload(df_ops)
    importlib.reload(df_ui)
    
else:

    from . import df_props
    from . import df_ops
    from . import df_ui
    
import bpy


def register():

    df_props.register()
    df_ops.register()
    df_ui.register()
    
def unregister():

    df_props.unregister()
    df_ops.unregister()
    df_ui.unregister()