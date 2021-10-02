/*
 -------------------------------------------------------------------------------------------------------------
	Copyright(C) 2021 Pixenal


	This file is part of MULDIFA.

	blend_df_tool is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	blend_df_tool is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with blend_df_tool.If not, see < https://www.gnu.org/licenses/>.
	-------------------------------------------------------------------------------------------------------------
*/


#include "../df_common/df_lib.h"

extern shared_type shared;


/*df_type::df_loader_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*	The constructor for "df_loader_type" has 2 overloads, the first is used when the intent is to load from a file on disk, and the second is
	used when the intent is to load from a "regions_buffer_type" object in memory.	Other than some misc statements, the primary purpose of
	these constructors is to setup various data members in the current "df_loader_type" objects nested class type data members (such as 
	setting pointers that point to the current "df_loader_type" object so the member functions located within said data members can navigate
	around their enclosing object)	*/

df_type::df_loader_type::df_loader_type(const std::string& dir, const std::string& file_name, volume_local_type& volume_local, update_local_type& update_local, df_type& df)
{
	file_mode = true;

	file = std::ifstream(dir + file_name, std::ios::binary);
	reader = reader_type(file);

	director.set_file_mode(file_mode);
	reader.set_file_mode(file_mode);
	bin_interpretor.set_file_mode(file_mode);

	director.set_df_loader_ptr(*this);
	bin_interpretor.set_df_loader_ptr(*this);
	df_ptr = &df;

	volume_local_ptr = &volume_local;
	update_local_ptr = &update_local;
}


df_type::df_loader_type::df_loader_type(regions_buffer_type& regions_buffer, volume_local_type& volume_local, update_local_type& update_local, df_type& df)
{
	file_mode = false;
	director.set_file_mode(file_mode);
	reader.set_file_mode(file_mode);
	bin_interpretor.set_file_mode(file_mode);

	director.set_df_loader_ptr(*this);
	bin_interpretor.set_df_loader_ptr(*this);
	df_ptr = &df;
	reader.set_df_ptr(df);

	/*	Gets virtual stream	object (this is needed as the "df_loader_type" class is designed to interface with a file (that is to say,
		an output stream), and so in order for it to be able to read from a binary file from memory, a virtual stream class is used which
		emulates the output stream one would encounter when reading from disk)	*/

	reader.get_virtual_stream();

	volume_local_ptr = &volume_local;
	update_local_ptr = &update_local;
	regions_buffer_ptr = &regions_buffer;
}


df_type::df_loader_type::~df_loader_type()
{
	if (file_mode == true)
	{
		file.close();
	}
}


/*df_type::df_loader_type::director_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Sets pointer used for navigating enclosing class	*/
void df_type::df_loader_type::director_type::set_df_loader_ptr(df_loader_type& df_loader)
{
	df_loader_ptr = &df_loader;
	macro_regions_buffer.set_df_loader_ptr(df_loader);
}


void df_type::df_loader_type::director_type::call_volume_local_load()
{
	df_loader_ptr->volume_local_ptr->load_from_buffer(macro_regions_buffer.volume_local, macro_regions_buffer.update_local.dfc_ids);
}


void df_type::df_loader_type::director_type::call_update_local_load()
{
	df_loader_ptr->update_local_ptr->load_from_buffer(macro_regions_buffer.update_local);
}


/*	Loads data to buffer from output stream (from "file"), in practice this is either loading from a file on disk, or from memory with the
	assistance of a virtual stream, depending on the value of "file_mode"	*/
void df_type::df_loader_type::director_type::load_to_buffer()
{
	macro_regions_buffer.buffer_grid_is_valid();
	macro_regions_buffer.buffer_write_id();
	macro_regions_buffer.buffer_grid_min_coord();
	macro_regions_buffer.buffer_grid_space();
	macro_regions_buffer.buffer_df_distance();
	macro_regions_buffer.buffer_cmprt_size();
	macro_regions_buffer.buffer_volume_verts();
	macro_regions_buffer.buffer_grid_point_amount();
	macro_regions_buffer.buffer_max_grid_point();
	macro_regions_buffer.buffer_dfc_amount();
	macro_regions_buffer.buffer_dfc_ids();
	macro_regions_buffer.buffer_grid_points();
	macro_regions_buffer.buffer_cmprt_data();
	macro_regions_buffer.buffer_dfc_cache_is_valid();
	macro_regions_buffer.buffer_vert_amount_total();
	macro_regions_buffer.buffer_vert_cache_next_index();
	macro_regions_buffer.buffer_vert_cache();
	macro_regions_buffer.buffer_tri_cache_size();
	macro_regions_buffer.buffer_tri_cache();
	macro_regions_buffer.buffer_mesh_cache();
	macro_regions_buffer.buffer_existing_dfcs();
	macro_regions_buffer.buffer_dfc_id_index_table();
	macro_regions_buffer.buffer_dfr_ids();
	macro_regions_buffer.buffer_dfc_layers();
	macro_regions_buffer.buffer_dfr_layers();
}


