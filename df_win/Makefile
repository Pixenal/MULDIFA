DF_SHARED_PATH = ../df_shared
INTERMEDIATES_PATH = ../intermediates/df_win
TARGET_PATH = ../lib/df_win
GOAL = $(TARGET_PATH)/df_win.dll
COMPILER = clang++
COMP_ARGS = $(COMPILER) -D WINDOWS -D PCH_INCLUDE="\"../df_shared/dummy.h\"" -D EXPORT=__declspec(dllexport) -std=c++2a -ffast-math -O3 -DNDEBUG -flto -c
LINK_ARGS = $(COMPILER) -fuse-ld=lld -shared -flto -pthread
OBJECTS = $(INTERMEDIATES_PATH)/df_lib.o $(INTERMEDIATES_PATH)/df_loader.o $(INTERMEDIATES_PATH)/df_regions_buffer.o $(INTERMEDIATES_PATH)/df_writer.o \
$(INTERMEDIATES_PATH)/distance_field.o $(INTERMEDIATES_PATH)/shared.o $(INTERMEDIATES_PATH)/thread_pool.o


#BINARY
$(GOAL) : $(OBJECTS)
	$(LINK_ARGS) -o $(GOAL) $(OBJECTS)


#INTERMEDIATES
$(INTERMEDIATES_PATH)/df_lib.o : $(DF_SHARED_PATH)/df_lib.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_lib.cpp -o $(INTERMEDIATES_PATH)/df_lib.o

$(INTERMEDIATES_PATH)/df_loader.o : $(DF_SHARED_PATH)/df_loader.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_loader.cpp -o $(INTERMEDIATES_PATH)/df_loader.o

$(INTERMEDIATES_PATH)/df_regions_buffer.o : $(DF_SHARED_PATH)/df_regions_buffer.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_regions_buffer.cpp -o $(INTERMEDIATES_PATH)/df_regions_buffer.o

$(INTERMEDIATES_PATH)/df_writer.o : $(DF_SHARED_PATH)/df_writer.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/df_writer.cpp -o $(INTERMEDIATES_PATH)/df_writer.o

$(INTERMEDIATES_PATH)/distance_field.o : $(DF_SHARED_PATH)/distance_field.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/distance_field.cpp -o $(INTERMEDIATES_PATH)/distance_field.o

$(INTERMEDIATES_PATH)/shared.o : $(DF_SHARED_PATH)/shared.cpp $(DF_SHARED_PATH)/df_lib.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/shared.cpp -o $(INTERMEDIATES_PATH)/shared.o

$(INTERMEDIATES_PATH)/thread_pool.o : $(DF_SHARED_PATH)/thread_pool.cpp $(DF_SHARED_PATH)/thread_pool.h $(DF_SHARED_PATH)/dummy.h
	$(COMP_ARGS) $(DF_SHARED_PATH)/thread_pool.cpp -o $(INTERMEDIATES_PATH)/thread_pool.o
	
$(INTERMEDIATES_PATH)/dllmain.o : dllmain.cpp
	$(COMP_ARGS) dllmain.cpp -o $(INTERMEDIATES_PATH)/dllmain.o


#PHONY TARGETS
.PHONY : clean

clean : 
	rm $(GOAL) $(OBJECTS)
