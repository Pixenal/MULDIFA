## This project is archived.
The addon was a personal exercise in C++/ algorithms/ etc, and the code is not in a state I would consider maintainable.

# MULDIFA

MULDIFA is a distance field addon for Blender (2.83+).  
One can set which objects influence and sample from the distance field.  
MULDIFA is also multi-layered, meaning that one can control which influencing objects affect which sampling objects, allowing for isolation.  
Sampling is primarily done in the form of baking to a png texture, though there is also an option for writing to vertex colours or groups.

Notice:

This addon is not a rendering engine, and does not contain any distance field rendering functionality. The field is also not signed (there is no distinction made between the inside and outside of a mesh).  
The addon also only supports use within a single scene (I've not tested it in a multi-scene setup, so it may work depending on the use case, though I can't give any guarantees).

On top of this, the creation of this addon was first and foremost a learning exercise in programming. This is the first non-trivial software project I've attempted, and so it may contain some bugs.

Platforms:

MULDIFA supports use on 64bit versions of Windows, Linux, and Intel MacOS.
(I've not yet tested the addon on an ARM platform (such as M1 MacOS), so I can't guarantee that it will work on these systems).

Contact:

Feel free to contact me if you have any questions or issues regarding MULDIFA, my email is:

pixenal.contact@gmail.com