/*	Loads the data current stored within the buffer into the state of the current "df_type" object (this would obviously be called after
	"load to buffer" is called)	*/
void df_type::df_loader_type::director_type::load_to_df_state()
{
	df_loader_ptr->df_ptr->write_index = macro_regions_buffer.write_id.index;
	df_loader_ptr->df_ptr->dfc_layers = macro_regions_buffer.update_local.dfc_layers;
	df_loader_ptr->df_ptr->dfr_layers = macro_regions_buffer.update_local.dfr_layers;
	call_volume_local_load();
	call_update_local_load();
}


void df_type::df_loader_type::director_type::set_file_mode(bool file_mode)
{
	this->file_mode = file_mode;
}


/*df_type::df_loader_type::director_type::buffers_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*	Reads, and returns, a single unsigned short value from the output stream (from "file")	*/
unsigned short df_type::df_loader_type::director_type::buffers_type::get_ushort()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as an unsigned short */
	return df_loader_ptr->bin_interpretor.intrp_as_ushort();
}


/*	Reads, and returns, a unsigned short value from the output stream (from "file"), reading only the number of bits specified*/
unsigned short df_type::df_loader_type::director_type::buffers_type::get_ushort(const unsigned short bit_size)
{
	/*	Reads value (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_spec_bits(bit_size);
	/*	Interprets the resulting byte string as an unsigned short */
	return df_loader_ptr->bin_interpretor.intrp_as_ushort();
}


/*	Reads, and returns, a single unsigned long value from the output stream (from "file")	*/
unsigned long df_type::df_loader_type::director_type::buffers_type::get_ulong()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as an unsigned long */
	return df_loader_ptr->bin_interpretor.intrp_as_ulong();
}


/*	Reads, and returns, a unsigned long value from the output stream (from "file"), reading only the number of bits specified*/
unsigned long df_type::df_loader_type::director_type::buffers_type::get_ulong(const unsigned short bit_size)
{
	/*	Reads value (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_spec_bits(bit_size);
	/*	Interprets the resulting byte string as an unsigned long */
	return df_loader_ptr->bin_interpretor.intrp_as_ulong();
}


/*	Reads, and returns, a single unsigned long long value from the output stream (from "file")	*/
unsigned long long df_type::df_loader_type::director_type::buffers_type::get_ullong()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as an unsigned long long*/
	return df_loader_ptr->bin_interpretor.intrp_as_ullong();
}


/*	Reads, and returns, a single float value from the output stream (from "file")	*/
float df_type::df_loader_type::director_type::buffers_type::get_float()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as a float */
	return df_loader_ptr->bin_interpretor.intrp_as_float();
}


/*	Reads, and returns, a single double value from the output stream (from "file")	*/
double df_type::df_loader_type::director_type::buffers_type::get_double()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as a double */
	return df_loader_ptr->bin_interpretor.intrp_as_double();
}


/*	Reads, and returns, a single boolean value from the output stream (from "file"), reading only a single bit*/
bool df_type::df_loader_type::director_type::buffers_type::get_bool()
{
	/*	Reads a single bit (unlike other related functions "read_bit" does return a value, that being the value of the bit
		represented as a bool)	*/
	return df_loader_ptr->reader.read_bit();
}


/*	Reads, and returns, a single boolean value from the output stream (from "file"), reading a full byte	*/
bool df_type::df_loader_type::director_type::buffers_type::get_bool_byte()
{
	/*	Reads value (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_spec_bits(8u);
	/*	Interprets the resulting byte string as a unsigned long, then casts it to a bool	*/
	return (bool)df_loader_ptr->bin_interpretor.intrp_as_ulong();
}


