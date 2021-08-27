DF_SHARED_PATH = ../df_shared
INTERMEDIATES_PATH = ../intermediates
INTERMEDIATES_SUBPATH = $(INTERMEDIATES_PATH)/df_win
LIB_PATH = ../lib
TARGET_PATH = $(LIB_PATH)/df_win
GOAL = $(TARGET_PATH)/df_win.dll
COMPILER = clang++
COMP_ARGS = $(COMPILER) -D WINDOWS -D PCH_INCLUDE="\"../df_shared/dummy.h\"" -D EXPORT=__declspec(dllexport) -std=c++2a -stdlib=libc++ -ffast-math -O3 -DNDEBUG -flto -c
LINK_ARGS = $(COMPILER) -fuse-ld=lld -shared -flto
OBJECTS = $(INTERMEDIATES_SUBPATH)/df_lib.o $(INTERMEDIATES_SUBPATH)/df_loader.o $(INTERMEDIATES_SUBPATH)/df_regions_buffer.o $(INTERMEDIATES_SUBPATH)/df_writer.o \
$(INTERMEDIATES_SUBPATH)/distance_field.o $(INTERMEDIATES_SUBPATH)/shared.o $(INTERMEDIATES_SUBPATH)/thread_pool.o

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
$(INTERMEDIATES_SUBPATH)/df_lib.o : $(DF_SHARED_PATH)/df_lib.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_lib.cpp -o $(INTERMEDIATES_SUBPATH)/df_lib.o

$(INTERMEDIATES_SUBPATH)/df_loader.o : $(DF_SHARED_PATH)/df_loader.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_loader.cpp -o $(INTERMEDIATES_SUBPATH)/df_loader.o

$(INTERMEDIATES_SUBPATH)/df_regions_buffer.o : $(DF_SHARED_PATH)/df_regions_buffer.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_regions_buffer.cpp -o $(INTERMEDIATES_SUBPATH)/df_regions_buffer.o

$(INTERMEDIATES_SUBPATH)/df_writer.o : $(DF_SHARED_PATH)/df_writer.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_writer.cpp -o $(INTERMEDIATES_SUBPATH)/df_writer.o

$(INTERMEDIATES_SUBPATH)/distance_field.o : $(DF_SHARED_PATH)/distance_field.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/distance_field.cpp -o $(INTERMEDIATES_SUBPATH)/distance_field.o

$(INTERMEDIATES_SUBPATH)/shared.o : $(DF_SHARED_PATH)/shared.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/shared.cpp -o $(INTERMEDIATES_SUBPATH)/shared.o

$(INTERMEDIATES_SUBPATH)/thread_pool.o : $(DF_SHARED_PATH)/thread_pool.cpp $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/thread_pool.cpp -o $(INTERMEDIATES_SUBPATH)/thread_pool.o

#CLEAN
clean : 
	rm $(GOAL) $(OBJECTS)
