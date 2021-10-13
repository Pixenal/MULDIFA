# Copyright (C) 2021 Pixenal \
	\
	\
	This file is part of MULDIFA. \
	\
	blend_df_tool is free software: you can redistribute it and/or modify \
	it under the terms of the GNU General Public License as published by \
	the Free Software Foundation, either version 3 of the License, or \
	(at your option) any later version. \
	\
	blend_df_tool is distributed in the hope that it will be useful, \
	but WITHOUT ANY WARRANTY; without even the implied warranty of \
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
	GNU General Public License for more details. \
	\
	You should have received a copy of the GNU General Public License \
	along with blend_df_tool.  If not, see <https://www.gnu.org/licenses/>. \


DF_COMMON_PATH = ../df_common
INTERMEDIATES_PATH = ../intermediates
INTERMEDIATES_SUBPATH = $(INTERMEDIATES_PATH)/df_linux
LIB_PATH = ../lib
TARGET_PATH = $(LIB_PATH)/df_linux
GOAL = $(TARGET_PATH)/df_linux.so
COMPILER = clang++
COMP_ARGS = $(COMPILER) -D LINUX -D EXPORT="" -std=c++2a -stdlib=libc++ -ffast-math -O3 -DNDEBUG -fPIC -c
LINK_ARGS = $(COMPILER) -shared -pthread
OBJECTS = $(INTERMEDIATES_SUBPATH)/df_lib.o $(INTERMEDIATES_SUBPATH)/df_loader.o $(INTERMEDIATES_SUBPATH)/df_regions_buffer.o $(INTERMEDIATES_SUBPATH)/df_writer.o \
$(INTERMEDIATES_SUBPATH)/distance_field.o $(INTERMEDIATES_SUBPATH)/shared.o $(INTERMEDIATES_SUBPATH)/thread_pool.o $(INTERMEDIATES_SUBPATH)/deflate.o \
$(INTERMEDIATES_SUBPATH)/png_encoder.o

#PHONY TARGETS
.PHONY : validate_dir clean

#BINARY
$(GOAL) : validate_dir $(OBJECTS)
	$(LINK_ARGS) -o $(GOAL) $(OBJECTS)

#VALIDATE DIR
validate_dir:
	-mkdir "$(INTERMEDIATES_PATH)"
	-mkdir "$(INTERMEDIATES_SUBPATH)"
	-mkdir "$(LIB_PATH)"
	-mkdir "$(TARGET_PATH)"

#INTERMEDIATES
$(INTERMEDIATES_SUBPATH)/df_lib.o : $(DF_COMMON_PATH)/df_lib.cpp $(DF_COMMON_PATH)/df_lib.h $(DF_COMMON_PATH)/thread_pool.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/df_lib.cpp -o $(INTERMEDIATES_SUBPATH)/df_lib.o

$(INTERMEDIATES_SUBPATH)/df_loader.o : $(DF_COMMON_PATH)/df_loader.cpp $(DF_COMMON_PATH)/df_lib.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/df_loader.cpp -o $(INTERMEDIATES_SUBPATH)/df_loader.o

$(INTERMEDIATES_SUBPATH)/df_regions_buffer.o : $(DF_COMMON_PATH)/df_regions_buffer.cpp $(DF_COMMON_PATH)/df_lib.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/df_regions_buffer.cpp -o $(INTERMEDIATES_SUBPATH)/df_regions_buffer.o

$(INTERMEDIATES_SUBPATH)/df_writer.o : $(DF_COMMON_PATH)/df_writer.cpp $(DF_COMMON_PATH)/df_lib.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/df_writer.cpp -o $(INTERMEDIATES_SUBPATH)/df_writer.o

$(INTERMEDIATES_SUBPATH)/distance_field.o : $(DF_COMMON_PATH)/distance_field.cpp $(DF_COMMON_PATH)/df_lib.h $(DF_COMMON_PATH)/thread_pool.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/distance_field.cpp -o $(INTERMEDIATES_SUBPATH)/distance_field.o

$(INTERMEDIATES_SUBPATH)/shared.o : $(DF_COMMON_PATH)/shared.cpp $(DF_COMMON_PATH)/df_lib.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/shared.cpp -o $(INTERMEDIATES_SUBPATH)/shared.o

$(INTERMEDIATES_SUBPATH)/thread_pool.o : $(DF_COMMON_PATH)/thread_pool.cpp $(DF_COMMON_PATH)/thread_pool.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/thread_pool.cpp -o $(INTERMEDIATES_SUBPATH)/thread_pool.o

$(INTERMEDIATES_SUBPATH)/deflate.o : $(DF_COMMON_PATH)/deflate.cpp $(DF_COMMON_PATH)/deflate.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/deflate.cpp -o $(INTERMEDIATES_SUBPATH)/deflate.o

$(INTERMEDIATES_SUBPATH)/png_encoder.o : $(DF_COMMON_PATH)/png_encoder.cpp $(DF_COMMON_PATH)/png_encoder.h
	$(COMP_ARGS) $(DF_COMMON_PATH)/png_encoder.cpp -o $(INTERMEDIATES_SUBPATH)/png_encoder.o

#CLEAN
clean : 
	rm $(GOAL) $(OBJECTS)