/*	Reads, and returns, a single int value from the output stream (from "file")	*/
int df_type::df_loader_type::director_type::buffers_type::get_int()
{
	/*	Reads sv format (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)) (the result of the below function is stored in a buffer data member, so no return value)	*/
	df_loader_ptr->reader.read_sv();
	/*	Interprets the resulting byte string as an int	*/
	return df_loader_ptr->bin_interpretor.intrp_as_int();
}


void df_type::df_loader_type::director_type::buffers_type::set_df_loader_ptr(df_loader_type& df_loader)
{
	df_loader_ptr = &df_loader;
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_grid_is_valid()
{
	volume_local.grid_is_valid = get_bool();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_write_id()
{
	write_id.index = get_int();
	write_id.rand = get_int();
	df_loader_ptr->director.write_id_ptr = &write_id;
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the grid is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the grid is not valid,
	then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_grid_min_coord()
{
	if (volume_local.grid_is_valid == true)
	{
		volume_local.grid_min_coord.x = get_double();
		volume_local.grid_min_coord.y = get_double();
		volume_local.grid_min_coord.z = get_double();
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_grid_space()
{
	volume_local.grid_spacing = get_double();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_df_distance()
{
	volume_local.df_distance = get_float();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_cmprt_size()
{
	volume_local.cmprt_size = get_ushort();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_volume_verts()
{
	for (unsigned short a = 0u; a < 8u; ++a)
	{
		volume_local.volume_verts[a].x = get_double();
		volume_local.volume_verts[a].y = get_double();
		volume_local.volume_verts[a].z = get_double();
	}

	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the grid is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the grid is not valid,
	then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_grid_point_amount()
{
	if (volume_local.grid_is_valid == true)
	{
		volume_local.grid_point_amount.x = get_ushort();
		volume_local.grid_point_amount.y = get_ushort();
		volume_local.grid_point_amount.z = get_ushort();
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}

	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the grid is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the grid is not valid,
	then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_max_grid_point()
{
	if (volume_local.grid_is_valid == true)
	{
		volume_local.max_grid_point.x = get_ushort();
		volume_local.max_grid_point.y = get_ushort();
		volume_local.max_grid_point.z = get_ushort();
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	This function is differs quite a bit from other buffer functions, in that the format used is not just a standard sv format,
	being instead a bit more complicated (though not by much). For size optimization purposes, the data that represents the grid
	is split into 2 regions, "grid_meta" and "grid_values". Grid meta contains information, individually for each grid point on
	how the the data stored in in "grid_values" is formated (obviously, said information in "grid_meta" is also used to convey
	info in and of itself, such as number of layers). Each grid points entry in "grid_meta" is layed out as follows:
	- 1st bit flag is 1 if the grid point has more than 0 layers, else it's 0. If 0, there are no more bits for the current grid point
	- 2nd bit flag is 1 if there is more than 1 layer, else it's 0. If 0, there are no more bits for the current grid point	
	- 3rd bit flag is 1 if the number of bytes used to represent the number of layers is 2, if 0, then is 1 byte
	- The next 1 - 2 byte(s) represent the number of layers	in the current grid point	

	If the grid is not valid, the region is skipped (see "skip_region"'s description for more info on that function's purpose), as if
	the grid is not valid, then this region should have been skipped on write (this is assumed to be the case).	
	
	Keep in mind that the layout of the grid_points buffer differs from the way in which the grid points are layed out in df state.
	See definition of "grid_points_type" in "volume_local_type"s' definition for more info	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_grid_points()
{
	if (volume_local.grid_is_valid == true)
	{
		unsigned long grid_amount_total = volume_local.grid_point_amount.x * volume_local.grid_point_amount.y * volume_local.grid_point_amount.z;

		unsigned long grid_point_next_index = 0u;
		for (unsigned long a = 0u; a < grid_amount_total; ++a)
		{
			bool flag_0 = get_bool();

			/*	If number of grid points > 0, continue reading, else move on to next grid point	*/
			if (flag_0 == true)
			{
				volume_local.grid_points.grid_point_indices.push_back(a);

				bool flag_1 = get_bool();

				unsigned long layer_amount = 1u;

				/*	If the number of layers > 1, continue reading, else set layer amount in buffer to 1 and move on to next grid point	*/
				if (flag_1 == true)
				{
					bool flag_2 = get_bool();

					/*	Read 2 bytes if the 3rd bit flag is true, otherwise read only 1 byte	*/
					if (flag_2 == true)
					{
						layer_amount = get_ulong(16u);
					}
					else
					{
						layer_amount = get_ulong(6u);
					}
				}

				volume_local.grid_points.grid_point_meta.push_back(layer_amount);
			}
		}

		/*	Now that the "grid_meta" region has been read, the "clear_byte_buffer" must be called, as we're reading 2 regions within
			a single function	*/
		df_loader_ptr->reader.clear_byte_buffer();

		/*	Keep in mind that the buffer only stores the values of grid points that have layers	*/
		unsigned long non_empty_grid_point_amount = volume_local.grid_points.grid_point_meta.size();
		volume_local.grid_points.grid_point_values = new df_type::mesh_layer_type * [non_empty_grid_point_amount];

		/*	If no grid points have layers, skip region (see "skip_region"'s description for more info on that function's purpose)	*/
		if (non_empty_grid_point_amount == 0)
		{
			df_loader_ptr->reader.skip_region();
		}

		/*	Loops through non empty grid point	*/
		for (unsigned long a = 0u; a < non_empty_grid_point_amount; ++a)
		{
			/*	Dynamically allocates new array of "mesh_layer_type" objects	*/
			volume_local.grid_points.grid_point_values[a] = new df_type::mesh_layer_type[volume_local.grid_points.grid_point_meta[a]];

			/*	Loops through each layer in the current grid point	*/
			for (unsigned long b = 0u; b < volume_local.grid_points.grid_point_meta[a]; ++b)
			{
				/*	Reads value of current layer from output stream (from "file"), and converted the integral value back into a
					floating point value with norm 1 (the value was stored as a short for size optimization)	*/
				float value = (float)get_ushort(6u) / 63.0f;
				volume_local.grid_points.grid_point_values[a][b].value = value;

				/*	Reads dfc_id from output stream (from "file"), then finds the dfc_id's entry in the dfc_id region, and sets the
				dfc_id data member in the current grid point layers entry in the buffer to equal the dfc_id's entry's memory address	*/
				unsigned long dfc_id = get_ulong();
				update_local.dfc_ids.calc_size();
				for (unsigned long c = 0u; c < update_local.dfc_ids.size; ++c)
				{
					if (update_local.dfc_ids.vector[c]->id == dfc_id)
					{
						volume_local.grid_points.grid_point_values[a][b].mesh_indx.dfc_id = &update_local.dfc_ids.vector[c]->id;
					}
				}

				/*	Reads "mesh_index" and "mesh_layer_table_index" from output stream (from "file")	*/
				volume_local.grid_points.grid_point_values[a][b].mesh_indx.mesh_index = get_ulong();
				volume_local.grid_points.grid_point_values[a][b].mesh_layer_table_index = get_ulong();
			}
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file"). Keep in mind that the layout of the "cmprt_data" region in the buffer differs
	from the layout of the compartments in the df_state (see the definition of "cmprt_data_type" in "volume_local_type"'s definition
	for more info)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_cmprt_data()
{
	if (volume_local.grid_is_valid == true)
	{
		volume_local.cmprt_data.cmprt_amount.x = get_ushort();
		volume_local.cmprt_data.cmprt_amount.y = get_ushort();
		volume_local.cmprt_data.cmprt_amount.z = get_ushort();

		unsigned long cmprt_amount_total = volume_local.cmprt_data.cmprt_amount.x * volume_local.cmprt_data.cmprt_amount.y * volume_local.cmprt_data.cmprt_amount.z;
		volume_local.cmprt_data.cmprts = new df_type::volume_local_type::cmprt_data_type::cmprt_type[cmprt_amount_total];
		for (unsigned long a = 0u; a < cmprt_amount_total; ++a)
		{
			volume_local.cmprt_data.cmprts[a].size = get_ulong();
			volume_local.cmprt_data.cmprts[a].grid_points = new unsigned long[volume_local.cmprt_data.cmprts[a].size];
			for (unsigned long b = 0u; b < volume_local.cmprt_data.cmprts[a].size; ++b)
			{
				volume_local.cmprt_data.cmprts[a].grid_points[b] = get_ulong();
			}

			volume_local.cmprt_data.cmprts[a].rlvncy_table_size = get_ulong();
			volume_local.cmprt_data.cmprts[a].rlvncy_table = new unsigned long[volume_local.cmprt_data.cmprts[a].rlvncy_table_size];
			for (unsigned long b = 0u; b < volume_local.cmprt_data.cmprts[a].rlvncy_table_size; ++b)
			{
				volume_local.cmprt_data.cmprts[a].rlvncy_table[b] = get_ulong();
			}
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfc_amount()
{
	update_local.dfc_amount = get_ulong();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfc_ids()
{
	unsigned long size = get_ulong();
	for (unsigned long a = 0u; a < size; ++a)
	{
		update_local.dfc_ids.vector.push_back(new dfc_id_indx_type());

		update_local.dfc_ids.vector[a]->id = get_ulong();
		update_local.dfc_ids.vector[a]->index = get_ulong();
	}

	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfc_cache_is_valid()
{
	update_local.dfc_cache_is_valid = get_bool_byte();
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_vert_amount_total()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.vert_amount_total = get_ulong();
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_vert_cache_next_index()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.vert_cache_next_index = get_ulong();
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_vert_cache()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.vert_cache = new shared_type::vert_tri_ref_type[update_local.vert_amount_total];
		for (unsigned long a = 0u; a < update_local.vert_amount_total; ++a)
		{
			update_local.vert_cache[a].dfc_index = get_double();

			update_local.vert_cache[a].coord.x = get_double();
			update_local.vert_cache[a].coord.y = get_double();
			update_local.vert_cache[a].coord.z = get_double();
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_tri_cache_size()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.tri_cache_size = new unsigned long[update_local.dfc_amount];
		for (unsigned long a = 0u; a < update_local.dfc_amount; ++a)
		{
			update_local.tri_cache_size[a] = get_ulong();
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_tri_cache()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.tri_cache = new shared_type::tri_info_type * [update_local.dfc_amount];
		for (unsigned long a = 0u; a < update_local.dfc_amount; ++a)
		{
			update_local.tri_cache[a] = new shared_type::tri_info_type[update_local.tri_cache_size[a]];
			for (unsigned long b = 0u; b < update_local.tri_cache_size[a]; ++b)
			{
				update_local.tri_cache[a][b].vert_0 = get_ulong();
				update_local.tri_cache[a][b].vert_1 = get_ulong();
				update_local.tri_cache[a][b].vert_2 = get_ulong();
			}
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_mesh_cache()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.mesh_cache = new std::vector<df_type::update_local_type::dfc_cache_type::mesh_info_type*>*[update_local.dfc_amount];
		for (unsigned long a = 0u; a < update_local.dfc_amount; ++a)
		{
			update_local.mesh_cache[a] = new std::vector<df_type::update_local_type::dfc_cache_type::mesh_info_type*>;
			unsigned long mesh_size = get_ulong();
			for (unsigned long b = 0u; b < mesh_size; ++b)
			{
				update_local.mesh_cache[a]->push_back(new df_type::update_local_type::dfc_cache_type::mesh_info_type);

				unsigned long tris_size = get_ulong();
				for (unsigned long c = 0u; c < tris_size; ++c)
				{
					(*update_local.mesh_cache[a])[b]->tris.push_back(get_ulong());
				}

				unsigned long layer_indices_size = get_ulong();
				(*update_local.mesh_cache[a])[b]->layer_indices = new std::vector<shared_type::index_xyzw_type>;
				for (unsigned long c = 0u; c < layer_indices_size; ++c)
				{
					(*update_local.mesh_cache[a])[b]->layer_indices->push_back(shared_type::index_xyzw_type());

					unsigned long long linear_index = get_ullong();
					unsigned long grid_points_xy_amount = volume_local.grid_point_amount.z * volume_local.grid_point_amount.y;
					(*(*update_local.mesh_cache[a])[b]->layer_indices)[c].x = linear_index / grid_points_xy_amount;
					unsigned long grid_points_xy_amount_mx = grid_points_xy_amount * (*(*update_local.mesh_cache[a])[b]->layer_indices)[c].x;
					(*(*update_local.mesh_cache[a])[b]->layer_indices)[c].y = (linear_index - grid_points_xy_amount_mx) / volume_local.grid_point_amount.z;
					(*(*update_local.mesh_cache[a])[b]->layer_indices)[c].z = (linear_index - grid_points_xy_amount_mx) - (volume_local.grid_point_amount.z * (*(*update_local.mesh_cache[a])[b]->layer_indices)[c].y);

					(*(*update_local.mesh_cache[a])[b]->layer_indices)[c].w = get_ulong();
				}

				(*update_local.mesh_cache[a])[b]->moved_from_legacy = get_bool();
			}
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	If the dfc_cache is valid, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if the
	dfc_cache is not valid, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_existing_dfcs()
{
	if (update_local.dfc_cache_is_valid == true)
	{
		update_local.existing_dfcs = new bool[update_local.dfc_amount];
		for (unsigned long a = 0u; a < update_local.dfc_amount; ++a)
		{
			update_local.existing_dfcs[a] = get_bool_byte();
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}

/*	If the dfc_amount > 0, reads region from output stream (from "file") and loads it into the buffer,
	else skips region (see "skip_region"'s description for more info on that function's purpose) as if
	dfc_amount == 0, then this region should have been skipped on write (this is assumed to be the case)	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfc_id_index_table()
{
	if (update_local.dfc_amount > 0u)
	{
		update_local.dfc_id_index_table = new dfc_id_indx_type * [update_local.dfc_amount];
		for (unsigned long a = 0u; a < update_local.dfc_amount; ++a)
		{
			unsigned long dfc_id = get_ulong();
			/*	Searches dfc_ids to find the the matching id entry, and sets the current
				entry in the indx to id table to equal the pointer to said dfc id entry	*/
			update_local.dfc_ids.calc_size();
			for (unsigned long b = 0u; b < update_local.dfc_ids.size; ++b)
			{
				if (update_local.dfc_ids.vector[b]->id == dfc_id)
				{
					update_local.dfc_id_index_table[a] = update_local.dfc_ids.vector[b];
					break;
				}
			}
		}
	}
	else
	{
		df_loader_ptr->reader.skip_region();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfr_ids()
{
	unsigned long size = get_ulong();
	for (unsigned long a = 0u; a < size; ++a)
	{
		update_local.dfr_ids.vector.push_back(new dfr_id_indx_type());

		update_local.dfr_ids.vector[a]->id = get_ulong();
		update_local.dfr_ids.vector[a]->index = get_ulong();
	}
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfc_layers()
{
	unsigned long layers_size = get_ulong();
	for (unsigned long a = 0u; a < layers_size; ++a)
	{
		update_local.dfc_layers.push_back(shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>());
		unsigned long dfc_amount = get_ulong();
		for (unsigned long b = 0u; b < dfc_amount; ++b)
		{
			unsigned long dfc_id = get_ulong();
			update_local.dfc_ids.calc_size();
			for (unsigned long c = 0u; c < update_local.dfc_ids.size; ++c)
			{
				if (update_local.dfc_ids.vector[c]->id == dfc_id)
				{
					update_local.dfc_layers[a].vector.push_back(update_local.dfc_ids.vector[c]);
					break;
				}
			}
		}
	}
	df_loader_ptr->director.dfc_layers_ptr = &update_local.dfc_layers;
	df_loader_ptr->reader.clear_byte_buffer();
}


/*	Reads region from output stream (from "file") and loads it into the buffer	*/
void df_type::df_loader_type::director_type::buffers_type::buffer_dfr_layers()
{
	unsigned long layers_size = get_ulong();
	for (unsigned long a = 0u; a < layers_size; ++a)
	{
		update_local.dfr_layers.push_back(shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>());
		unsigned long dfr_amount = get_ulong();
		for (unsigned long b = 0u; b < dfr_amount; ++b)
		{
			unsigned long dfr_id = get_ulong();
			update_local.dfr_ids.calc_size();
			for (unsigned long c = 0u; c < update_local.dfr_ids.size; ++c)
			{
				if (update_local.dfr_ids.vector[c]->id == dfr_id)
				{
					update_local.dfr_layers[a].vector.push_back(update_local.dfr_ids.vector[c]);
					break;
				}
			}
		}
	}
	df_loader_ptr->director.dfr_layers_ptr = &update_local.dfr_layers;
	df_loader_ptr->reader.clear_byte_buffer();
}


/*df_type::df_loader_type::reader_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	If "next_bit_index" is greater than 8, increments, by 1, the byte being read from the output stream (from "file")	*/
void df_type::df_loader_type::reader_type::validate_byte_buffer()
{
	++next_bit_index;
	if (next_bit_index >= 8u)
	{
		iterate_byte_buffer();
	}
}


df_type::df_loader_type::reader_type::reader_type(std::ifstream& file)
{
	this->file = &file;
}


df_type::df_loader_type::reader_type::reader_type()
{
}

/*	Reads a specificed number of bits from the output stream (from "file"), and puts said read data into a buffer data member	*/
void df_type::df_loader_type::reader_type::read_spec_bits(const unsigned short bit_size)
{
	if (byte_buffer_occupied == false)
	{
		iterate_byte_buffer();
	}

	read_byte_vec_buffer.incrmt();

	for (unsigned long a = 0u; a < bit_size; ++a)
	{
		read_byte_vec_buffer.next_bit_index = a % 8;
		unsigned short return_buffer_byte_index = a / 8u;

		if (read_byte_vec_buffer.char_vec.size() <= return_buffer_byte_index)
		{
			read_byte_vec_buffer.incrmt();
		}

		/*	"next_bit_index" is a data_member that represents the index of the next bit to read from the current byte bring read from the
			output stream (from "file"), whereas "read_byte_vec_buffer.next_bit_index" represents the next bit to write to in the
			current	byte currently being written to in within "read_byte_vec_buffer"	*/
		read_byte_vec_buffer.byte_buffer[read_byte_vec_buffer.next_bit_index] = (int)byte_buffer[next_bit_index];

		validate_byte_buffer();
	}
	read_byte_vec_buffer.buffer_to_char_vec();
}


/*	Reads a single vaule from the output stream (from "file"), a single value (or sv) refers to the specific format used in the dfcache for storing individual values,
	the format consists of an initial 6 bits which states the number of bits used to represent the value (starting after said initial 6 bits), and the next n number of
	bits represent the actual value (essentially Type-Length-Value (TLV) encoding but without the type part (just the length and value)).	*/
void df_type::df_loader_type::reader_type::read_sv()
{
	if (byte_buffer_occupied == false)
	{
		iterate_byte_buffer();
	}

	/*The below for loop iterates through the first 8 bits in order to determine how many bits to read afterwards (the loop needs to read
	bit by bit instead of just taking the whole byte as the 8 bits used for the first component may not (and often does not) align with
	the bytes of the output stream)	*/

	std::bitset<8> size_bin_buffer;
	unsigned long bit_size = 0u;
	for (unsigned short a = 0u; a < shared.sv_meta_size; ++a)
	{
		size_bin_buffer[a] = byte_buffer[next_bit_index];

		++next_bit_index;
		if (next_bit_index >= 8u)
		{
			iterate_byte_buffer();
		}
	}
	bit_size = size_bin_buffer.to_ulong();

	/*	Now that it's length has been read, reads the value component of the sv	(Note that the result is stored within a buffer data member,
		this is why there's no return value from the below, or current, function (the "delete"))	*/
	read_spec_bits(bit_size);
}

/*	Reads a single bit from the output stream (from "file", this is used for reading boolean values)	*/
bool df_type::df_loader_type::reader_type::read_bit()
{
	if (byte_buffer_occupied == false)
	{
		iterate_byte_buffer();
	}

	bool return_buffer = (bool)byte_buffer[next_bit_index];

	validate_byte_buffer();

	return return_buffer;
}


/*	Iterates "byte_buffer", this means getting the next byte in the file output stream if "file_mode" == true, or getting the next
	byte from the virtual stream if false	*/
void df_type::df_loader_type::reader_type::iterate_byte_buffer()
{
	if (file_mode == true)
	{
		byte_buffer = std::bitset<8>((unsigned long long)file->get());
	}
	else
	{
		byte_buffer = virtual_stream.get_byte();
	}


	byte_buffer_occupied = true;
	next_bit_index = 0u;
}

/*	Increments byte currently being read from the output stream (from "file") if "next_bit_index" != 0, that is to say, if the current
	byte has been read from already when this function is called, then the current byte is incremented. This function
	is most commonly called after a region has finished being read, as while values within a region are usually packed (not aligned
	to the bytes of the output stream), the actuall regions themselves are, and as such when a region has been read, the byte buffer
	is incremented so as to ensure the next region begins at the start of the next byte	*/
void df_type::df_loader_type::reader_type::clear_byte_buffer()
{
	if (next_bit_index != 0u)
	{
		byte_buffer.reset();
		byte_buffer_occupied = false;
		next_bit_index = 0u;
	}
}

/*	This function is called when skipping a region, this function assumes the region was was skipped on write, and as such the byte buffer
	must only be incremented once (as if the region was skipped on write, it would only take up a single byte)	*/
void df_type::df_loader_type::reader_type::skip_region()
{
	if (byte_buffer_occupied == true)
	{
		iterate_byte_buffer();
	}
	else
	{
		iterate_byte_buffer();
		byte_buffer_occupied = false;
	}
}


void df_type::df_loader_type::reader_type::set_file_mode(bool file_mode)
{
	this->file_mode = file_mode;
}


void df_type::df_loader_type::reader_type::set_df_ptr(df_type& df)
{
	df_ptr = &df;
}


void df_type::df_loader_type::reader_type::get_virtual_stream()
{
	virtual_stream = df_ptr->regions_buffer.get_virtual_stream();
}


/*df_type::df_loader_type::bin_interpretor_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as an unsigned short	*/
unsigned short df_type::df_loader_type::bin_interpretor_type::intrp_as_ushort()
{
	return (unsigned short)intrp_as_ulong();
}

/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as an unsigned long	*/
unsigned long df_type::df_loader_type::bin_interpretor_type::intrp_as_ulong()
{
	std::bitset<32> bin_buffer;

	unsigned short byte_vec_size = df_loader_ptr->reader.read_byte_vec_buffer.char_vec.size();
	for (unsigned short a = 0u; (a < 4u) && (a < byte_vec_size); ++a)
	{
		df_loader_ptr->reader.read_byte_vec_buffer.buffer_char(a);
		for (unsigned short b = 0u; b < 8u; ++b)
		{
			unsigned short bin_buffer_index = (a * 8) + b;
			bin_buffer[bin_buffer_index] = (int)df_loader_ptr->reader.read_byte_vec_buffer.byte_buffer[b];
		}
	}

	df_loader_ptr->reader.read_byte_vec_buffer.clean();
	return bin_buffer.to_ulong();
}

/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as an unsigned long long	*/
unsigned long long df_type::df_loader_type::bin_interpretor_type::intrp_as_ullong()
{
	std::bitset<64> bin_buffer;

	unsigned short byte_vec_size = df_loader_ptr->reader.read_byte_vec_buffer.char_vec.size();
	for (unsigned short a = 0u; (a < 8u) && (a < byte_vec_size); ++a)
	{
		df_loader_ptr->reader.read_byte_vec_buffer.buffer_char(a);
		for (unsigned short b = 0u; b < 8u; ++b)
		{
			unsigned short bin_buffer_index = (a * 8) + b;
			bin_buffer[bin_buffer_index] = (int)df_loader_ptr->reader.read_byte_vec_buffer.byte_buffer[b];
		}
	}

	df_loader_ptr->reader.read_byte_vec_buffer.clean();
	return bin_buffer.to_ullong();
}


/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as a float	*/
float df_type::df_loader_type::bin_interpretor_type::intrp_as_float()
{
	unsigned long long_buffer = intrp_as_ulong();
	shared.invert_bit_order(long_buffer);
	float return_value = .0f;
	std::memcpy(&return_value, &long_buffer, 4);

	return return_value;
}


/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as a double	*/
double df_type::df_loader_type::bin_interpretor_type::intrp_as_double()
{
	unsigned long long long_long_buffer = intrp_as_ullong();
	shared.invert_bit_order(long_long_buffer);
	double return_value = .0;
	std::memcpy(&return_value, &long_long_buffer, 8);

	return return_value;
}


/*	Interprets the byte string currently stored within the "byte_vec_buffer" data member as an int	*/
int df_type::df_loader_type::bin_interpretor_type::intrp_as_int()
{
	unsigned long long long_long_buffer = intrp_as_ullong();
	int return_value = 0u;
	std::memcpy(&return_value, &long_long_buffer, 4);

	return return_value;
}


void df_type::df_loader_type::bin_interpretor_type::set_file_mode(bool file_mode)
{
	this->file_mode = file_mode;
}


void df_type::df_loader_type::bin_interpretor_type::set_df_loader_ptr(df_loader_type& df_loader_ptr)
{
	this->df_loader_ptr = &df_loader_ptr;
}