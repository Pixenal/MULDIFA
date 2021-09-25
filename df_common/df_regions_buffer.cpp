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


/*df_type::regions_buffer_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_grid_is_valid(const volume_local_type& volume_local)
{
	grid_is_valid.clean();
	grid_is_valid.incrmt();

	shared.feed_by_bit(grid_is_valid, 0u, volume_local.grid != nullptr, 1u);
	grid_is_valid.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_write_id(const shared_type::write_id_type write_id)
{
	this->write_id.clean();
	this->write_id.incrmt();

	shared.feed_by_bit(this->write_id, 0u, write_id.index, 0u);
	shared.feed_by_bit(this->write_id, (this->write_id.char_vec.size() - 1), write_id.rand, 0u);
	this->write_id.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if grid is not valid	*/
void df_type::regions_buffer_type::prep_grid_min_coord(const volume_local_type& volume_local)
{
	grid_min_coord.clean();
	grid_min_coord.incrmt();

	if (volume_local.grid != nullptr)
	{
		shared.feed_by_bit(grid_min_coord, 0u, volume_local.min_grid_coord.x, 0u);
		shared.feed_by_bit(grid_min_coord, (grid_min_coord.char_vec.size() - 1), volume_local.min_grid_coord.y, 0u);
		shared.feed_by_bit(grid_min_coord, (grid_min_coord.char_vec.size() - 1), volume_local.min_grid_coord.z, 0u);
	}
	grid_min_coord.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_grid_spacing(const volume_local_type& volume_local)
{
	grid_spacing.clean();
	grid_spacing.incrmt();
	shared.feed_by_bit(grid_spacing, 0u, volume_local.intern_grid_spacing, 0u);
	grid_spacing.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_df_distance(const volume_local_type& volume_local)
{
	df_distance.clean();
	df_distance.incrmt();
	shared.feed_by_bit(df_distance, 0u, volume_local.intern_df_distance, 0u);
	df_distance.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_cmprt_size(const volume_local_type& volume_local)
{
	cmprt_size.clean();
	cmprt_size.incrmt();
	shared.feed_by_bit(cmprt_size, 0u, volume_local.intern_df_cmprt_size, 0u);
	cmprt_size.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_volume_verts(const volume_local_type& volume_local)
{
	volume_verts.clean();
	volume_verts.incrmt();
	for (unsigned short a = 0u; a < 8u; ++a)
	{
		shared.feed_by_bit(volume_verts, (volume_verts.char_vec.size() - 1), volume_local.intern_volume_verts[a].x, 0u);
		shared.feed_by_bit(volume_verts, (volume_verts.char_vec.size() - 1), volume_local.intern_volume_verts[a].y, 0u);
		shared.feed_by_bit(volume_verts, (volume_verts.char_vec.size() - 1), volume_local.intern_volume_verts[a].z, 0u);
	}
	volume_verts.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if grid is not valid	*/
void df_type::regions_buffer_type::prep_grid_point_amount(const volume_local_type& volume_local)
{
	grid_point_amount.clean();
	grid_point_amount.incrmt();

	if (volume_local.grid != nullptr)
	{
		shared.feed_by_bit(grid_point_amount, (grid_point_amount.char_vec.size() - 1), volume_local.grid_amount.x, 0u);
		shared.feed_by_bit(grid_point_amount, (grid_point_amount.char_vec.size() - 1), volume_local.grid_amount.y, 0u);
		shared.feed_by_bit(grid_point_amount, (grid_point_amount.char_vec.size() - 1), volume_local.grid_amount.z, 0u);
	}
	grid_point_amount.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if grid is not valid	*/
void df_type::regions_buffer_type::prep_max_grid_point(const volume_local_type& volume_local)
{
	max_grid_point.clean();
	max_grid_point.incrmt();

	if (volume_local.grid != nullptr)
	{
		shared.feed_by_bit(max_grid_point, (max_grid_point.char_vec.size() - 1), volume_local.max_grid_point.x, 0u);
		shared.feed_by_bit(max_grid_point, (max_grid_point.char_vec.size() - 1), volume_local.max_grid_point.y, 0u);
		shared.feed_by_bit(max_grid_point, (max_grid_point.char_vec.size() - 1), volume_local.max_grid_point.z, 0u);
	}
	max_grid_point.buffer_to_char_vec();
}

/*	first bit: Wether grid point has any layers
	second bit: Wether grid point has more than 1 layer
	third bit: Wether the amount of layers is represented with 6 bits or 16 bits
	next 6/16 bits: The amount of layers the grid point has	*/

/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state.
	This function is a bit more complicated than that of other regions. For size optimization purposes, the data that represents the grid
	is split into 2 regions, "grid_meta" and "grid_values". Grid meta contains information, individually for each grid point on
	how the the data stored in in "grid_values" is formated (obviously, said information in "grid_meta" is also used to convey
	info in and of itself, such as number of layers). Each grid points entry in "grid_meta" is layed out as follows:
	- 1st bit flag is 1 if the grid point has more than 0 layers, else it's 0. If 0, there are no more bits for the current grid point
	- 2nd bit flag is 1 if there is more than 1 layer, else it's 0. If 0, there are no more bits for the current grid point	
	- 3rd bit flag is 1 if the number of bytes used to represent the number of layers is 2, if 0, then is 1 byte
	- The next 1 - 2 byte(s) represent the number of layers	in the current grid point 	
	
	This function's inconsistancy with the way the other regons's respective functions operate is primarily due to it being pretty much
	the first function I made for the serializer, I was still figuring out how to go about laying everything out	*/
void df_type::regions_buffer_type::prep_grid_meta(const volume_local_type& volume_local)
{
	grid_meta.clean();
	grid_values.clean();

	/*	Sets up objects as appropriate	*/
	bool repeat = false;
	unsigned short bit_counter = 0u;
	unsigned short values_bit_counter = 0u;
	unsigned short layer_itrtns_counter = 0u;
	std::bitset<16> layers_size_buffer;
	bool layers_size_2_byte = false;
	unsigned short layers_size_buffer_counter = 0u;
	unsigned short layers_size = 0u;;
	grid_meta.incrmt();
	unsigned long long values_byte_index = 0u;
	grid_values.incrmt();

	/*	Leaves as single empty byte if grid is not valid	*/
	if (volume_local.grid != nullptr)
	{
		/*	The below for loops may iterate multiple times for each grid point (note the lack of increment expresion, this is done in the 
		compound statement). If there are no layers in the current grid point, then the loop's body will only be run once for that grid point,
		however if there are layers, the body will be run twice (and more if there are more than 1 layer) to the additional data	*/
		for (unsigned short a = 0u; a < volume_local.grid_amount.x;)
		{
			for (unsigned short b = 0u; b < volume_local.grid_amount.y;)
			{
				for (unsigned short c = 0u; c < volume_local.grid_amount.z;)
				{
					/*	Increment byte_vec if "bit_counter" is >= 8, and set "bit_counter" to 0	*/
					if (bit_counter >= 8)
					{
						grid_meta.incrmt();
						bit_counter = 0u;
					}

					if (layer_itrtns_counter == 0u)
					{
						volume_local.grid[a][b][c].calc_layer_size();
						layers_size = volume_local.grid[a][b][c].layers_size;

						/*	If amount of layers is > 0, continue, otherwise set 1st bit flag to 0 and move on to next grid point	*/
						if (layers_size > 0u)
						{
							repeat = true;
							grid_meta.byte_buffer[bit_counter] = 1;
							++layer_itrtns_counter;

							/*	The "grid_values" region is written to in this function as well, the below for loop serializes and writes the
								necessary info for each layer in the current grid point	*/
							for (unsigned short d = 0u; d < layers_size; ++d)
							{
								/*	For the purpose of size optimization, get integral representation of value (with value being a float)	*/
								unsigned short expanded_value = (unsigned short)(volume_local.grid[a][b][c].layers[d]->value * 63.0f);

								/*	Format:
									-	First 6 bits: Value
									-	sv entry representing dfc_id
									-	sv entry representing mesh_index
									-	sv entry representing mesh_layer-table_index	*/

								shared.feed_by_bit(grid_values, (grid_values.char_vec.size() - 1), expanded_value, 6u);
								shared.feed_by_bit(grid_values, (grid_values.char_vec.size() - 1), *volume_local.grid[a][b][c].layers[d]->mesh_indx.dfc_id, 0u);
								shared.feed_by_bit(grid_values, (grid_values.char_vec.size() - 1), volume_local.grid[a][b][c].layers[d]->mesh_indx.mesh_index, 0u);
								shared.feed_by_bit(grid_values, (grid_values.char_vec.size() - 1), volume_local.grid[a][b][c].layers[d]->mesh_layer_table_index, 0u);
							}
						}
						else
						{
							repeat = false;
							grid_meta.byte_buffer[bit_counter] = 0;
						}
					}
					else if (layer_itrtns_counter == 1u)
					{
						/*	If amount of layers > 1, set flag to 1, else set to 0 and continue on to next grid point	*/
						if (layers_size > 1u)
						{
							grid_meta.byte_buffer[bit_counter] = 1;
							++layer_itrtns_counter;
						}
						else
						{
							repeat = false;
							grid_meta.byte_buffer[bit_counter] = 0;
							layer_itrtns_counter = 0u;
						}
					}
					else if (layer_itrtns_counter == 2u)
					{
						/*	Calc number of bits needed to represent the number of layers	*/
						unsigned short bit_size = std::log2(layers_size) + 1u;

						/*	If the number of bits exceeds 63, use 2 bytes, else use 1 byte	*/
						if (bit_size > 63u)
						{
							layers_size_2_byte = true;
							grid_meta.byte_buffer[bit_counter] = 1;
						}
						else
						{
							layers_size_2_byte = false;
							grid_meta.byte_buffer[bit_counter] = 0;
						}

						++layer_itrtns_counter;
					}
					/*	The enclosing loop will iterate and continue to run the body of the below else if statement until all
						6 or 16 bits are written, after which the loop will move on to the next grid point	*/
					else if (((layers_size_buffer_counter < 16u) && (layers_size_2_byte == true)) ||
							((layers_size_buffer_counter < 6u) && (layers_size_2_byte == false)))
					{
						if (layer_itrtns_counter == 3u)
						{
							layers_size_buffer = std::bitset<16>(layers_size);
						}

						grid_meta.byte_buffer[bit_counter] = (int)layers_size_buffer[layers_size_buffer_counter];

						++layers_size_buffer_counter;
						++layer_itrtns_counter;
					}
					else
					{
						repeat = false;
						layers_size_buffer_counter = 0u;
						layer_itrtns_counter = 0u;

						goto incrmt_iterator;
					}

					++bit_counter;

				incrmt_iterator:

					if (repeat == false)
					{
						++c;
					}
				}

				if (repeat == false)
				{
					++b;
				}
			}

			if (repeat == false)
			{
				++a;
			}
		}
	}
	grid_meta.buffer_to_char_vec();
	grid_values.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if grid is not valid	*/
void df_type::regions_buffer_type::prep_cmprt_data(const volume_local_type& volume_local)
{
	cmprt_data.clean();
	cmprt_data.incrmt();

	if (volume_local.grid != nullptr)
	{
		shared.feed_by_bit(cmprt_data, 0u, volume_local.cmprt_amount.x, 0u);
		shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), volume_local.cmprt_amount.y, 0u);
		shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), volume_local.cmprt_amount.z, 0u);

		for (unsigned long a = 0u; a < volume_local.cmprt_amount_total; ++a)
		{
			shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), volume_local.cmprt_meta[a].size, 0u);

			for (unsigned long b = 0u; b < volume_local.cmprt_meta[a].size; ++b)
			{
				const shared_type::index_xyz_type& index = volume_local.cmprt_table[a][b]->index;
				unsigned long linear_index = (index.z + (volume_local.grid_amount.z * index.y)) + ((volume_local.grid_amount.z * volume_local.grid_amount.y) * index.x);

				shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), linear_index, 0u);
			}

			shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), volume_local.cmprt_meta[a].rlvncy_table_size, 0u);

			for (unsigned short b = 0u; b < volume_local.cmprt_meta[a].rlvncy_table_size; ++b)
			{
				shared.feed_by_bit(cmprt_data, (cmprt_data.char_vec.size() - 1), volume_local.cmprt_rlvncy_table[a][b], 0u);
			}
		}
	}
	cmprt_data.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfc_amount(update_local_type& update_local)
{
	dfc_amount.clean();
	dfc_amount.incrmt();
	shared.feed_by_bit(dfc_amount, (dfc_amount.char_vec.size() - 1), update_local.dfc_cache.dfc_amount, 0u);
	dfc_amount.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfc_ids(update_local_type& update_local)
{
	dfc_ids.clean();
	update_local.dfc_cache.dfc_ids.calc_size();
	shared.feed_by_bit(dfc_ids, 0u, update_local.dfc_cache.dfc_ids.size, 0u);

	for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_ids.size; ++a)
	{
		shared.feed_by_bit(dfc_ids, (dfc_ids.char_vec.size() - 1), update_local.dfc_cache.dfc_ids.vector[a]->id, 0u);
		shared.feed_by_bit(dfc_ids, (dfc_ids.char_vec.size() - 1), update_local.dfc_cache.dfc_ids.vector[a]->index, 0u);
	}
	dfc_ids.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfc_cache_is_valid(update_local_type& update_local)
{
	dfc_cache_is_valid.clean();
	dfc_cache_is_valid.incrmt();
	dfc_cache_is_valid.byte_buffer[0] = (int)update_local.dfc_cache.is_valid;
	dfc_cache_is_valid.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_vert_amount_total(update_local_type& update_local)
{
	vert_amount_total.clean();
	vert_amount_total.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		shared.feed_by_bit(vert_amount_total, (vert_amount_total.char_vec.size() - 1), update_local.dfc_cache.vert_amount_total, 0u);
	}
	vert_amount_total.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_vert_cache_next_index(update_local_type& update_local)
{
	vert_cache_next_index.clean();
	vert_cache_next_index.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		shared.feed_by_bit(vert_cache_next_index, (vert_cache_next_index.char_vec.size() - 1), update_local.dfc_cache.vert_cache_next_index, 0u);
	}
	vert_cache_next_index.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_vert_cache(update_local_type& update_local)
{
	vert_cache.clean();
	vert_cache.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		for (unsigned long long a = 0u; a < update_local.dfc_cache.vert_amount_total; ++a)
		{
			shared.feed_by_bit(vert_cache, (vert_cache.char_vec.size() - 1), update_local.dfc_cache.vert_cache[a].dfc_index, 0u);

			shared.feed_by_bit(vert_cache, (vert_cache.char_vec.size() - 1), update_local.dfc_cache.vert_cache[a].coord.x, 0u);
			shared.feed_by_bit(vert_cache, (vert_cache.char_vec.size() - 1), update_local.dfc_cache.vert_cache[a].coord.y, 0u);
			shared.feed_by_bit(vert_cache, (vert_cache.char_vec.size() - 1), update_local.dfc_cache.vert_cache[a].coord.z, 0u);
		}
	}
	vert_cache.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_tri_cache_size(update_local_type& update_local)
{
	tri_cache_size.clean();
	tri_cache_size.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_amount; ++a)
		{
			shared.feed_by_bit(tri_cache_size, (tri_cache_size.char_vec.size() - 1), update_local.dfc_cache.tri_cache_size[a], 0u);
		}
	}
	tri_cache_size.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_tri_cache(update_local_type& update_local)
{
	tri_cache.clean();
	tri_cache.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_amount; ++a)
		{
			for (unsigned long long b = 0u; b < update_local.dfc_cache.tri_cache_size[a]; ++b)
			{
				shared.feed_by_bit(tri_cache, (tri_cache.char_vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].vert_0, 0u);
				shared.feed_by_bit(tri_cache, (tri_cache.char_vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].vert_1, 0u);
				shared.feed_by_bit(tri_cache, (tri_cache.char_vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].vert_2, 0u);

				//shared.feed_by_bit(tri_cache, (tri_cache.vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].normal.x, 0u);
				//shared.feed_by_bit(tri_cache, (tri_cache.vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].normal.y, 0u);
				//shared.feed_by_bit(tri_cache, (tri_cache.vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].normal.z, 0u);

				//shared.feed_by_bit(tri_cache, (tri_cache.vec.size() - 1), update_local.dfc_cache.tri_cache[a][b].d, 0u);
			}
		}
	}
	tri_cache.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_mesh_cache(volume_local_type& volume_local, update_local_type& update_local)
{
	mesh_cache.clean();
	mesh_cache.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_amount; ++a)
		{
			unsigned long mesh_cache_size = update_local.dfc_cache.mesh_cache[a]->size();
			shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), mesh_cache_size, 0u);

			for (unsigned long b = 0u; b < mesh_cache_size; ++b)
			{
				unsigned long long tris_size = update_local.dfc_cache.mesh_cache[a]->at(b)->tris.size();
				shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), tris_size, 0u);

				for (unsigned long long c = 0u; c < tris_size; ++c)
				{
					shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), update_local.dfc_cache.mesh_cache[a]->at(b)->tris[c], 0u);
				}

				unsigned long long layer_indices_size = update_local.dfc_cache.mesh_cache[a]->at(b)->layer_indices->size();
				shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), layer_indices_size, 0u);

				for (unsigned long long c = 0u; c < layer_indices_size; ++c)
				{
					shared_type::index_xyzw_type& index = update_local.dfc_cache.mesh_cache[a]->at(b)->layer_indices->at(c);
					unsigned long long linear_index = (index.z + (volume_local.grid_amount.z * index.y)) + ((volume_local.grid_amount.z * volume_local.grid_amount.y) * index.x);

					shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), linear_index, 0u);
					shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), index.w, 0u);
				}

				shared.feed_by_bit(mesh_cache, (mesh_cache.char_vec.size() - 1), (unsigned long long)update_local.dfc_cache.mesh_cache[a]->at(b)->moved_from_legacy, 1u);
			}
		}
	}
	mesh_cache.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state
	Leaves as single empty byte if dfc_cache is not valid	*/
