/*
 -------------------------------------------------------------------------------------------------------------
	Copyright(C) 2021 Pixenal


	This file is part of blend_df_tool.

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

unsigned long long funny_counter = 0u;


/*df_type::df_writer_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*	Sets "file" to equal a new ofstream object, and sets regions buffer pointer to point to the regions_buffer_type parameter*/
df_type::df_writer_type::df_writer_type(const std::string& file_dir, const std::string& file_name, regions_buffer_type& regions_buffer)
{
	file = std::ofstream(file_dir + file_name, std::ios::binary);
	this->regions_buffer = &regions_buffer;
}

/*	Writes a single byte to the file represented by "file"	*/
void df_type::df_writer_type::write_byte_vec(shared_type::byte_vec_type& byte_vec, std::ofstream& file, const bool& byte_endianness)
{
	/*	Loops through and writes each byte in "byte_vec"	*/

	unsigned long long size = byte_vec.char_vec.size();
	for (unsigned long long a = 0u; a < size; ++a)
	{
		unsigned long long byte_index;
		if (byte_endianness == true)
		{
			byte_index = (size - a) - 1u;
		}
		else
		{
			byte_index = a;
		}

		file.put(byte_vec.char_vec[byte_index]);
	}
}

/*	Writes the contents of "regions_buffer" to the file represented by "file" (endianness parameter is false for all calls to "write_byte_vec")	*/
void df_type::df_writer_type::write_to_file()
{
	write_byte_vec(regions_buffer->grid_is_valid, file, false);
	write_byte_vec(regions_buffer->write_id, file, false);
	write_byte_vec(regions_buffer->grid_min_coord, file, false);
	write_byte_vec(regions_buffer->grid_spacing, file, false);
	write_byte_vec(regions_buffer->df_distance, file, false);
	write_byte_vec(regions_buffer->cmprt_size, file, false);
	write_byte_vec(regions_buffer->volume_verts, file, false);
	write_byte_vec(regions_buffer->grid_point_amount, file, false);
	write_byte_vec(regions_buffer->max_grid_point, file, false);
	write_byte_vec(regions_buffer->dfc_amount, file, false);
	write_byte_vec(regions_buffer->dfc_ids, file, false);
	write_byte_vec(regions_buffer->grid_meta, file, false);
	write_byte_vec(regions_buffer->grid_values, file, false);
	write_byte_vec(regions_buffer->cmprt_data, file, false);
	write_byte_vec(regions_buffer->dfc_cache_is_valid, file, false);
	write_byte_vec(regions_buffer->vert_amount_total, file, false);
	write_byte_vec(regions_buffer->vert_cache_next_index, file, false);
	write_byte_vec(regions_buffer->vert_cache, file, false);
	write_byte_vec(regions_buffer->tri_cache_size, file, false);
	write_byte_vec(regions_buffer->tri_cache, file, false);
	write_byte_vec(regions_buffer->mesh_cache, file, false);
	write_byte_vec(regions_buffer->existing_dfcs, file, false);
	write_byte_vec(regions_buffer->dfc_id_index_table, file, false);
	write_byte_vec(regions_buffer->dfr_ids, file, false);
	write_byte_vec(regions_buffer->dfc_layers, file, false);
	write_byte_vec(regions_buffer->dfr_layers, file, false);
}


df_type::df_writer_type::~df_writer_type()
{
	file.close();
}