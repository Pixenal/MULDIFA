/*
 -------------------------------------------------------------------------------------------------------------
	Copyright(C) 2021 Pixenal


	This file is part of MULDIFA.

	MULDIFA is free software : you can redistribute it and /or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MULDIFA is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MULDIFA.If not, see < https://www.gnu.org/licenses/>.
	-------------------------------------------------------------------------------------------------------------
*/


#include "../df_common/df_lib.h"
#include "../df_common/deflate.h"


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

/*	Writes the contents of "regions_buffer" to the file represented by "file" (endianness parameter is false for all calls to "shared.write_byte_vec")	*/
void df_type::df_writer_type::write_to_file()
{
	//deflate_code_type deflate_test(regions_buffer->cmprt_data);
	shared.write_byte_vec(regions_buffer->grid_is_valid, file, false);
	shared.write_byte_vec(regions_buffer->write_id, file, false);
	shared.write_byte_vec(regions_buffer->grid_min_coord, file, false);
	shared.write_byte_vec(regions_buffer->grid_spacing, file, false);
	shared.write_byte_vec(regions_buffer->df_distance, file, false);
	shared.write_byte_vec(regions_buffer->cmprt_size, file, false);
	shared.write_byte_vec(regions_buffer->volume_verts, file, false);
	shared.write_byte_vec(regions_buffer->grid_point_amount, file, false);
	shared.write_byte_vec(regions_buffer->max_grid_point, file, false);
	shared.write_byte_vec(regions_buffer->dfc_amount, file, false);
	shared.write_byte_vec(regions_buffer->dfc_ids, file, false);
	shared.write_byte_vec(regions_buffer->grid_meta, file, false);
	shared.write_byte_vec(regions_buffer->grid_values, file, false);
	shared.write_byte_vec(regions_buffer->cmprt_data, file, false);
	shared.write_byte_vec(regions_buffer->dfc_cache_is_valid, file, false);
	shared.write_byte_vec(regions_buffer->vert_amount_total, file, false);
	shared.write_byte_vec(regions_buffer->vert_cache_next_index, file, false);
	shared.write_byte_vec(regions_buffer->vert_cache, file, false);
	shared.write_byte_vec(regions_buffer->tri_cache_size, file, false);
	shared.write_byte_vec(regions_buffer->tri_cache, file, false);
	shared.write_byte_vec(regions_buffer->mesh_cache, file, false);
	shared.write_byte_vec(regions_buffer->existing_dfcs, file, false);
	shared.write_byte_vec(regions_buffer->dfc_id_index_table, file, false);
	shared.write_byte_vec(regions_buffer->dfr_ids, file, false);
	shared.write_byte_vec(regions_buffer->dfc_layers, file, false);
	shared.write_byte_vec(regions_buffer->dfr_layers, file, false);
#ifdef DFMAPINDFCACHE
	shared.write_byte_vec(regions_buffer->dfr_cache_is_valid, file, false);
	shared.write_byte_vec(regions_buffer->dfr_cache, file, false);
#endif
}


df_type::df_writer_type::~df_writer_type()
{
	file.close();
}