void df_type::regions_buffer_type::prep_existing_dfcs(update_local_type& update_local)
{
	existing_dfcs.clean();
	existing_dfcs.incrmt();

	if (update_local.dfc_cache.is_valid == true)
	{
		for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_amount; ++a)
		{
			shared.feed_by_bit(existing_dfcs, existing_dfcs.char_vec.size() - 1, update_local.dfc_cache.existing_dfcs[a], 8u);
		}
	}
	existing_dfcs.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfc_id_index_table(update_local_type& update_local)
{
	dfc_id_index_table.clean();
	dfc_id_index_table.incrmt();
	for (unsigned long a = 0u; a < update_local.dfc_cache.dfc_amount; ++a)
	{
		shared.feed_by_bit(dfc_id_index_table, (dfc_id_index_table.char_vec.size() - 1), update_local.dfc_cache.dfc_indx_to_id_table[a]->id, 0u);
	}
	dfc_id_index_table.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfr_ids(update_local_type& update_local)
{
	dfr_ids.clean();
	dfr_ids.incrmt();
	update_local.dfr_cache.dfr_ids.calc_size();
	shared.feed_by_bit(dfr_ids, 0u, update_local.dfr_cache.dfr_ids.size, 0u);

	for (unsigned long a = 0u; a < update_local.dfr_cache.dfr_ids.size; ++a)
	{
		shared.feed_by_bit(dfr_ids, (dfr_ids.char_vec.size() - 1), update_local.dfr_cache.dfr_ids.vector[a]->id, 0u);
		shared.feed_by_bit(dfr_ids, (dfr_ids.char_vec.size() - 1), update_local.dfr_cache.dfr_ids.vector[a]->index, 0u);
	}
	dfr_ids.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfc_layers(std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers)
{
	this->dfc_layers.clean();
	this->dfc_layers.incrmt();
	unsigned short layers_size = dfc_layers.size();
	shared.feed_by_bit(this->dfc_layers, 0u, layers_size, 0u);

	for (unsigned short a = 0u; a < layers_size; ++a)
	{
		dfc_layers[a].calc_size();
		shared.feed_by_bit(this->dfc_layers, (this->dfc_layers.char_vec.size() - 1), dfc_layers[a].size, 0u);

		for (unsigned short b = 0u; b < dfc_layers[a].size; ++b)
		{
			shared.feed_by_bit(this->dfc_layers, (this->dfc_layers.char_vec.size() - 1), dfc_layers[a].vector[b]->id, 0u);
		}
	}
	this->dfc_layers.buffer_to_char_vec();
}


/*	Serializes respective object(s) in df state, storing the resulting byte string in it's respective region in the buffer.
	This does not affect the objects in df state	*/
void df_type::regions_buffer_type::prep_dfr_layers(std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers)
{
	this->dfr_layers.clean();
	this->dfr_layers.incrmt();
	unsigned short layers_size = dfr_layers.size();
	shared.feed_by_bit(this->dfr_layers, 0u, layers_size, 0u);

	for (unsigned short a = 0u; a < layers_size; ++a)
	{
		dfr_layers[a].calc_size();
		shared.feed_by_bit(this->dfr_layers, (this->dfr_layers.char_vec.size() - 1), dfr_layers[a].size, 0u);

		for (unsigned short b = 0u; b < dfr_layers[a].size; ++b)
		{
			shared.feed_by_bit(this->dfr_layers, (this->dfr_layers.char_vec.size() - 1), dfr_layers[a].vector[b]->id, 0u);
		}
	}
	this->dfr_layers.buffer_to_char_vec();
}


void df_type::regions_buffer_type::clean()
{
	write_id.clean();
	grid_min_coord.clean();
	grid_spacing.clean();
	df_distance.clean();
	cmprt_size.clean();
	volume_verts.clean();
	grid_point_amount.clean();
	max_grid_point.clean();
	grid_meta.clean();
	grid_values.clean();
	cmprt_data.clean();
	dfc_amount.clean();
	dfc_ids.clean();
	dfc_cache_is_valid.clean();
	vert_amount_total.clean();
	vert_cache_next_index.clean();
	vert_cache.clean();
	tri_cache_size.clean();
	tri_cache.clean();
	mesh_cache.clean();
	existing_dfcs.clean();
	dfc_id_index_table.clean();
	dfr_ids.clean();
	dfc_layers.clean();
	dfr_layers.clean();
}


/*	Returns copy of virtual_stream object (using an explicitely defined copy constructor)	*/
df_type::regions_buffer_type::virtual_stream_type df_type::regions_buffer_type::get_virtual_stream()
{
	return virtual_stream_type(*this);
}


/*df_type::regions_buffer_type::virtual_stream_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*	Increments index of current byte being read from virtual stream. As the regions buffer is made up of multiple regions,
	each containing a byte string/ byte vec, the index must jump between each region when the it reaches the end of the
	region it's current reading from	*/
int df_type::regions_buffer_type::virtual_stream_type::incrmt_index()
{
	++index.byte;
	if (index.byte >= current_region_size)
	{
		++index.region;
		if (index.region >= regions_size)
		{
			intern_end_of_stream = true;
			return 1;
		}
		else
		{
			current_region_size = regions_arr[index.region]->char_vec.size();
			index.byte = 0u;
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


/*	Copy constructor	*/
df_type::regions_buffer_type::virtual_stream_type::virtual_stream_type(regions_buffer_type& regions_buffer)
{
	regions_buffer_ptr = &regions_buffer;

	regions_arr[0] = &regions_buffer.grid_is_valid;
	regions_arr[1] = &regions_buffer.write_id;
	regions_arr[2] = &regions_buffer.grid_min_coord;
	regions_arr[3] = &regions_buffer.grid_spacing;
	regions_arr[4] = &regions_buffer.df_distance;
	regions_arr[5] = &regions_buffer.cmprt_size;
	regions_arr[6] = &regions_buffer.volume_verts;
	regions_arr[7] = &regions_buffer.grid_point_amount;
	regions_arr[8] = &regions_buffer.max_grid_point;
	regions_arr[9] = &regions_buffer.dfc_amount;
	regions_arr[10] = &regions_buffer.dfc_ids;
	regions_arr[11] = &regions_buffer.grid_meta;
	regions_arr[12] = &regions_buffer.grid_values;
	regions_arr[13] = &regions_buffer.cmprt_data;
	regions_arr[14] = &regions_buffer.dfc_cache_is_valid;
	regions_arr[15] = &regions_buffer.vert_amount_total;
	regions_arr[16] = &regions_buffer.vert_cache_next_index;
	regions_arr[17] = &regions_buffer.vert_cache;
	regions_arr[18] = &regions_buffer.tri_cache_size;
	regions_arr[19] = &regions_buffer.tri_cache;
	regions_arr[20] = &regions_buffer.mesh_cache;
	regions_arr[21] = &regions_buffer.existing_dfcs;
	regions_arr[22] = &regions_buffer.dfc_id_index_table;
	regions_arr[23] = &regions_buffer.dfr_ids;
	regions_arr[24] = &regions_buffer.dfc_layers;
	regions_arr[25] = &regions_buffer.dfr_layers;

	current_region_size = regions_arr[0]->char_vec.size();
}


df_type::regions_buffer_type::virtual_stream_type::virtual_stream_type()
{

}


/*	Returns current byte in virtual stream, if end of stream has been reached returns an empty byte	*/
std::bitset<8> df_type::regions_buffer_type::virtual_stream_type::get_byte()
{
	if (intern_end_of_stream == false)
	{
		regions_arr[index.region]->buffer_char(index.byte);
		std::bitset<8> return_buffer = regions_arr[index.region]->byte_buffer;
		incrmt_index();
		return return_buffer;

	}
	else
	{
		return std::bitset<8>();
	}
}

/*	Assignment operater overload	*/
df_type::regions_buffer_type::virtual_stream_type df_type::regions_buffer_type::virtual_stream_type::operator=(const virtual_stream_type& old)
{
	intern_end_of_stream = old.end_of_stream;
	index.region = old.index.region;
	index.byte = old.index.byte;
	current_region_size = old.current_region_size;
	regions_buffer_ptr = old.regions_buffer_ptr;

	for (unsigned short a = 0; a < regions_size; ++a)
	{
		regions_arr[a] = old.regions_arr[a];
	}

	return *this;
}


/*df_type::regions_buffer_type::virtual_stream_type::index_type*/
/*-------------------------------------------------------------------------------------------------------------*/