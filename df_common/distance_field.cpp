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
#include "../df_common/thread_pool.h"
#include "../df_common/deflate.h"
#include "../df_common/png_encoder.h"
#include <chrono>
#include <limits>

extern shared_type shared;
extern thread_pool_type thread_pool;

extern std::string lib_dir;


/*df_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Contains logic specific to the function in this this function was created to be called within, either mark this as being specific to that function, or make it more generalized*/
/*	Converts world space coordinates to compartment index space*/
shared_type::coord_xyz_type df_type::wrld_space_to_cmprt_indx_space(const shared_type::coord_xyz_type& coordinates)
{
	shared_type::coord_xyz_type current_coord_grid_space;
	shared_type::coord_xyz_type current_coord_indx_space;

	/*	Converts worldspace coord in index space by first converting to grid index space (translate to 0, then divide by the grid spacing), the coords, now in grid index space are
		then divided by the amount of grid points in per compartment in order to get the coord in compartment index space*/
	current_coord_grid_space.x = (coordinates.x - volume_local.min_grid_coord.x) / volume_local.intern_grid_spacing;
	current_coord_indx_space.x = current_coord_grid_space.x / volume_local.intern_df_cmprt_size;

	current_coord_grid_space.y = (coordinates.y - volume_local.min_grid_coord.y) / volume_local.intern_grid_spacing;
	current_coord_indx_space.y = current_coord_grid_space.y / volume_local.intern_df_cmprt_size;

	current_coord_grid_space.z = (coordinates.z - volume_local.min_grid_coord.z) / volume_local.intern_grid_spacing;
	current_coord_indx_space.z = current_coord_grid_space.z / volume_local.intern_df_cmprt_size;

	/*Clamps results (0 to max compart index - .5)*/

	if (current_coord_indx_space.x < 0)
	{
		current_coord_indx_space.x = 0;
	}
	else if (current_coord_indx_space.x >= (volume_local.cmprt_amount.x))
	{
		current_coord_indx_space.x = volume_local.cmprt_amount.x - .5;
	}

	if (current_coord_indx_space.y < 0)
	{
		current_coord_indx_space.y = 0;
	}
	else if (current_coord_indx_space.y >= (volume_local.cmprt_amount.y))
	{
		current_coord_indx_space.y = volume_local.cmprt_amount.y - .5;
	}

	if (current_coord_indx_space.z < 0)
	{
		current_coord_indx_space.z = 0;
	}
	else if (current_coord_indx_space.z >= (volume_local.cmprt_amount.z))
	{
		current_coord_indx_space.z = volume_local.cmprt_amount.z - .5;
	}

	return current_coord_indx_space;
}


/*	Truncates value to nearest .5 increment, eg: 0, .5, 1, 1.5, 2, 2.5, ...	*/
double df_type::trunc_to_pnt_five_incrmts(const double& value)
{
	double return_value;

	return_value = trunc(value);

	/*	If the difference between the original value and the truncated value is greater than .5, then .5 is added, eg, if the original value was 1.7, then
		the truncated value is obviously 1, resulting in a differenc of .7, which, in turn, results in .5 being added to the truncated value, resulting in
		1.5	*/	
	if ((value - return_value) > .5)
	{
		return_value += .5;
	}

	return return_value;
}


void df_type::rasterize_on_axis(shared_type::index_xyz_type* enclosing_cmprts, unsigned short& enclosing_cmprts_next_index, const shared_type::coord_xyz_type& vert_0_indx_space, const shared_type::coord_xyz_type& vert_1_indx_space, const shared_type::coord_xyz_type& vert_2_indx_space, const char& axis)
{
	/*Note: the switch statemtns in this function are for adapting the function to whichever axis is specified in the parameter "axis" */

	shared_type::coord_xyz_type max_vert;
	shared_type::coord_xyz_type mid_vert;
	shared_type::coord_xyz_type min_vert;

	double max_vert_incrmt = 0;
	double mid_vert_incrmt = 0;
	double min_vert_incrmt = 0;

	/*Creates a variation of the scan direction coord that is rounded to the nearest .5 (eg, 6.5, 6, 5.5, 5, 4.5, etc),
	  note, again, that this is only nessecary for the axis being scanned across										*/
	  /*-------------------------------------------------------------------------------------------------------------*/

	switch (axis)
	{
	case 'x':

		sort_on_axis(max_vert, mid_vert, min_vert, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'x');

		max_vert_incrmt = trunc_to_pnt_five_incrmts(max_vert.x);
		mid_vert_incrmt = trunc_to_pnt_five_incrmts(mid_vert.x);
		min_vert_incrmt = trunc_to_pnt_five_incrmts(min_vert.x);
		break;

	case 'y':

		sort_on_axis(max_vert, mid_vert, min_vert, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'y');

		max_vert_incrmt = trunc_to_pnt_five_incrmts(max_vert.y);
		mid_vert_incrmt = trunc_to_pnt_five_incrmts(mid_vert.y);
		min_vert_incrmt = trunc_to_pnt_five_incrmts(min_vert.y);
		break;

	case 'z':

		sort_on_axis(max_vert, mid_vert, min_vert, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'z');

		max_vert_incrmt = trunc_to_pnt_five_incrmts(max_vert.z);
		mid_vert_incrmt = trunc_to_pnt_five_incrmts(mid_vert.z);
		min_vert_incrmt = trunc_to_pnt_five_incrmts(min_vert.z);
		break;

	default:

		std::cout << "ERROR: call to function df_type::rasterize_on_axis, 0 or invalid axis argument provided" << std::endl;
	}


	shared_type::index_xyz_type cmprt_to_add;

	/*	Calculates how many increments it takes to go from max to mid (front end)	*/
	unsigned short incrmt_amount_front_end = (max_vert_incrmt - mid_vert_incrmt) / .5;

	/*	Calculates how many increments it takes to go from mid to min (back end)	*/
	unsigned short incrmt_amount_back_end = (mid_vert_incrmt - min_vert_incrmt) / .5;

	/*	Checks to see if if the increment amount is 0 or less, as there's no point doing the below vert_minus_vert
		calcs if the loops arn't going to actually iterate. This would occur if the triangle is only contained
		within a single half of a compartment (half because it increments in 0.5, which, given we're in compartmnet
		index space, equates to half of a compartment)																*/
	if ((incrmt_amount_front_end + incrmt_amount_back_end) > 0)
	{
		/*	This calculates the distance vectors between points, which is needed
			for getting the coords of the point of intersection along each edge.
			Is done here so that it doesn't need to be done every loop iteration	*/
		double mid_minus_max_x = mid_vert.x - max_vert.x;
		double mid_minus_max_y = mid_vert.y - max_vert.y;
		double mid_minus_max_z = mid_vert.z - max_vert.z;
		double min_minus_mid_x = min_vert.x - mid_vert.x;
		double min_minus_mid_y = min_vert.y - mid_vert.y;
		double min_minus_mid_z = min_vert.z - mid_vert.z;
		double min_minus_max_x = min_vert.x - max_vert.x;
		double min_minus_max_y = min_vert.y - max_vert.y;
		double min_minus_max_z = min_vert.z - max_vert.z;


		/*	Scans from max to mid along the axis specificed by the parameter "axis"	*/
		for (unsigned short b = 0; b < incrmt_amount_front_end; ++b)
		{
			double current_coord = max_vert_incrmt - (.5 * b);

			double t = 0;

			switch (axis)
			{
			case 'x':
			{
				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.x = (unsigned short)current_coord;

				t = (current_coord - max_vert.x) / min_minus_max_x;

				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * min_minus_max_y));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * min_minus_max_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				/*	Gets the compartment that encloses the current point of intersection on mid max edge of the current triangle	*/

				t = (current_coord - max_vert.x) / mid_minus_max_x;

				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * mid_minus_max_y));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * mid_minus_max_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;
				break;
			}
			case 'y':
			{
				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.y = (unsigned short)current_coord;

				t = (current_coord - max_vert.y) / min_minus_max_y;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * min_minus_max_x));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * min_minus_max_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				/*	Gets the compartment that encloses the current point of intersection on mid max edge of the current triangle	*/

				t = (current_coord - max_vert.y) / mid_minus_max_y;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * mid_minus_max_x));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * mid_minus_max_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;
				break;
			}
			case 'z':
			{
				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.z = (unsigned short)current_coord;

				t = (current_coord - max_vert.z) / min_minus_max_z;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * min_minus_max_x));
				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * min_minus_max_y));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				/*	Gets the compartment that encloses the current point of intersection on mid max edge of the current triangle	*/

				t = (current_coord - max_vert.z) / mid_minus_max_z;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * mid_minus_max_x));
				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * mid_minus_max_y));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;
				break;
			}
			default:
			{
				std::cout << "ERROR: call to function df_type::rasterize_on_axis, 0 or invalid axis argument provided" << std::endl;
			}
			}

		}

		/*Scans from mid to min along the axis specificed by the parameter "axis" */
		for (unsigned short b = 0; b < incrmt_amount_back_end; ++b)
		{
			double current_coord = mid_vert_incrmt - (.5 * b);

			double t = 0;

			switch (axis)
			{
			case 'x':
			{

				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.x = (unsigned short)current_coord;

				t = (current_coord - max_vert.x) / min_minus_max_x;

				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * min_minus_max_y));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * min_minus_max_z));



				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;


				/*	Gets the compartment that encloses the current point of intersection on mid min edge of the current triangle	*/

				t = (current_coord - mid_vert.x) / min_minus_mid_x;

				cmprt_to_add.y = (unsigned short)(mid_vert.y + (t * min_minus_mid_y));
				cmprt_to_add.z = (unsigned short)(mid_vert.z + (t * min_minus_mid_z));


				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				break;
			}
			case 'y':
			{
				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.y = (unsigned short)current_coord;

				t = (current_coord - max_vert.y) / min_minus_max_y;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * min_minus_max_x));
				cmprt_to_add.z = (unsigned short)(max_vert.z + (t * min_minus_max_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				/*	Gets the compartment that encloses the current point of intersection on mid min edge of the current triangle	*/

				t = (current_coord - mid_vert.y) / min_minus_mid_y;

				cmprt_to_add.x = (unsigned short)(mid_vert.x + (t * min_minus_mid_x));
				cmprt_to_add.z = (unsigned short)(mid_vert.z + (t * min_minus_mid_z));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;
				break;
			}
			case 'z':
			{
				/*	Gets the compartment that encloses the current point of intersection on hypotenuse of the current triangle	*/

				cmprt_to_add.z = (unsigned short)current_coord;

				t = (current_coord - max_vert.z) / min_minus_max_z;

				cmprt_to_add.x = (unsigned short)(max_vert.x + (t * min_minus_max_x));
				cmprt_to_add.y = (unsigned short)(max_vert.y + (t * min_minus_max_y));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;

				/*	Gets the compartment that encloses the current point of intersection on mid min edge of the current triangle	*/

				t = (current_coord - mid_vert.z) / min_minus_mid_z;

				cmprt_to_add.x = (unsigned short)(mid_vert.x + (t * min_minus_mid_x));
				cmprt_to_add.y = (unsigned short)(mid_vert.y + (t * min_minus_mid_y));

				enclosing_cmprts[enclosing_cmprts_next_index] = cmprt_to_add;
				enclosing_cmprts_next_index += 1;
				break;
			}
			default:
			{
				std::cout << "ERROR: call to function df_type::rasterize_on_axis, 0 or invalid axis argument provided" << std::endl;
			}
			}
		}
	}
}


/*Sorts coords on specific axis into min, mid, and max*/
void df_type::sort_on_axis(shared_type::coord_xyz_type& max_write_obj, shared_type::coord_xyz_type& mid_write_obj, shared_type::coord_xyz_type& min_write_obj, const shared_type::coord_xyz_type& coord_0, const shared_type::coord_xyz_type& coord_1, const shared_type::coord_xyz_type& coord_2, const char& axis)
{
	switch (axis)
	{
	case 'x':

		if (coord_1.x > coord_0.x)
		{
			if (coord_2.x > coord_1.x)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_1;
				min_write_obj = coord_0;
			}
			else
			{
				max_write_obj = coord_1;

				if (coord_2.x > coord_0.x)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_0;
				}
				else
				{
					mid_write_obj = coord_0;
					min_write_obj = coord_2;
				}
			}
		}
		else
		{
			if (coord_2.x > coord_0.x)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_0;
				min_write_obj = coord_1;
			}
			else
			{
				max_write_obj = coord_0;

				if (coord_2.x > coord_1.x)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_1;
				}
				else
				{
					mid_write_obj = coord_1;
					min_write_obj = coord_2;
				}
			}
		}
		break;

	case 'y':

		if (coord_1.y > coord_0.y)
		{
			if (coord_2.y > coord_1.y)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_1;
				min_write_obj = coord_0;
			}
			else
			{
				max_write_obj = coord_1;

				if (coord_2.y > coord_0.y)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_0;
				}
				else
				{
					mid_write_obj = coord_0;
					min_write_obj = coord_2;
				}
			}
		}
		else
		{
			if (coord_2.y > coord_0.y)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_0;
				min_write_obj = coord_1;
			}
			else
			{
				max_write_obj = coord_0;

				if (coord_2.y > coord_1.y)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_1;
				}
				else
				{
					mid_write_obj = coord_1;
					min_write_obj = coord_2;
				}
			}
		}
		break;

	case 'z':

		if (coord_1.z > coord_0.z)
		{
			if (coord_2.z > coord_1.z)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_1;
				min_write_obj = coord_0;
			}
			else
			{
				max_write_obj = coord_1;

				if (coord_2.z > coord_0.z)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_0;
				}
				else
				{
					mid_write_obj = coord_0;
					min_write_obj = coord_2;
				}
			}
		}
		else
		{
			if (coord_2.z > coord_0.z)
			{
				max_write_obj = coord_2;
				mid_write_obj = coord_0;
				min_write_obj = coord_1;
			}
			else
			{
				max_write_obj = coord_0;

				if (coord_2.z > coord_1.z)
				{
					mid_write_obj = coord_2;
					min_write_obj = coord_1;
				}
				else
				{
					mid_write_obj = coord_1;
					min_write_obj = coord_2;
				}
			}
		}
		break;

	default:

		std::cout << "ERROR: call to function df_type::sort_on_axis, 0 or invalid axis argument provided" << std::endl;
		break;
	}
}


/*	Returns the lerped value of 2 grid points; this is done by mutliplying the difference between the min and max values by the lerp alpha*/
/*	This can probably be generalized and put in "shared_type"	*/	
float df_type::lerp_btwn_grid_points_values(const unsigned short& min_grid_point_coord, const float& min_grid_point_value, const unsigned short& max_grid_point_coord, const float& max_grid_point_value, double lerp_alpha)
{
	/*	Calculates lerp differently depending on wether the max grid point, or the min grid point, is the greatest	*/
	if (max_grid_point_value > min_grid_point_value)
	{
		float value_difference = max_grid_point_value - min_grid_point_value;

		return min_grid_point_value + (value_difference * (float)lerp_alpha);
	}
	else
	{
		/*	In the event that the min is greater than the max, the lerp alpha is inverted	*/	

		lerp_alpha = 1.0f - lerp_alpha;

		float value_difference = min_grid_point_value - max_grid_point_value;

		return max_grid_point_value + (value_difference * (float)lerp_alpha);
	}
}


/*	Returns a pointer to dfc (specifically to the dfc's "dfc_id_indx_type" object) if it exists withing the specified dfc layer, and sets the value of parameter "return_dfc_id" to the dfcs index in said layer.
	However if the dfc does not exist in the dfc layer, returns nullptr and "return_dfc_index" remains unchanged	*/
df_type::dfc_id_indx_type* df_type::get_dfc_in_dfc_layer(unsigned long& return_dfc_indx, const unsigned long& layer_indx, const unsigned long& dfc_id)
{
	dfc_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfc_layers[layer_indx].size; ++a)
	{
		if (dfc_id == dfc_layers[layer_indx].vector[a]->id)
		{
			return_dfc_indx = a;
			return dfc_layers[layer_indx].vector[a];
		}
	}

	return nullptr;
}


/*	Returns a pointer to dfc (specifically to the dfc's "dfc_id_indx_type" object) if it exists withing the specified dfc layer, if not, returns nullptr	*/
df_type::dfc_id_indx_type* df_type::get_dfc_in_dfc_layer(const unsigned long& layer_indx, const unsigned long& dfc_id)
{
	dfc_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfc_layers[layer_indx].size; ++a)
	{
		if (dfc_id == dfc_layers[layer_indx].vector[a]->id)
		{
			return dfc_layers[layer_indx].vector[a];
		}
	}

	return nullptr;
}


/*	Returns a pointer to dfr (specifically to the dfr's "dfr_id_indx_type" object) if it exists withing the specified dfr layer, and sets the value of parameter "return_dfr_id" to the dfrs index in said layer.
	However if the dfr does not exist in the dfr layer, returns nullptr and "return_dfr_index" remains unchanged	*/
df_type::dfr_id_indx_type* df_type::get_dfr_in_dfr_layer(unsigned long& return_dfr_indx, const unsigned long& layer_indx, const unsigned long& dfr_id)
{
	dfr_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfr_layers[layer_indx].size; ++a)
	{
		if (dfr_id == dfr_layers[layer_indx].vector[a]->id)
		{
			return_dfr_indx = a;
			return dfr_layers[layer_indx].vector[a];
		}
	}

	return nullptr;
}


/*	Returns a pointer to dfr (specifically to the dfr's "dfr_id_indx_type" object) if it exists withing the specified dfr layer, if not, returns nullptr	*/
df_type::dfr_id_indx_type* df_type::get_dfr_in_dfr_layer(const unsigned long& layer_indx, const unsigned long& dfr_id)
{
	dfr_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfr_layers[layer_indx].size; ++a)
	{
		if (dfr_id == dfr_layers[layer_indx].vector[a]->id)
		{
			return dfr_layers[layer_indx].vector[a];
		}
	}

	return nullptr;
}


/*Calls function add_dfc_to_cache, is used as the dfc_cache class is private*/
int df_type::add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc)
{
	update_local.dfc_cache.add_dfc_to_cache(verts, vert_amount, tris, tri_amount, dfc_index, split_dfc);
	return 0;
}


/*initializes/ creates data structures (this should only need to be done if the bounding volume's dimensions,
  or settings related to the afor mentioned data structures, are changed (which generally wouldn't be very often given this tools intended use))*/

  /*The actual values of each point in the grid (the actual "distance field") is NOT calculated in this function, that is done in df_type::update */
  /*-------------------------------------------------------------------------------------------------------------*/

int df_type::initialize_volume(const shared_type::coord_xyz_type* volume_verts, const float& df_distance, const unsigned short& df_cmprt_size, const double& grid_spacing, const bool skip_sync_check)
{
	if (!skip_sync_check)
	{
		/*Checks if volume's transform and settings have changed, if not, then return as there is no need to re-initialize the volume*/
		if ((volume_local.intern_df_distance == df_distance) && (volume_local.intern_df_cmprt_size == df_cmprt_size) && (volume_local.intern_grid_spacing == grid_spacing))
		{
			for (short a = 0; a < 8; ++a)
			{
				if (volume_local.intern_volume_verts[a] != volume_verts[a])
				{
					goto has_changed;
				}
			}
			return 2;
		}
	}

has_changed:

	/*Cleans data, which involes deleting df related heap allocated data structures if they exist (the reason for this, and the reason why these data structures are dynamically
	allocated in the first place, is because the grid, and the structures derived from/ related to it, need to be able to change if the user defined volume changes*/
	/*-------------------------------------------------------------------------------------------------------------*/

	volume_local.clean();
	if (update_local.dfc_cache.is_valid)
	{
		update_local.dfc_cache.clean();
	}

	/*Stores internal copy of parameters so that they can be accessed in other methods (mainly so that df_check_volume can
	check if the current volume in scene, as well as the volume properties, match the parameters that the volume was initialized with)*/
	/*-------------------------------------------------------------------------------------------------------------*/

	volume_local.intern_df_distance = df_distance;
	volume_local.intern_df_cmprt_size = df_cmprt_size;
	volume_local.intern_grid_spacing = grid_spacing;

	/*Stores volume's vertice's coordinates*/
	for (short a = 0; a < 8; ++a)
	{
		volume_local.intern_volume_verts[a] = volume_verts[a];
	}

	/*Calculate what dimensions df.grid should have*/
	/*-------------------------------------------------------------------------------------------------------------*/

	volume_local.volume_verts_max = { -100000, -100000, -100000 };
	shared_type::coord_xyz_type volume_verts_min = { 100000, 100000, 100000 };

	/*Search through all vertices in volume object to get maximum and minimum positions for x, y, and z axis*/
	for (unsigned char a = 0; a < 8; ++a)
	{
		if ((volume_verts[a].x) > volume_local.volume_verts_max.x)
		{
			volume_local.volume_verts_max.x = (volume_verts[a].x);
		}
		if ((volume_verts[a].x) < volume_verts_min.x)
		{
			volume_verts_min.x = (volume_verts[a].x);
		}
		if ((volume_verts[a].y) > volume_local.volume_verts_max.y)
		{
			volume_local.volume_verts_max.y = (volume_verts[a].y);
		}
		if ((volume_verts[a].y) < volume_verts_min.y)
		{
			volume_verts_min.y = (volume_verts[a].y);
		}
		if ((volume_verts[a].z) > volume_local.volume_verts_max.z)
		{
			volume_local.volume_verts_max.z = (volume_verts[a].z);
		}
		if ((volume_verts[a].z) < volume_verts_min.z)
		{
			volume_verts_min.z = (volume_verts[a].z);
		}
	}

	/*Divides volume distance by desired spacing between grid points (obviously this is done for each axis, as the size of the volume may not be 1-1-1 in ratio)*/

	shared_type::scale_xyz_type volume_scale = { volume_local.volume_verts_max.x - volume_verts_min.x, volume_local.volume_verts_max.y - volume_verts_min.y, volume_local.volume_verts_max.z - volume_verts_min.z };

	volume_local.grid_amount.x = (unsigned short)((volume_scale.x / volume_local.intern_grid_spacing) + 1);
	volume_local.grid_amount.y = (unsigned short)((volume_scale.y / volume_local.intern_grid_spacing) + 1);
	volume_local.grid_amount.z = (unsigned short)((volume_scale.z / volume_local.intern_grid_spacing) + 1);

	unsigned short cmprt_size_m_5 = volume_local.intern_df_cmprt_size * 6;



	if ((volume_local.grid_amount.x < cmprt_size_m_5) ||
		(volume_local.grid_amount.y < cmprt_size_m_5) ||
		(volume_local.grid_amount.z < cmprt_size_m_5))
	{
		return 1;
	}

	volume_local.min_grid_coord = volume_verts_min;

	volume_local.max_grid_point.x = volume_local.grid_amount.x - 1;
	volume_local.max_grid_point.y = volume_local.grid_amount.y - 1;
	volume_local.max_grid_point.z = volume_local.grid_amount.z - 1;


	/*Defines relevent objects*/

	volume_local.cmprt_amount.x = ceil((double)volume_local.grid_amount.x / (double)volume_local.intern_df_cmprt_size);
	volume_local.cmprt_amount.y = ceil((double)volume_local.grid_amount.y / (double)volume_local.intern_df_cmprt_size);
	volume_local.cmprt_amount.z = ceil((double)volume_local.grid_amount.z / (double)volume_local.intern_df_cmprt_size);

	volume_local.cmprt_amount_total = (unsigned long)volume_local.cmprt_amount.x * (unsigned long)volume_local.cmprt_amount.y * (unsigned long)volume_local.cmprt_amount.z;


	/*Genereates Sphere Octant which will be used to generate cmprt rlvncy table later on in the function (it's done now so it can be used to estimate memory usage)*/
	/*The compartment relevancy table is a 2D lookup table, where each compartment has its own entry which contains an array of all the compartments that are able to be reached from within said
		compartment given the distance defined in intern_df_distance (essentially the distance of the distance field), ie, it contains all compartments that are "relevent" to the compartment.

	Eg, all the compartments in the below diagram shaded wit # are irrelevent to the center left compartment as they cannot be reached from within said compartment given the distancee in intern_df_distance
	_______________________________________
	#########|#########|#########|#########|
	#########|#########|#########|#########|
	#########|#########|#########|#########|
	#########|#########|#########|#########|
	         |         |#########|#########|
	    o    |         |#########|#########|
	    |    |    o    |#########|#########|
	____|____|___/_____|#########|#########|
	    |    |  /      |         |#########|
	    |    | /       |         |#########|
	    |    |/        |         |#########|
	____|____|_________|_________|#########|
	    |   /|         |         |#########|
	        _|_________|____o    |#########|
	         |         |         |#########|
	____|___\|_________|_________|#########|
	    |    |         |         |#########|
	    |    |\        |         |#########|
	    |    | \       |         |#########|
	____|____|__\______|_________|#########|
	    |    |   \     |#########|#########|
	    |    |    o    |#########|#########|
	    o    |         |#########|#########|
	         |         |#########|#########|
	#########|#########|#########|#########|
	#########|#########|#########|#########|
	#########|#########|#########|#########|
	#########|#########|#########|#########|


	The way that these relevency table is generated is by first generating an octant of the shape around compartment (0, 0, 0), eg:

	               |
	               |
	               |
	               |
	               |
	              / ^^/|
	             /___/ |____
	            |    | /   /|
	            |____|/___/ |___
	           /    /|    | /   ----____
	          /____/ |____|/            ----__
	          |    | /
	          | ___|/
	          /
	         /
	        /
	       /

	The the shape is then (in a very informal sense atleast), duplicated 8 times around each compartment in order to get a full voxelized sphere around each compartmnet
	(with the voxzelized sphere shape obvisously just being an array of compartment indices (again, each compartment has their own array of compartments which are relevent
	to it)). This octant only needs to be generated once, as the distance remains the same for every compartment, and so the same octant can be translated to, and duplicated
	around, each compartment. Also, as to why an octant of the shape is generated instead of a whole sphere, the sphere shape is identical on all axis, and so the octant only
	needs to be generated, as the full shape can then be gotten by just mirroring it on each axis.

	The compartment (0, 0, 0) is chosen as the center specifically as it makes it easiest to compute
	*/


	/*       Octant Generation
	        V        V        V        */


	/*Defines objects for keeping track of the next index in the array which will be allocated later on*/
	unsigned long rlvncy_sphere_octant_next_index = 0;
	unsigned long legacy_rlvncy_sphere_octant_next_index;

	/*Converts the world space distance of intern_df_distance into compartment index space, this is done because the way in which it is determined as to which compartment
		encloses a point in space is by truncating the coordinates of that point, which gives the index of the enclosing compartment (because we're working in index space).
		The  "- .00001" at the end is just to avoid floating point precision issues (was just concered about the possibility of a point that lies withing a compartment being
		mistaken as being in the next compartment up when truncating due to floating point inprecision (in the case that said point was very close to the edge of the compartment
		of course), this precaution may be pointless though, will need to research further as to weather or not it's necessary) */
		/*All geometry calculations from this point on (in the relevancy compartment generation at least) is in index space*/

	double df_distance_indx_space_flting_pnt = ((volume_local.intern_df_distance / volume_local.intern_grid_spacing) / volume_local.intern_df_cmprt_size) - .00001;

	/*The integral type variant of the above object is rounded to ceiling (does this by adding 1 then truncating)*/
	unsigned long df_distance_indx_space = (unsigned long)(df_distance_indx_space_flting_pnt + 1.0);


	/*The way in which the array of compartments that make up the octant is generated is through the rasterization technique of scanning across axis, in this case in increments
	  of 1 (as this is in compartment index space, 1 equates to a single compartment, so the scan is incrementing 1 compartment at a time). This scanning process is done by scanning
	  across the x axis, and at each increment, scanning up the z axis (eg, at x = 0, scans from (x = 0, z = 0) to (x = 0, z = n), then increments to x = 1, and then scanes from
	  (x = 1, z = 0) to (x = 1, z = n)) At each increment, the x and z values at that point are used to determine the y value in the spherical shape, and it is then tested wether or not
	  these (x, y, z) coordinates are on the surface of the spherical shape (eg, if they satisfy the equation fo the shape). If the coords to infact lie on the surface of the shape, then
	  the y coordiates are truncated (the x and z coords were already integers), and all 3 (x, y, z) coords are added to the array of compartments that make up the octant. If the point lies
	  on the surface, then, in addition to adding its enclosing compartment to the array, all the compartments  between the (x, y, z) and (x, 0, z) (with x, y, and z being the coords of the
	  compartment) are also added to the array of compartments that make up the octant, this done so that the compartments within the octant are also gotten, not just those it's the surface*/

	/*I should also note that even though I have been refering to the shape around each compartment as a sphere, it technically isn't exactly a sphere, and is actually a bit more complicated,
	If we were only measuring the distance from the centre of each comparemnt, then it would indeed be a sphere, however as the compartment contains a 3D array of grid points, each of which
	may have their distance to a mesh's tri taken, it must be assumed that if a compartment can be reached from anywhare within the current compartment, given the distance in intern_df_distance,
	then it is relevant, which means that distance must be measured from the outskirts of the compartment, as that gives the maximum reach from within said compartment. This results in more
	of a rounded cube like shape. The way that it is determined whether or not a point lies on this rounded cube like shape is to calculate the distance differently depending on where the
	point is positioned in relation to the cube, there are 3 seperate conditions for this:



	Keep in mind that the only coord that needs to be found while scanning is the y coord, as the scanning processess is essencitally moving across the z and x axis and finding how far to go in
	the y direction (essentially just finding the depth)

	Condition 1:

		If a set of x and z coords make it so that surface point must lie in the area closest to the furthest out vertex of the compartment (that is to say, if you got an octant of and actual sphere and placed
		it on the furthest out vertex of the compartment (0, 0, 0), then the point would be within this condition if it were in that octant), then the y coord of a particular x and z coord is gotten by
		finding the the y coord that would make the point lie on a sphere that was centered around the furthest out vertex of the compartment (0, 0, 0) (this involes checking wether or not a point actually
		lies on the sphere once the y value is known), and then truncating to get the compartment that encloses that point.



	             Z
	             |
	             |
	             |   _____
	             |  /     ^--.
	             | .          \
	             ||             \
	            / |              |
	           /_|               |
	           | |              /------------ Y
	           |_|         __ /
	           / |____..--^
	          /
	         /
	        X

	Condition 2 and 3:

		If a set of x and z coords make it so that a point must lie in the area closest to the edges, then the y coord is gotten by finding the y coord that makes the point lie on the cylinder created around the
		edge (this includes checking whether or not a point actually lies on the cylinder once the y coord is known)

	             Z
	             |     ____
	             |    /     ^--.
	             |   /____       \
	             |  |     ^--.      \
	             |  |           \    | <------- Condition 2
	             |__|             \  |
	            /   |              | /
	           /____|_____________ |/
	           |    /             .|-------- Y
	           |___/         __  / |
	           /  /____..--^      /
	          /   |          __ / <------- Condition 3
	         /    |____..--^
	        X

	Condition 4:

		If the point is lies directly to the side of the compartment, then the y coord of the compartment on the surface of the shape is simply the integer variant of df distance (in index space obviously)


	             Z
	             |
	             |
	             |___________________
	            /     /             /|
	           /____ /_____________/ |
	           |    |              | .------ Y
	           |____|______________|/
	           /
	          /
	         /
	        X
		*/

	/*Gets how far to scan along both the x and z axis (the shape is identical in size on both axis so the same max value can be used for both) */
	unsigned long end_index_p1 = df_distance_indx_space + 1;

	{
		unsigned short size_test_ushort = 0u;
		unsigned short size_ushort = sizeof(size_test_ushort);
		unsigned short size_ushort_m3 = (size_ushort * 3u);

		unsigned long long max_bytes = max_gb * 1073741824ull;
		unsigned long long max_bytes_d3 = max_bytes / size_ushort_m3;
		unsigned long long max_bytes_d3_sqr = std::cbrt(max_bytes_d3);
		if (end_index_p1 > max_bytes_d3_sqr)
		{
			return 3;
		}
	}

	/*	Defines a temporary array for the octant compartments to be stored in (temporary as in it will be deleted once the relevancy table is generated)
		this is the array that the aformentioned index counter objects were for keeping track of	*/
	shared_type::index_xyz_type* rlvncy_sphere_octant = new shared_type::index_xyz_type[end_index_p1 * end_index_p1 * end_index_p1];

	{		
		
		/*Gets the square of df distance so that it doesn't have to be recalculated every time the below loop iterates*/
		double df_distance_indx_space_flting_pnt_sqrd = df_distance_indx_space_flting_pnt * df_distance_indx_space_flting_pnt;


		/*a corrosponds to the x axis*/
		for (unsigned short a = 0; a < end_index_p1; ++a)
		{
			/*b corrosponds to the z axis*/
			for (unsigned short b = 0; b < end_index_p1; ++b)
			{
				/*First calculates the y coord of the sufrace point (as well as if the point is actually on the surface of the shape, depending on which condition is met)*/
				/*	V		V		V		V		V		V		V		V		V		V		V		V		V		V		V		V		V		V		V	  */

				/*Check first for condition 1*/
				if ((a != 0) && (b != 0))
				{
					double a_flting_pnt = a - 1;
					double b_flting_pnt = b - 1;

					double a_x_b = (a_flting_pnt * a_flting_pnt) + (b_flting_pnt * b_flting_pnt);

					double rlvnt_cmprt_y_index_flting_pnt = a_x_b - df_distance_indx_space_flting_pnt_sqrd;

					if (rlvnt_cmprt_y_index_flting_pnt < 0)
					{
						rlvnt_cmprt_y_index_flting_pnt = 0 - rlvnt_cmprt_y_index_flting_pnt;
					}

					double partial_sphere_formula_calc = (a_x_b + rlvnt_cmprt_y_index_flting_pnt);

					if ((partial_sphere_formula_calc < (df_distance_indx_space_flting_pnt_sqrd - .0001)) || (partial_sphere_formula_calc > (df_distance_indx_space_flting_pnt_sqrd + .0001)))
					{
						continue;
					}

					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].y = (unsigned short)(sqrt(rlvnt_cmprt_y_index_flting_pnt) + 1);
				}

				/*Then for Condition 2*/
				else if ((a == 0) && (b != 0))
				{
					double b_flting_pnt = b - 1;

					double rlvnt_cmprt_y_index_flting_pnt = (b_flting_pnt * b_flting_pnt) - df_distance_indx_space_flting_pnt_sqrd;

					if (rlvnt_cmprt_y_index_flting_pnt < 0)
					{
						rlvnt_cmprt_y_index_flting_pnt = 0 - rlvnt_cmprt_y_index_flting_pnt;
					}

					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].y = (unsigned short)(sqrt(rlvnt_cmprt_y_index_flting_pnt) + 1);
				}

				/*Then for Condition 3*/
				else if ((a != 0) && (b == 0))
				{
					double a_flting_pnt = a - 1;

					double rlvnt_cmprt_y_index_flting_pnt = (a_flting_pnt * a_flting_pnt) - df_distance_indx_space_flting_pnt_sqrd;

					if (rlvnt_cmprt_y_index_flting_pnt < 0)
					{
						rlvnt_cmprt_y_index_flting_pnt = 0 - rlvnt_cmprt_y_index_flting_pnt;
					}

					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].y = (unsigned short)(sqrt(rlvnt_cmprt_y_index_flting_pnt) + 1);
				}

				/*And lastly, if none of the above, then condition 4 is met*/
				else
				{
					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].y = df_distance_indx_space;
				}


				/*Then once the y coord is known, the x and z coords are added to current compartments entry in the array of compartmnts that make up the shape octant*/
				/*It is safe to assume that if this statment is reached, that the point within this compartment does indeed lie on the surface of the shape, as the
				  above conditions would have skipped the current loop iteration if it were not																		  */
				rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].x = a;
				rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].z = b;

				/*Loops through and adds each compartment between (x, 0, z) and (x, y, z) (with x, y, z being the coords of the surface compartment)*/
				legacy_rlvncy_sphere_octant_next_index = rlvncy_sphere_octant_next_index;
				for (unsigned short c = 0; c < rlvncy_sphere_octant[legacy_rlvncy_sphere_octant_next_index].y; ++c)
				{
					rlvncy_sphere_octant_next_index += 1;

					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].x = a;
					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].z = b;
					rlvncy_sphere_octant[rlvncy_sphere_octant_next_index].y = c;
				}

				rlvncy_sphere_octant_next_index += 1;
			}
		}
	}

	/*	Estimates roughly how much dynamically allocated memory will be used, and cancels initialization if extimate exceeds the hard coded max	*/
	{
		unsigned long grid_amount_total = volume_local.grid_amount.x * volume_local.grid_amount.y * volume_local.grid_amount.z;

		unsigned long long sphere_octant_m8 = rlvncy_sphere_octant_next_index * 8u;
		unsigned long long sphere_octant_m4 = rlvncy_sphere_octant_next_index * 4u;

		std::mutex size_test_mutex;
		unsigned long size_test_ulong = 0u;
		unsigned short size_test_ushort = 0u;
		std::vector<char> size_test_vec;
		char* size_test_ptr = nullptr;

		const unsigned short size_mutex = sizeof(size_test_mutex);
		const unsigned short size_ulong = sizeof(size_test_ulong);
		const unsigned short size_ushort = sizeof(size_test_ushort);
		const unsigned short size_vec = sizeof(size_test_vec);
		const unsigned short size_ptr = sizeof(size_test_ptr);

		const unsigned short size_grid_point =	(((	size_ptr + size_ulong) + size_ushort) + 10u) +
													size_vec +
													size_ushort +
													(size_ushort * 3u) +
													size_ptr +
													size_mutex;

		const unsigned long long size_grid = (volume_local.grid_amount.x * size_ptr) + ((volume_local.grid_amount.x * volume_local.grid_amount.y) * size_ptr) + (grid_amount_total * size_grid_point);
		const unsigned long long size_cmprt_table = (volume_local.cmprt_amount_total * size_ptr) + (grid_amount_total * size_ptr);
		const unsigned long long size_cmprt_meta = volume_local.cmprt_amount_total * (size_ulong * 2u);
		const unsigned long long rlvncy_table_edge_cmprts =		((volume_local.cmprt_amount.x * volume_local.cmprt_amount.y) * (2u * end_index_p1)) +
																((volume_local.cmprt_amount.x * volume_local.cmprt_amount.z) * (2u * end_index_p1)) +
																((volume_local.cmprt_amount.y * volume_local.cmprt_amount.z) * (2u * end_index_p1));
		const unsigned long long rlvncy_table_cmprts_full_total = (rlvncy_table_edge_cmprts * sphere_octant_m4) + ((volume_local.cmprt_amount_total - rlvncy_table_edge_cmprts) * sphere_octant_m8);
		const unsigned long long size_cmprt_rlvncy_table = (volume_local.cmprt_amount_total * size_ptr) + (rlvncy_table_cmprts_full_total * size_ulong);
		const unsigned long long size_cmprt_rlvncy_table_x_dims = (volume_local.cmprt_amount_total * size_ulong);
		const unsigned long long size_cmprt_rlvncy_table_buffer = (volume_local.cmprt_amount_total * size_ptr) + (volume_local.cmprt_amount_total * size_vec) + (rlvncy_table_cmprts_full_total * (size_ushort * 3u));

		const unsigned long long total_estimate =	size_grid +
													size_cmprt_table + 
													size_cmprt_meta + 
													size_cmprt_rlvncy_table + 
													size_cmprt_rlvncy_table_x_dims + 
													size_cmprt_rlvncy_table_buffer;
		/*
		std::cout << "Total Estimate: " << total_estimate << std::endl;
		std::cout << "size_grid: " << size_grid << std::endl;
		std::cout << "size_cmprt_table: " << size_cmprt_table << std::endl;
		std::cout << "size_cmprt_meta: " << size_cmprt_meta << std::endl;
		std::cout << "size_cmprt_rlvncy_table: " << size_cmprt_rlvncy_table << std::endl;
		std::cout << "end_index_p1: " << end_index_p1 << std::endl;
		std::cout << "size_cmprt_rlvncy_table_edge: " << rlvncy_table_edge_cmprts << std::endl;
		std::cout << "size_cmprt_rlvncy_table_x_dims: " << size_cmprt_rlvncy_table_x_dims << std::endl;
		std::cout << "size_cmprt_rlvncy_table_buffer: " << size_cmprt_rlvncy_table_buffer << std::endl;
		std::cout << "grid_type size" << sizeof(volume_local.grid[0][0][0]) << std::endl;
		*/

		const unsigned short total_estimate_gb = (total_estimate / 1073741824ull) + 1ull;
		if (total_estimate_gb > max_gb)
		{
			delete[] rlvncy_sphere_octant;
			return 3;
		}
	}

	/*Allocates 3D array (with df.grid pointing to it)*/
	/*-------------------------------------------------------------------------------------------------------------*/

	//Could potentially make this more efficient by setting the x, y, z values in the same loop nest as allocation


	/*Allocates*/
	volume_local.grid = new grid_type * *[volume_local.grid_amount.x];
	for (unsigned short a = 0; a < volume_local.grid_amount.x; ++a)
	{
		volume_local.grid[a] = new grid_type * [volume_local.grid_amount.y];
		for (unsigned short b = 0; b < volume_local.grid_amount.y; ++b)
		{
			volume_local.grid[a][b] = new grid_type[volume_local.grid_amount.z];
		}
	}

	/*Sets*/
	for (unsigned short a = 0; a < volume_local.grid_amount.x; ++a)
	{
		for (unsigned short b = 0; b < volume_local.grid_amount.y; ++b)
		{
			for (unsigned short c = 0; c < volume_local.grid_amount.z; ++c)
			{
				volume_local.grid[a][b][c].index.x = a;
				volume_local.grid[a][b][c].index.y = b;
				volume_local.grid[a][b][c].index.z = c;
			}
		}
	}


	/*Creates compartments*/
	/*-------------------------------------------------------------------------------------------------------------*/

	//Maybie dont search through the points you already searched through

	/*Dynamically allocates 2D array of pointers (the first dimension will be the list of compartments, and the 2nd dimension will contain each compartment's
	list of pointers to elements/points in the grid (the grid points contained in each compartment), eg:                                                     */

	/*-------------------------------------------------------------------------------*/
	/* Vertical Index 0 | grid point 0   | grid point 2   | grid point 3   | etc     */
	/*-------------------------------------------------------------------------------*/
	/* Vertical Index 1 | grid point 100 | grid point 101 | grid point 102 | etc     */      /*<------ Each vertical index corrosponds to a single compartment,*/
	/*-------------------------------------------------------------------------------*/
	/* Vertical Index 2 | grid point 200 | grid point 201 | grid point 202 | etc     */
	/*-------------------------------------------------------------------------------*/

	/*(note that the actual grid points have not been put into the array yet, this is
		just to show what is intended to be put in the array)                              */

	/*		V		V		V		V		V		V		V		V		V		 */

	unsigned short max_min_loop_index = 0;

	shared_type::index_xyz_type cmprt_slice_first_index;
	shared_type::index_xyz_type cmprt_slice_last_index;

	volume_local.cmprt_table = new grid_type * *[volume_local.cmprt_amount_total];

	/*Dynamically allocates a 1D array that will store meta info about each compartment*/
	volume_local.cmprt_meta = new cmprt_meta_type[volume_local.cmprt_amount_total];

	/*The large block below fills each compartment with pointers to the grid points in which it encloses*/

	/*The purpose of the 3 sets of if statements below is to get the index of both the max and min points of the current compartment
		on all 3 axis, so that when it comes time to loop through the points in the grid(in order to get each one's memory
		addresss and put it in the compartment lookup table (cmprt_table)), only the grid points that are within the compartment
		are iterated through. The else conditions are there because compartments on the outskirts of the grid may have different
		dimensions from the rest of the compartments and thus require special treatment (the amount of compartments may not divide
		evenly into the grids dimensions)*/

	for (unsigned short a = 0; a < volume_local.cmprt_amount.x; ++a)
	{
		if (a != (volume_local.cmprt_amount.x - 1))
		{
			cmprt_slice_last_index.x = (volume_local.intern_df_cmprt_size * (a + 1)) - 1;
			cmprt_slice_first_index.x = volume_local.intern_df_cmprt_size * (a);
		}
		else
		{

			cmprt_slice_first_index.x = (volume_local.intern_df_cmprt_size * (a));
			cmprt_slice_last_index.x = volume_local.grid_amount.x - 1;
		}


		for (unsigned short b = 0; b < volume_local.cmprt_amount.y; ++b)
		{
			if (b != (volume_local.cmprt_amount.y - 1))
			{
				cmprt_slice_last_index.y = (volume_local.intern_df_cmprt_size * (b + 1)) - 1;
				cmprt_slice_first_index.y = volume_local.intern_df_cmprt_size * (b);
			}

			else
			{

				cmprt_slice_first_index.y = (volume_local.intern_df_cmprt_size * (b));
				cmprt_slice_last_index.y = volume_local.grid_amount.y - 1;
			}

			for (unsigned short c = 0; c < volume_local.cmprt_amount.z; ++c)
			{
				if (c != (volume_local.cmprt_amount.z - 1))
				{
					cmprt_slice_last_index.z = (volume_local.intern_df_cmprt_size * (c + 1)) - 1;
					cmprt_slice_first_index.z = (volume_local.intern_df_cmprt_size * (c));
				}
				else
				{

					cmprt_slice_first_index.z = (volume_local.intern_df_cmprt_size * (c));
					cmprt_slice_last_index.z = volume_local.grid_amount.z - 1;
				}

				/*Converts 3D cmprt index into 1D index for compatability with the array pointed to by df_type.cmprt_table (which only allows 1D dimension for the compartments (being a 2D lookup table))*/
				unsigned long current_cmprt = (c + (volume_local.cmprt_amount.z * b)) + ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * a);

				//Sets the current compartment's equivalant element in cmprt_meta to its appropriate values (sets the position of 2 opposing vertieces of the compartment as well as the position of the center of the compartment*/
				grid_type* last_point = &(volume_local.grid[cmprt_slice_last_index.x][cmprt_slice_last_index.y][cmprt_slice_last_index.z]);
				grid_type* first_point = &(volume_local.grid[cmprt_slice_first_index.x][cmprt_slice_first_index.y][cmprt_slice_first_index.z]);

				/*Calculates the total amount of points in the current compartment*/
				unsigned long current_cmprt_size = ((cmprt_slice_last_index.x + 1) - cmprt_slice_first_index.x) * ((cmprt_slice_last_index.y + 1) - cmprt_slice_first_index.y) * ((cmprt_slice_last_index.z + 1) - cmprt_slice_first_index.z);

				/*Adds the amount of points in the current compartment to this compartments entry in cmprt_meta (this is needed to be able to know the size of each compartments array in the compartment lookup table (cmprt_table))*/
				volume_local.cmprt_meta[current_cmprt].size = current_cmprt_size;


				/*Allocates space in the compartment lookup table for the current compartments list of enclosed points.
					As implied above, the amount of grid points in each compartment can differ, and so each compartments
					respectice array in the compartment lookup table is allocated here rather than furhter up with the first
					dimensions allocation, so that each compartments array can have a different size, eg*/
					/*
						|	cmprt_0 | @ @ @ @ @ @ @ @ @ |
						|	cmprt_1 | @ @ @ @ @ @ @ @ @ |
						|	cmprt_2 | @ @ @ @ @ @ @ @ @ |
						|	cmprt_3 | @ @ @ @ @ @ @ @ @ |
						|	cmprt_4 | @ @ @ @ @ @ @ @ @ |
						|	cmprt_5 | @ @ @ @           |
					*/
				volume_local.cmprt_table[current_cmprt] = new grid_type * [current_cmprt_size];

				/*Iterates through each grid point enclosed within the current compartment, adding their memory address to the current compartments lookup table entry*/
				for (unsigned short d = cmprt_slice_first_index.x; d < (cmprt_slice_last_index.x + 1); ++d)
				{
					for (unsigned short e = cmprt_slice_first_index.y; e < (cmprt_slice_last_index.y + 1); ++e)
					{
						for (unsigned short f = cmprt_slice_first_index.z; f < (cmprt_slice_last_index.z + 1); ++f)
						{
							/*Converts grid point index to be relative to the current compartment*/
							unsigned short d_r = d - cmprt_slice_first_index.x;
							unsigned short e_r = e - cmprt_slice_first_index.y;
							unsigned short f_r = f - cmprt_slice_first_index.z;

							/*Converts the the index of the last grid point enclosed within the current compartment*/
							shared_type::index_xyz_type cmprt_slice_last_index_r;
							cmprt_slice_last_index_r.x = cmprt_slice_last_index.x - cmprt_slice_first_index.x;
							cmprt_slice_last_index_r.y = cmprt_slice_last_index.y - cmprt_slice_first_index.y;
							cmprt_slice_last_index_r.z = cmprt_slice_last_index.z - cmprt_slice_first_index.z;

							/*The conversions to relative space are needed for the below calculation to work*/
							/*	V		V		V		V		V		V		V		V		V		V	*/

							/*Converts the current grid point index (a 3D index) into a 1D index for compatability with df_type::cmprt_table (which only allows 1 dimensions for the grid points (again, being a 2D lookup table))*/
							unsigned long current_point = (f_r + ((cmprt_slice_last_index_r.z + 1) * e_r)) + (((cmprt_slice_last_index_r.z + 1) * (cmprt_slice_last_index_r.y + 1)) * d_r);

							/*Fills current compartment with the grid points it encloses*/
							volume_local.cmprt_table[current_cmprt][current_point] = &(volume_local.grid[d][e][f]);
						}
					}
				}
			}
		}
	}


	/*Creates compartment relevency table*/
	/*-------------------------------------------------------------------------------------------------------------*/


	/*Now that the octant has been generated, the below code then loops through each compartment in the df volume, and translates the octant to it, and mirrors it on each axis to create a full
	  shape around the compartment (with "creates shape" obviously being a high level way of saying that it adds the compartments around it that would be contained within the shape to that
	  compartments respective array of relevant compartments). It also checks if each compartment in the shape actually fits within the volume before adding it, obviously you can't create a full
	  shape around a compartment that is on the edge of the volume*/

	  /*Defines a buffer for storing the relevency table (this is done as the actual size of the relevancy table is not yet known)*/
	std::vector<shared_type::index_xyz_type>** cmprt_rlvncy_table_buffer = new std::vector<shared_type::index_xyz_type>*[volume_local.cmprt_amount_total];
	unsigned long cmprt_rlvncy_table_buffer_y_next_index = 0;

	/*Creates an array for storing how large each compartments respective array in the relevancy table is*/
	unsigned long* cmprt_rlvncy_table_buffer_x_dims = new unsigned long[volume_local.cmprt_amount_total];

	for (unsigned short a = 0; a < volume_local.cmprt_amount.x; ++a)
	{
		for (unsigned short b = 0; b < volume_local.cmprt_amount.y; ++b)
		{
			for (unsigned short c = 0; c < volume_local.cmprt_amount.z; ++c)
			{
				/*Creates an vector for the current compartment in the relevancy table buffer*/
				cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index] = new std::vector<shared_type::index_xyz_type>();
				unsigned long cmprt_rlvncy_table_buffer_x_next_index = 0;
				


				/*The below process translates each compartment in the octant around the current compartment, and checks if it fits within the volume. Because the compartments in the array of
				octants were added in the order that they were scanned (on the x axis), it can be assumed that if a compartment does not fit within the volume (on the x axis atleast), then
				all the compartments after it will not either, as they will only go further out on the x axis from the centre, and so the below flags "check_x_p" and "check_x_n" exist to flag
				if the bounds of the volume have been reached on either the positive x direction or the negative x direction, and they are checked before each compartment is processed in order
				to avoid wasting time on pointlessly processing compartments that are garenteed to not fit within the volume)*/

				bool check_x_p = true;
				bool check_x_n = true;

				for (unsigned long d = 0; d < rlvncy_sphere_octant_next_index; ++d)
				{
					shared_type::index_xyz_type sphr_oct_cmprt_trnsltn_ppp;
					shared_type::index_xyz_signed_type sphr_oct_cmprt_trnsltn_nnn;

					sphr_oct_cmprt_trnsltn_ppp.x = a + rlvncy_sphere_octant[d].x;
					sphr_oct_cmprt_trnsltn_ppp.y = b + rlvncy_sphere_octant[d].y;
					sphr_oct_cmprt_trnsltn_ppp.z = c + rlvncy_sphere_octant[d].z;

					sphr_oct_cmprt_trnsltn_nnn.x = (int)a - (int)rlvncy_sphere_octant[d].x;
					sphr_oct_cmprt_trnsltn_nnn.y = (int)b - (int)rlvncy_sphere_octant[d].y;
					sphr_oct_cmprt_trnsltn_nnn.z = (int)c - (int)rlvncy_sphere_octant[d].z;

					if (check_x_p == true)
					{
						if (sphr_oct_cmprt_trnsltn_ppp.x < volume_local.cmprt_amount.x)
						{
							if (sphr_oct_cmprt_trnsltn_ppp.y < volume_local.cmprt_amount.y)
							{
								if (sphr_oct_cmprt_trnsltn_ppp.z < volume_local.cmprt_amount.z)
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(sphr_oct_cmprt_trnsltn_ppp);
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}

								if ((sphr_oct_cmprt_trnsltn_nnn.z >= 0) && (rlvncy_sphere_octant[d].z != 0))
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = sphr_oct_cmprt_trnsltn_ppp.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = sphr_oct_cmprt_trnsltn_ppp.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}
							}

							if ((sphr_oct_cmprt_trnsltn_nnn.y >= 0) && (rlvncy_sphere_octant[d].y != 0))
							{
								if (sphr_oct_cmprt_trnsltn_ppp.z < volume_local.cmprt_amount.z)
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = sphr_oct_cmprt_trnsltn_ppp.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = sphr_oct_cmprt_trnsltn_ppp.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}

								if ((sphr_oct_cmprt_trnsltn_nnn.z >= 0) && (rlvncy_sphere_octant[d].z != 0))
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = sphr_oct_cmprt_trnsltn_ppp.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}
							}
						}
						else
						{
							check_x_p = false;
						}
					}

					if ((check_x_n == true) && (rlvncy_sphere_octant[d].x != 0))
					{
						if (sphr_oct_cmprt_trnsltn_nnn.x >= 0)
						{

							if (sphr_oct_cmprt_trnsltn_ppp.y < volume_local.cmprt_amount.y)
							{
								if (sphr_oct_cmprt_trnsltn_ppp.z < volume_local.cmprt_amount.z)
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = sphr_oct_cmprt_trnsltn_ppp.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = sphr_oct_cmprt_trnsltn_ppp.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}

								if ((sphr_oct_cmprt_trnsltn_nnn.z >= 0) && (rlvncy_sphere_octant[d].z != 0))
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = sphr_oct_cmprt_trnsltn_ppp.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}
							}

							if ((sphr_oct_cmprt_trnsltn_nnn.y >= 0) && (rlvncy_sphere_octant[d].y != 0))
							{
								if (sphr_oct_cmprt_trnsltn_ppp.z < volume_local.cmprt_amount.z)
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = sphr_oct_cmprt_trnsltn_ppp.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}

								if ((sphr_oct_cmprt_trnsltn_nnn.z >= 0) && (rlvncy_sphere_octant[d].z != 0))
								{
									/*Adds translated octant compartment to table*/
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->push_back(shared_type::index_xyz_type());
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).x = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.x;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).y = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.y;
									cmprt_rlvncy_table_buffer[cmprt_rlvncy_table_buffer_y_next_index]->at(cmprt_rlvncy_table_buffer_x_next_index).z = (unsigned short)sphr_oct_cmprt_trnsltn_nnn.z;
									cmprt_rlvncy_table_buffer_x_next_index += 1;
								}
							}
						}
						else
						{
							check_x_n = false;
						}
					}
				}

				cmprt_rlvncy_table_buffer_x_dims[cmprt_rlvncy_table_buffer_y_next_index] = cmprt_rlvncy_table_buffer_x_next_index;

				cmprt_rlvncy_table_buffer_y_next_index += 1;
			}
		}
	}


	/*Now that the actual size of the compartment relevancy table is known, creates final relevancy table structure and transfers from buffer to it*/
	/*This is probably unecessary, fix this*/
	volume_local.cmprt_rlvncy_table = new unsigned long* [volume_local.cmprt_amount_total];

	for (unsigned long a = 0; a < volume_local.cmprt_amount_total; ++a)
	{
		volume_local.cmprt_rlvncy_table[a] = new unsigned long[cmprt_rlvncy_table_buffer_x_dims[a]];

		for (unsigned long b = 0; b < cmprt_rlvncy_table_buffer_x_dims[a]; ++b)
		{
			unsigned long current_rlvnt_cmprt_linear = (cmprt_rlvncy_table_buffer[a]->at(b).z + (volume_local.cmprt_amount.z * cmprt_rlvncy_table_buffer[a]->at(b).y)) + ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * cmprt_rlvncy_table_buffer[a]->at(b).x);

			volume_local.cmprt_rlvncy_table[a][b] = current_rlvnt_cmprt_linear;

			volume_local.cmprt_meta[a].rlvncy_table_size = cmprt_rlvncy_table_buffer_x_dims[a];
		}
	}

	
	/*Cleans up dynamically allocated memory structures*/
	volume_local_type::delete_rlvncy_buffers_shared_local_type delete_rlvncy_buffers_shared_local;
	{
		delete_rlvncy_buffers_shared_local.args = new volume_local_type::delete_rlvncy_buffers_local_type * [thread_pool.thread_count];
		unsigned long min_cmprts_per_thread = (volume_local.cmprt_amount_total / thread_pool.thread_count);
		for (unsigned long a = 0u; a < thread_pool.thread_count; ++a)
		{
			delete_rlvncy_buffers_shared_local.args[a] = new volume_local_type::delete_rlvncy_buffers_local_type();
			delete_rlvncy_buffers_shared_local.args[a]->vector_arr = cmprt_rlvncy_table_buffer;

			delete_rlvncy_buffers_shared_local.args[a]->start_index = min_cmprts_per_thread * a;
			if (a < (thread_pool.thread_count - 1))
			{
				delete_rlvncy_buffers_shared_local.args[a]->end_next_index = min_cmprts_per_thread * (a + 1);
			}
			else
			{
				delete_rlvncy_buffers_shared_local.args[a]->end_next_index = volume_local.cmprt_amount_total;
			}
		}

		typedef void (*jobs)(void*, unsigned short);
		jobs jobs_ptr = &call_delete_rlvncy_buffers;

		bool add_jobs_success = false;
		bool do_once = true;
		/*Attempts to sends batch of jobs to thread pool*/
		thread_pool.set_jobs_per_iteration(1u);
	attempt_to_add_job_batch:
		add_jobs_success = thread_pool.add_jobs(jobs_ptr, thread_pool.thread_count, &delete_rlvncy_buffers_shared_local);

		if (do_once)
		{
			delete[] cmprt_rlvncy_table_buffer_x_dims;
			delete[] rlvncy_sphere_octant;
			do_once = false;
		}

		/*Checks if attempt to add jobs was successfull*/
		if (add_jobs_success == true)
		{
			goto batch_sent;
		}
		else
		{
			goto wait_for_space_in_job_stack;
		}

	wait_for_space_in_job_stack:
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));

			if (thread_pool.job_amount < 10)
			{
				goto attempt_to_add_job_batch;
			}
		}


	batch_sent:


		while (true)
		{
			delete_rlvncy_buffers_shared_local.token.lock();
			if (delete_rlvncy_buffers_shared_local.jobs_completed >= thread_pool.thread_count)
			{
				delete_rlvncy_buffers_shared_local.token.unlock();
				goto jobs_completed;
			}
			delete_rlvncy_buffers_shared_local.token.unlock();
			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
		}
	jobs_completed:

		delete[] delete_rlvncy_buffers_shared_local.args;

	}


	return 0;
}


int df_type::pre_update(const unsigned long* dfc_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount_total, const unsigned long* ignored_dfcs, const unsigned long ignored_dfcs_nxt_indx)
{
	update_local.clean();

	/*	Moves contents of existing dfc cache to legacy cache (at a macro level, this is done so that the last dfc cache state can be compared to the one about to be initialized). 
		After this is done, the new cache is then initialized (mesh data, such as verts and tris, are not added to the cache in this stage, this is done in the member function
		"add_dfc_to_cache", which is called after "pre_update")	*/
	update_local.dfc_cache.move_to_legacy();
	update_local.dfc_cache.initialize_cache(dfc_ids, dfc_amount, vert_amount_total);

	/*	Checks if any dfcs that existed in the legacy dfc cache do not exist in the current dfc cache, if so, all grid point mesh layers belonging to that dfc are removed	*/
	if (update_local.dfc_cache.legacy_is_valid == true)
	{
		unsigned long existing_dfcs_amount = 0;
		for (unsigned long a = 0; a < update_local.dfc_cache.dfc_amount_legacy; ++a)
		{
			if (dfc_cache.dfc_indx_to_id_table_legacy[a] != nullptr)
			{
				/*	Sets alias	*/
				update_local_type::dfc_cache_type& dfc_cache = update_local.dfc_cache;

				/*	Checks if the amount of dfcs that have been found to have carried over to the current dfc (those that exist in both the legacy, and current, cache) is equal
					to the amount of dfcs in the current cache, as obviously, if so, there is no point checking if any remaining dfcs in the legacy cache exist in the current
					cache, as all of the dfcs in the current cache have been found already	*/
				if (existing_dfcs_amount < dfc_cache.dfc_amount)
				{
					/*	Loop through all dfcs in the current dfc cache, and compare to the current dfc in the legacy cache	*/
					for (unsigned long b = 0; b < dfc_cache.dfc_amount; ++b)
					{
						if (dfc_cache.dfc_indx_to_id_table_legacy[a]->id == dfc_cache.dfc_indx_to_id_table[b]->id)
						{
							dfc_cache.existing_dfcs[b] = true;
							existing_dfcs_amount += 1;
							goto if_exists;
						}
					}
					goto if_doesnt_exist;

				if_exists:

					continue;
				}

				/*	If the current dfc in the legacy cache does not exist in the current cache, remove all grid point mesh layers	*/
			if_doesnt_exist:

				/*	Removes mesh layers for every mesh within the current dfc	*/
				unsigned long mesh_cache_legacy_y_size = dfc_cache.mesh_cache_legacy[a]->size();
				for (unsigned long b = 0; b < mesh_cache_legacy_y_size; ++b)
				{
					dfc_cache.remove_mesh_layers(dfc_cache.dfc_indx_to_id_table_legacy[a]->id, b, true, volume_local);
				}

				for (unsigned long b = 0u; b < ignored_dfcs_nxt_indx; ++b)
				{
					if (dfc_cache.dfc_indx_to_id_table_legacy[a]->id == ignored_dfcs[0])
					{
						goto exists_in_ignored;
					}
				}
				goto doesnt_exist_in_ignored;

			exists_in_ignored:

				continue;

			doesnt_exist_in_ignored:

				/*	Checks if dfc is still within dfc layers (this can happen if a dfc is deleted before being removed from it's respective layers)	*/
				unsigned long dfc_layer_amount = dfc_layers.size();
				for (unsigned long b = 0u; b < dfc_layer_amount; ++b)
				{
					if (get_dfc_in_dfc_layer(b, dfc_cache.dfc_indx_to_id_table_legacy[a]->id) != nullptr)
					{
						unsigned long temp_arg = 1ul;
						remove_dfcs_from_dfc_layer(b, &dfc_cache.dfc_indx_to_id_table_legacy[a]->id, temp_arg, false);
					}
				}

				/*	Delets dfc	*/
				dfc_id_indx_type** moved_dfc = update_local.dfc_cache.dfc_ids.remove_element(dfc_cache.dfc_indx_to_id_table_legacy[a]->index);
				if (moved_dfc != nullptr)
				{
					(*moved_dfc)->index = dfc_cache.dfc_indx_to_id_table_legacy[a]->index;
				}
				delete dfc_cache.dfc_indx_to_id_table_legacy[a];
				dfc_cache.dfc_indx_to_id_table_legacy[a] = nullptr;
			}
		}
	}

	{
		unsigned long* current_dfcs = new unsigned long[dfc_cache.dfc_amount];
		for (unsigned long a = 0u; a < dfc_cache.dfc_amount; ++a)
		{
			current_dfcs[a] = dfc_cache.dfc_indx_to_id_table[a]->id;
		}

		remove_deleted_element_from_layer_system<std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>>(dfc_layers, &df_type::remove_dfcs_from_dfc_layer, current_dfcs, dfc_cache.dfc_amount, ignored_dfcs, ignored_dfcs_nxt_indx);
	}

	thread_pool.set_jobs_per_iteration(23u);

	return 0;
}


/*This function may be called on multiple threads at once*/
int df_type::update_grid_points(void* arg_ptr, unsigned short job_index)
{
	tri_local_type& current_tri_local_state = *(tri_local_type*)arg_ptr;
	update_local_type::dfc_cache_type& dfc_cache = update_local.dfc_cache;
	mesh_local_type& current_mesh_local_state = *current_tri_local_state.mesh_local_state;

	for (unsigned long a = 0; a < current_tri_local_state.job_cmprt_table_next_index_y; ++a)
	{
		unsigned long& grid_points_first_index = current_tri_local_state.job_cmprt_table_cropped[a][1 + (job_index * 2)];
		unsigned long grid_points_last_index_p1 = current_tri_local_state.job_cmprt_table_cropped[a][2 + (job_index * 2)] + 1;

		/*Loops through each grid point in the current relevent compartment*/
		for (unsigned long b = grid_points_first_index; b < grid_points_last_index_p1; ++b)
		{
			unsigned long& current_cmprt = current_tri_local_state.job_cmprt_table_cropped[a][0];

			/*Sets alias*/
			grid_type*& current_grid_point = volume_local.cmprt_table[current_cmprt][b];
			shared_type::coord_xyz_type current_grid_point_coord;
			current_grid_point_coord.x = volume_local.min_grid_coord.x + (volume_local.intern_grid_spacing * (double)current_grid_point->index.x);
			current_grid_point_coord.y = volume_local.min_grid_coord.y + (volume_local.intern_grid_spacing * (double)current_grid_point->index.y);
			current_grid_point_coord.z = volume_local.min_grid_coord.z + (volume_local.intern_grid_spacing * (double)current_grid_point->index.z);

			/*Projects current grid point into the plane created by the current triangle*/
			/*-------------------------------------------------------------------------------------------------------------*/

			/*Gets distance from grid point to plane, d1 = dot(normal, grid point) + d0*/
			double pnt_plne_dist = (current_tri_local_state.normal.x * current_grid_point_coord.x) + (current_tri_local_state.normal.y * current_grid_point_coord.y) + (current_tri_local_state.normal.z * current_grid_point_coord.z) + update_local.dfc_cache.tri_cache[update_local.dfc_index][current_tri_local_state.tri_index].d;

			shared_type::coord_xyz_type nearest_plane_point;

			/*Translates grid point along inverse of normal by the amount defined by plane d, resulting the nearest point on the plane*/
			nearest_plane_point.x = current_grid_point_coord.x + ((pnt_plne_dist * -1.0) * current_tri_local_state.normal.x);
			nearest_plane_point.y = current_grid_point_coord.y + ((pnt_plne_dist * -1.0) * current_tri_local_state.normal.y);
			nearest_plane_point.z = current_grid_point_coord.z + ((pnt_plne_dist * -1.0) * current_tri_local_state.normal.z);


			/*Converts nearest_plane_point from cartesian coordinates to barycentric coordinates, this allows one to quickly findout where the current grid point's
			  projected point on the plane made by the triange lies realtive to the triangle (this is important for finding the distance)							*/
			  /*-------------------------------------------------------------------------------------------------------------*/

			shared_type::coord_uvw_type nearest_plane_point_bc = shared.cartesian_to_barycentric(*current_tri_local_state.vert_0, *current_tri_local_state.vert_1, *current_tri_local_state.vert_2, nearest_plane_point, current_tri_local_state.normal);


			double distance = volume_local.intern_df_distance;


			/*Dertermines how to, and then calculates, distance*/
			/*-------------------------------------------------------------------------------------------------------------*/

			/*The below if and else if statements determine where the projected point's position in regard to the triangle, now that the position of the projected point is in
			  barycentric coordinates, it can now quickly be dertermined which of the below areas the projected point lies within, this must be known as the distance will need
			  to be calculated differently depending on where it is located.																															*/
			  /*
		                                        \    Vert     /
		                                           \       /
		                                              \/
		                                              /\
		                                             /  \
		                                            /    \
		                                           /      \
		                                Edge      /        \     Edge
		                                         /          \
		                                        /            \
		                                       /              \
		                                      /    Triangle    \
		                                     /                  \
		                          __        /                    \        __
		                            ----___/______________________\___----
		                                   |                      |
		                            Vert   |         Edge         |   Vert
		                                   |                      |
			  */


			  /*If the projected point is within the triangle, then the distance is just the previously calculated perpendicular distance between the current grid points position and the plane created by the triangle*/
			  /*This is the case if all 3 coordinates (u, v, and w) are greater than 0*/
			if ((nearest_plane_point_bc.u >= 0) && (nearest_plane_point_bc.v >= 0) && (nearest_plane_point_bc.w >= 0))
			{
				//distance = (volume_local.intern_df_distance / 8.0) * 7.0;

				distance = pnt_plne_dist * pnt_plne_dist;
			}

			/*The next 3 else if statemtnts test if the projected point is within the vert areas pictured above, if so then the distance is just the distance from the current grid point (in world space) to
			  the nearest vertex																																												*/

			  /*If both the v and w coordinates are below 0, then the point must lie within the vert area closest to the vert represented by u (current_tri_local_state.vert_0)*/
			else if ((nearest_plane_point_bc.v < 0) && (nearest_plane_point_bc.w < 0))
			{
			vert_0_distance:

				//distance = (volume_local.intern_df_distance / 8.0) * 6.0;

				double distance_x = current_tri_local_state.vert_0->x - current_grid_point_coord.x;
				double distance_y = current_tri_local_state.vert_0->y - current_grid_point_coord.y;
				double distance_z = current_tri_local_state.vert_0->z - current_grid_point_coord.z;

				distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
			}

			/*If both the u and w coordinates are below 0, then the point must lie within the vert area closest to the vert represented by v (current_tri_local_state.vert_1)*/
			else if ((nearest_plane_point_bc.u < 0) && (nearest_plane_point_bc.w < 0))
			{
			vert_1_distance:

				//distance = (volume_local.intern_df_distance / 8.0) * 5.0;

				double distance_x = current_tri_local_state.vert_1->x - current_grid_point_coord.x;
				double distance_y = current_tri_local_state.vert_1->y - current_grid_point_coord.y;
				double distance_z = current_tri_local_state.vert_1->z - current_grid_point_coord.z;

				distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
			}

			/*If both the u and v coordinates are below 0, then the point must lie within the vert area closest to the vert represented by w (current_tri_local_state.vert_2)*/
			else if ((nearest_plane_point_bc.u < 0) && (nearest_plane_point_bc.v < 0))
			{
			vert_2_distance:

				//distance = (volume_local.intern_df_distance / 8.0) * 4.0;

				double distance_x = current_tri_local_state.vert_2->x - current_grid_point_coord.x;
				double distance_y = current_tri_local_state.vert_2->y - current_grid_point_coord.y;
				double distance_z = current_tri_local_state.vert_2->z - current_grid_point_coord.z;

				distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
			}

			/*The below 3 else if statements test if the projected point is within the edge areas pictured above, if so then the distance is the perpendicular distance
			  from the current grid point (in world space) to the edge
			*/
			/*It must be noted that there are also special cases which are also tested for (withing the statemtents compounds statment) as well, this is because the
			above vert areas tests test for vert areas whose bounds are not perpendicular with the edges of the triangle, as the vert area is only within the confines
			of the over/under shoots of the edges, which (with the exception of a single vertex on a right angled triangle) result in areas that look like this:

			   /\
			  /  \
			 /    \
			       \              \     /
			________\________       \ /
			         \              / \
			          \           /     \
			           \

			Instead of this:

			   /\
			  /  \                       /
			 /    \                     /
			       \       ___      \  /
			________\___---          \/
			        |
			        |
			        |

			This leads to visual artifacting, and so before the below algorithms preceed to calculate the distance, they first check if that projected point is actually on the edge,
			if it is not, then the flow is redirected back up to the above vert distance calc algorithms.
			(They way the algoithm checks to see if the point is on the edge or not is to use the distance between one vertices, and the projected point (which is already calculated
			in order to be able to calculate the distance), and see if it is either below 0, or greater than the distance between said vert and the other vert that makes up the edge)
			*/

			else if ((nearest_plane_point_bc.u < 0) && (nearest_plane_point_bc.v >= 0) && (nearest_plane_point_bc.w >= 0))
			{
				shared_type::coord_xyz_type dist_1_point;
				double dist_1_ppoint;

				/*Gets distance vector from current_tri_local_state.vert_1 to current grid point*/
				dist_1_point.x = current_grid_point_coord.x - current_tri_local_state.vert_1->x;
				dist_1_point.y = current_grid_point_coord.y - current_tri_local_state.vert_1->y;
				dist_1_point.z = current_grid_point_coord.z - current_tri_local_state.vert_1->z;

				/*Gets distance between current_tri_local_state.vert_1 and the current grid point's projected point onto the line between current_tri_local_state.vert_1 and current_tri_local_state.vert_2 (does a dot product)*/
				dist_1_ppoint = (dist_1_point.x * current_tri_local_state.unit_1_2.x) + (dist_1_point.y * current_tri_local_state.unit_1_2.y) + (dist_1_point.z * current_tri_local_state.unit_1_2.z);

				/*Checks if projected point actually lies on the edge, if not, redirects flow back up to vert area algorithms*/
				if (dist_1_ppoint < 0)
				{
					goto vert_1_distance;
				}
				else if (dist_1_ppoint > current_tri_local_state.distance_1_2)
				{
					goto vert_2_distance;
				}

				/*If the point is inside of the edge, then calc the distance as usual*/
				else
				{
					shared_type::coord_xyz_type ppoint;

					/*Gets position of projected point by translating current_tri_local_state.vert_1 in the direction of current_tri_local_state.unit_1_2 by the distance given by dist_1_ppoint*/
					ppoint.x = current_tri_local_state.vert_1->x + (dist_1_ppoint * current_tri_local_state.unit_1_2.x);
					ppoint.y = current_tri_local_state.vert_1->y + (dist_1_ppoint * current_tri_local_state.unit_1_2.y);
					ppoint.z = current_tri_local_state.vert_1->z + (dist_1_ppoint * current_tri_local_state.unit_1_2.z);


					/*Calculates distance between current grid point and projected point*/

					double distance_x = ppoint.x - current_grid_point_coord.x;
					double distance_y = ppoint.y - current_grid_point_coord.y;
					double distance_z = ppoint.z - current_grid_point_coord.z;

					distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
				}
			}
			else if ((nearest_plane_point_bc.v < 0) && (nearest_plane_point_bc.u >= 0) && (nearest_plane_point_bc.w >= 0))
			{
				shared_type::coord_xyz_type dist_0_point;
				double dist_0_ppoint;

				/*Gets distance vector from current_tri_local_state.vert_0 to current grid point*/
				dist_0_point.x = current_grid_point_coord.x - current_tri_local_state.vert_0->x;
				dist_0_point.y = current_grid_point_coord.y - current_tri_local_state.vert_0->y;
				dist_0_point.z = current_grid_point_coord.z - current_tri_local_state.vert_0->z;

				/*Gets distance between current_tri_local_state.vert_0 and the current grid point's projected point onto the line between current_tri_local_state.vert_0 and current_tri_local_state.vert_2 (does a dot product)*/
				dist_0_ppoint = (dist_0_point.x * current_tri_local_state.unit_0_2.x) + (dist_0_point.y * current_tri_local_state.unit_0_2.y) + (dist_0_point.z * current_tri_local_state.unit_0_2.z);

				/*Checks if projected point actually lies on the edge, if not, redirects flow back up to vert area algorithms*/
				if (dist_0_ppoint < 0)
				{
					goto vert_0_distance;
				}
				else if (dist_0_ppoint > current_tri_local_state.distance_0_2)
				{
					goto vert_2_distance;
				}

				/*If the point is inside of the edge, then calc the distance as usual*/
				else
				{
					shared_type::coord_xyz_type ppoint;

					/*Gets position of projected point by translating current_tri_local_state.vert_0 in the direction of current_tri_local_state.unit_0_2 by the distance given by dist_0_ppoint*/
					ppoint.x = current_tri_local_state.vert_0->x + (dist_0_ppoint * current_tri_local_state.unit_0_2.x);
					ppoint.y = current_tri_local_state.vert_0->y + (dist_0_ppoint * current_tri_local_state.unit_0_2.y);
					ppoint.z = current_tri_local_state.vert_0->z + (dist_0_ppoint * current_tri_local_state.unit_0_2.z);

					/*Calculates distance between current grid point and projected point*/

					double distance_x = ppoint.x - current_grid_point_coord.x;
					double distance_y = ppoint.y - current_grid_point_coord.y;
					double distance_z = ppoint.z - current_grid_point_coord.z;

					distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
				}
			}
			else if ((nearest_plane_point_bc.w < 0) && (nearest_plane_point_bc.u >= 0) && (nearest_plane_point_bc.v >= 0))
			{
				shared_type::coord_xyz_type dist_0_point;
				double dist_0_ppoint;

				/*Gets distance vector from current_tri_local_state.vert_0 to current grid point*/
				dist_0_point.x = current_grid_point_coord.x - current_tri_local_state.vert_0->x;
				dist_0_point.y = current_grid_point_coord.y - current_tri_local_state.vert_0->y;
				dist_0_point.z = current_grid_point_coord.z - current_tri_local_state.vert_0->z;

				/*Gets distance between current_tri_local_state.vert_0 and the current grid point's projected point onto the line between current_tri_local_state.vert_0 and current_tri_local_state.vert_1 (does a dot product)*/
				dist_0_ppoint = (dist_0_point.x * current_tri_local_state.unit_0_1.x) + (dist_0_point.y * current_tri_local_state.unit_0_1.y) + (dist_0_point.z * current_tri_local_state.unit_0_1.z);


				/*Checks if projected point actually lies on the edge, if not, redirects flow back up to vert area algorithms*/
				if (dist_0_ppoint < 0)
				{
					goto vert_0_distance;
				}
				else if (dist_0_ppoint > current_tri_local_state.distance_0_1)
				{
					goto vert_1_distance;
				}

				/*If the point is inside of the edge, then calc the distance as usual*/
				else
				{
					shared_type::coord_xyz_type ppoint;

					/*Gets position of projected point by translating current_tri_local_state.vert_0 in the direction of current_tri_local_state.unit_0_1 by the distance given by dist_0_ppoint*/
					ppoint.x = current_tri_local_state.vert_0->x + (dist_0_ppoint * current_tri_local_state.unit_0_1.x);
					ppoint.y = current_tri_local_state.vert_0->y + (dist_0_ppoint * current_tri_local_state.unit_0_1.y);
					ppoint.z = current_tri_local_state.vert_0->z + (dist_0_ppoint * current_tri_local_state.unit_0_1.z);

					/*Calculates distance between current grid point and projected point*/

					double distance_x = ppoint.x - current_grid_point_coord.x;
					double distance_y = ppoint.y - current_grid_point_coord.y;
					double distance_z = ppoint.z - current_grid_point_coord.z;

					distance = (distance_x * distance_x) + (distance_y * distance_y) + (distance_z * distance_z);
				}
			}

			/*Converts the distance to a range between 0 and 1, and inverts it*/
			distance = 1 - ((1 / (volume_local.intern_df_distance * volume_local.intern_df_distance)) * distance);

			current_grid_point->token->lock();

			/*	Gets the value stored within the current meshes layer entry in the current grid point	*/
			float point_mesh_value = current_grid_point->get_mesh_value(current_mesh_local_state.mesh_indx);

			/*	If the above value is < 0, then the current mesh has no layer entry in the current grid point, meaning that this is the first time that this mesh has been calced for the current
				mesh. In this case, a new mesh layer is added to the current grid point, and the current distance is written to it 	*/
			if (point_mesh_value < 0)
			{
				shared_type::index_xyzw_type layer_index;
				layer_index.x = current_grid_point->index.x;
				layer_index.y = current_grid_point->index.y;
				layer_index.z = current_grid_point->index.z;
				layer_index.w = current_grid_point->layers.size();
				current_mesh_local_state.token->lock();
				unsigned long mesh_layer_table_next_index = current_mesh_local_state.mesh->layer_indices->size();
				current_mesh_local_state.mesh->layer_indices->push_back(layer_index);
				current_mesh_local_state.token->unlock();

				mesh_layer_type* layer_to_add = new mesh_layer_type;
				if (distance > .0f)
				{
					layer_to_add->value = distance;
				}
				layer_to_add->mesh_indx = current_mesh_local_state.mesh_indx;
				layer_to_add->mesh_layer_table_index = mesh_layer_table_next_index;
				current_grid_point->layers.push_back(layer_to_add);
			}
			/*	If the above value is > 0, then the current mesh already has a mesh layer entry in the current grid point, in this case it is checked if the current distance is greater than the
				value currently stored within the mesh layer (in in order to implement lighten blending), as well as if the distance is > 0, if so, the current distance is then written to the
				mesh layer	*/
			else if ((point_mesh_value < distance) && (distance > .0f))
			{
				current_grid_point->set_mesh_value(distance, current_mesh_local_state.mesh_indx);
			}

			//current_grid_point->v = .65;

			current_grid_point->token->unlock();

		}
	}

	/*Each batch of jobs have a counter that is incremented everytime a job is completed, ie, once a thread completes a job, it increments the counter within that job's tri_local instance,
	it then checks to see if the number of jobs that have been completed matches the number of jobs that were sent in that jobs job batch (in this case that number is the thread count,
	however it would probably be better to have an actual variable that has the max number of jobs instead of using the thread count), if the numbers do match, then the thread assumes that
	the job that it just completed was the last one in that batch, and so it then take the responsibility to clean up the dynamically allocated memory structures belonging to that batch, once
	it does this, it increments the number of completed batches by one (this counter is used by the main thread to know once all the batches have been completed)*/

	/*It should also be noted for clarity that 1 batch is sent every triangle*/

	current_tri_local_state.token->lock();
	current_tri_local_state.jobs_completed += 1;
	if (current_tri_local_state.jobs_completed >= thread_pool.thread_count)
	{
		current_tri_local_state.token->unlock();
		delete current_tri_local_state.token;

		current_mesh_local_state.token->lock();
		current_mesh_local_state.tris_completed += 1;
		if (current_mesh_local_state.tris_completed >= current_mesh_local_state.mesh->tris.size())
		{
			current_mesh_local_state.token->unlock();
			delete current_mesh_local_state.token;
			delete& current_mesh_local_state;
		}
		else
		{
			current_mesh_local_state.token->unlock();
		}

		for (unsigned long a = 0; a < current_tri_local_state.job_cmprt_table_next_index_y; ++a)
		{
			delete[] current_tri_local_state.job_cmprt_table_cropped[a];
		}
		delete[] current_tri_local_state.job_cmprt_table_cropped;

		delete& current_tri_local_state;


		update_local.token.lock();
		update_local.batches_completed += 1;
		update_local.token.unlock();
	}
	else
	{
		current_tri_local_state.token->unlock();
	}

	return 0;
}


/*	This function essentially, atleast in a high level sense, loops through each triangle in each mesh belonging to the dfc passed to it, and for each, determines whic compartments in
	the df grid are relevent to said before dispatching distance calc function calls for said tri to the thread pool. Once a tri has its respective dist calc function calls dispatched
	(its "jobs"), the below function's loops do NOT wait for those jobs to be completed, instead it continues on to the next tri and repeats the above (unless the thread pools job stack
	is full, in which case it waits until the number of jobs in the stack decreases sufficiently). Once all tris have had their batches dispatched (the jobs for each triangle are refered
	to collectively as a "batch"), it will wait until all batches are completed, before cleaning up and exiting	*/
int df_type::update_per_tri(const unsigned long& dfc_id, const unsigned long& dfc_index)
{
	/*	Before looping through each tri, we must first check wether or not each mesh in the current dfc has changed, as we should only be updating the df grid point layer for a mesh
	if said mesh has actually changed, to avoid wasting computing time. It should be noted that it is likely that each dfc only contnains a single mesh, as in order for dfcs have multiple
	they must be split into seperate parts by enabling a function that does that, however said function is currently a poor implementation (it's expensive), and so as such is currently
	disabled. Keep in mind that the below code assumes that a dfc may have multiple meshes, just so that it is compatable with the scneario where the dfcs do infact get split, even though
	that feature currently isn't enabled.	*/

	/*	Sets alias	*/	
	update_local_type::dfc_cache_type& dfc_cache = update_local.dfc_cache;

	/*	Sets objects	*/
	update_local.dfc_index = update_local.dfc_cache.dfc_id_to_indx(dfc_id, 1);
	//unsigned long legacy_dfc_id_index = update_local.dfc_cache.dfc_id_to_indx()
	unsigned long dfc_mesh_cache_y_size = dfc_cache.mesh_cache[update_local.dfc_index]->size();

	/*	Checks if the current dfc is new, that is to say, if it existed when the distance field was last updated	*/
	bool is_new_dfc = false;
	if (dfc_cache.legacy_is_valid == true)
	{
		if (dfc_cache.existing_dfcs[update_local.dfc_index] == false)
		{
			is_new_dfc = true;
		}
	}
	else
	{
		is_new_dfc = true;
	}

	/*	Checks to see if there are less meshes in the current dfc then there were in this dfc last time the distance field was updated, that is to say, if a mesh has been removed. If so,
		all mesh layers in the df grid belonging to the current dfc are removed (this is probably an overly precautious measure, theres probably better ways than just nuking all the layers).
		Note that this is obvisously only done if the current dfc is not new, and as well as this, because split dfcs (multiple meshes per dfc) are current disabled, this is basically a no-op
		atm	*/	
	unsigned long dfc_legacy_index = 0;
	unsigned long dfc_mesh_cache_legacy_y_size = 0;
	if (is_new_dfc == false)
	{
		dfc_legacy_index = dfc_cache.dfc_id_to_indx(dfc_id, 2);
		dfc_mesh_cache_legacy_y_size = dfc_cache.mesh_cache_legacy[dfc_legacy_index]->size();
		if (dfc_mesh_cache_y_size < dfc_mesh_cache_legacy_y_size)
		{
			for (unsigned long a = dfc_mesh_cache_y_size; a < dfc_mesh_cache_legacy_y_size; ++a)
			{
				dfc_cache.remove_mesh_layers(dfc_id, a, true, volume_local);
			}
		}
	}

	/*	Loops through each mesh in the current dfc (as mentioned above, currently this would only iterate once as each dfc would only have one mesh as split dfcs are currently disabled due to
		poor perf)	*/
	for (unsigned long a = 0; a < dfc_mesh_cache_y_size; ++a)
	{

		/*	Determines 2 things, if the current mesh is new, and if it has changed (the latter obviously only being relevent if the former is false)	*/
		bool has_changed = false;
		//unsigned long* calced_verts = nullptr;
		bool is_new_mesh = false;
		if (is_new_dfc == false)
		{
			/*	If the current mesh's index is greater than that of the last mesh's index in the current dfc's legacy cache, than treat the current mesh as new (again, no split dfc so this will
				pretty much always be false atm)	*/
			if (a >= dfc_mesh_cache_legacy_y_size)
			{
				is_new_mesh = true;
			}
			/*	If false, then check to see if the current mesh has the same number of tris as it's equivalent mesh in the current dfc's legacy cache, if not, mark the current mesh as having changed*/
			else
			{
				dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->calc_tris_size();
				dfc_cache.mesh_cache_legacy[dfc_cache.dfc_id_to_indx(dfc_id, 2)]->at(a)->calc_tris_size();
				if (dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->tris_size != dfc_cache.mesh_cache_legacy[dfc_cache.dfc_id_to_indx(dfc_id, 2)]->at(a)->tris_size)
				{
					has_changed = true;
				}
				else
				{
					//unsigned long mesh_verts_amount = dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->tris_size * 3u;
					//calced_verts = new unsigned long[mesh_verts_amount];
				}
			}
		}
		/*	If the current dfc is new, then all meshes in it are treated as new as well	*/
		else
		{
			is_new_mesh = true;
		}

		mesh_local_type* mesh_local = nullptr;


	loop_through_tris:

		/*	If the mesh has changed or is new, creates a new mesh_local_type object, and initializes it appripriatly	*/
		if ((has_changed == true) || (is_new_mesh == true))
		{
			mesh_local = new mesh_local_type;
			mesh_local->token = new std::mutex;
			mesh_local->dfc_indx = update_local.dfc_index;
			mesh_local->mesh_indx.dfc_id = &dfc_cache.dfc_indx_to_id_table[update_local.dfc_index]->id;
			mesh_local->mesh_indx.mesh_index = a;
			dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->layer_indices = new std::vector<shared_type::index_xyzw_type>;

			/*	If the current mesh has changed, set mesh_local_type object's state appropriately, and remove all df grid layers beloning to said mesh	*/
			if (has_changed == true)
			{
				mesh_local->has_changed = true;
				dfc_cache.mesh_cache_legacy[dfc_cache.dfc_id_to_indx(dfc_id, 2)]->at(a)->moved_from_legacy = true;
				dfc_cache.remove_mesh_layers(dfc_id, a, true, volume_local);
			}
			/*	if the current mesh is new, set mesh_local_type object's state appropriately	*/
			else
			{
				mesh_local->is_new_mesh = true;
			}
		}

		/*	Loops through each triangle in the current mesh	*/
		for (unsigned long b = 0; b < dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->tris_size; ++b)
		{
			/*	Defines alias	*/
			shared_type::tri_info_type& current_tri = dfc_cache.tri_cache[update_local.dfc_index][dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->tris[b]];

			/*	If the mesh has not already been flagged as changed nor new, The below statement checks if the current tri is the same as it's counterpart in the the legacy cache, if so, continue to next tri, as normal
				however if not, mark the mesh as having changed and jump to the above label "loop_through_tris". Once jumping to this label, the mesh will be treated as having changed, and the below statemtn
				will not be run again for the current mesh. All tris are looped through and no discrepancies are found with the those of the legacy mesh, then the for loop completes and the flow moves on, and no batches
				are sent for the current mesh (as it hasn't changed)
				
				It may be helpful to note that that the reason as to why the above code does some preliminary checks to see if a mesh has changed or is new before resorting to brute force checking every triangle, is for
				optimization, as obvisouly checking every tri against that of the legacy mesh is expensive	*/
			if ((is_new_mesh == false) && (has_changed == false))
			{
				shared_type::tri_info_type& current_legacy_tri = dfc_cache.tri_cache_legacy[dfc_cache.dfc_id_to_indx(dfc_id, 2)][dfc_cache.mesh_cache_legacy[dfc_cache.dfc_id_to_indx(dfc_id, 2)]->at(a)->tris[b]];

				if ((dfc_cache.vert_cache[current_tri.vert_0].coord != dfc_cache.vert_cache_legacy[current_legacy_tri.vert_0].coord) ||
					(dfc_cache.vert_cache[current_tri.vert_1].coord != dfc_cache.vert_cache_legacy[current_legacy_tri.vert_1].coord) ||
					(dfc_cache.vert_cache[current_tri.vert_2].coord != dfc_cache.vert_cache_legacy[current_legacy_tri.vert_2].coord))
				{
					has_changed = true;
					goto loop_through_tris;
				}

				continue;
			}


			/*	Creates tri_local_type object for the current tri and set's it's "mesh_local_state" pointer to point to the current mesh's mesh_local_type object.
				Also sets state in current mesh's mesh_local_type object	*/
			tri_local_type* tri_local = new tri_local_type;
			tri_local->mesh_local_state = mesh_local;
			tri_local->mesh_local_state->mesh = dfc_cache.mesh_cache[update_local.dfc_index]->at(a);

			/*Defines aliases*/
			tri_local->vert_0 = &dfc_cache.vert_cache[current_tri.vert_0].coord;
			tri_local->vert_1 = &dfc_cache.vert_cache[current_tri.vert_1].coord;
			tri_local->vert_2 = &dfc_cache.vert_cache[current_tri.vert_2].coord;


			tri_local->tri_index = dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->tris[b];


			/*Calculates which enclosing compartments enclose the current triangle*/
			/*This is done in a way that is similiar to 2D rasterization (notably Bresenham's Line Algorithm), but wtih 3D instead. The coords of the vertices in world space are
			  first converted into compartment index space (that is, there position is represented by where they are in relation to each compartment's index, eg, if a vertex is in
			  the centre of compartment 6, then it's compartment index space coords are (6.5, 6.5 6.5)), the reason why this is done is because if you know the coordinates of a point
			  in compartment index space, then you can find which compartment it lies in by just truncating the coords.

			  After the coords are converted, the compartments enclosing the 3 vertices of the triangle are gotten (by truncating the vertices coords), and are added to the list of
			  enclosing compartments. Once this is done, the rasterize_on_axis function is called 3 times (once for each axis). What this function does is, for the axis passes to it,
			  first sorts the 3 vertices along that axis, into min, mid, and max (lowest number, mid number, and highest number). It essentially then calculates the amount of times
			  it will take to scan (in intervals of 0.5) from the max vertex to the mid vertex (along the axis passed to the function), then it calculates the same for the distance
			  between the mid vertex and the min vertex. The function will then perform 2 loop statements, one that scans from max to mid, and the other from mid to min:

			                             Mid
			                          __--O.
			                     __---       \
			                __---              \
			           __---                     \
			      __---                            \
			 Min O-----------------------------------O Max
			                              <-----------
			     <------------------------     |
			            |                      |
			            |       The first loop statement scans from max to mid, only taking
			            |       into account the first part of the hyptenuse, and the line
			            |       between max and mid
			            |
			            |
				The second loop statement scans from mid to min
				(obviously, starting where the first loop statement
				left off), and only takes into account the last part
				of the hypotenus, and the line between mid and min

			  At each increment in the scanning process (that is to say, in each loop iteration), the coordinates on the 2 axis other than the one being scanned (eg, if scanning across
			  the x axis, then only the y and z coordinates) is gotten at the points where the current scan axis coordinates (6.5, 6, 5.5, 5, etc) intersects the 2 lines being taken
			  into account in the current loop statement (either the hypotenuse and max-mid, or the hypotenuse and mid-mid), and the compartment that surrounds the 2 edges/ lines of
			  triangle at that particular coord on the scan axis is gotten by just truncating both sets of coords, and they can then be added to the list of enclosing compartments.

			  This rasterize_on_axis function is called 3 times, once for each axis, so that each axis is scanned through.

			  It should also be noted as to why 0.5 is chosen as the increment amount while scanning through, this is to (presumably) increase the accuracy of of the rastierization,
			  however as of currently writing, I've havn't tested whether or not it actuall makes a difference, so I shall do that. If not then the increment amount should be changed
			  to (probably) 1.0, as it would speed up the scanning process.

			  Once this is done, the list of enclosing compartments, in which each compartment has a 3D vector index (refering to the actual array elements used to ID the compartment, not the actual indices of
			  each element of the array of enclosing compartments), must have each of it's compartmnet's indices converted into linear indices, sd all the existing compartment lookup tables ID each compartment by a single 1D value, not in the form of a 3D vector.																	*/
			  /*-------------------------------------------------------------------------------------------------------------*/

			shared_type::coord_xyz_type vert_0_indx_space;
			shared_type::coord_xyz_type vert_1_indx_space;
			shared_type::coord_xyz_type vert_2_indx_space;

			/*Converts vertices from worlds space to compartment index space*/
			vert_0_indx_space = wrld_space_to_cmprt_indx_space(*tri_local->vert_0);
			vert_1_indx_space = wrld_space_to_cmprt_indx_space(*tri_local->vert_1);
			vert_2_indx_space = wrld_space_to_cmprt_indx_space(*tri_local->vert_2);

			/*Gets enclosing cmprt of the current tri's vertices*/

			shared_type::index_xyz_type vert_0_enclosing_cmprt = get_enclsing_cmprt_from_indx_space(vert_0_indx_space);
			shared_type::index_xyz_type vert_1_enclosing_cmprt = get_enclsing_cmprt_from_indx_space(vert_1_indx_space);
			shared_type::index_xyz_type vert_2_enclosing_cmprt = get_enclsing_cmprt_from_indx_space(vert_2_indx_space);

			/*Defines array that enclosing compartments will be added to, current temp max is 400*/
			unsigned long enclosing_cmprts_linear[400] = {};
			unsigned short enclosing_cmprts_linear_next_index = 0;

			/*Checks all 3 verts are in the same cmprt*/
			if ((vert_0_enclosing_cmprt.x == vert_1_enclosing_cmprt.x) && (vert_0_enclosing_cmprt.y == vert_1_enclosing_cmprt.y) && (vert_0_enclosing_cmprt.z == vert_1_enclosing_cmprt.z) &&
				(vert_0_enclosing_cmprt.x == vert_2_enclosing_cmprt.x) && (vert_0_enclosing_cmprt.y == vert_2_enclosing_cmprt.y) && (vert_0_enclosing_cmprt.z == vert_2_enclosing_cmprt.z))
			{
				/*If they're in the same cmprt*/

				enclosing_cmprts_linear[0] = (vert_0_enclosing_cmprt.z + (volume_local.cmprt_amount.z * vert_0_enclosing_cmprt.y)) + ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * vert_0_enclosing_cmprt.x);
				enclosing_cmprts_linear_next_index = 1;
			}
			else
			{
				/*If they're in different cmprts*/

				shared_type::index_xyz_type enclosing_cmprts[400];

				enclosing_cmprts[0] = vert_0_enclosing_cmprt;
				enclosing_cmprts[1] = vert_1_enclosing_cmprt;
				enclosing_cmprts[2] = vert_2_enclosing_cmprt;

				unsigned short enclosing_cmprts_next_index = 3;

				/*Calls funcrtion rasterize_on_axis 3 times, once for each axis, such that the process will scan through each axis*/
				rasterize_on_axis(enclosing_cmprts, enclosing_cmprts_next_index, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'x');
				rasterize_on_axis(enclosing_cmprts, enclosing_cmprts_next_index, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'y');
				rasterize_on_axis(enclosing_cmprts, enclosing_cmprts_next_index, vert_0_indx_space, vert_1_indx_space, vert_2_indx_space, 'z');


				/*Converts the 3D indices of the compartments contained within the array enclosing_cmprts into linear/ 1D indices*/



				for (unsigned short c = 0; c < enclosing_cmprts_next_index; ++c)
				{
					unsigned long current_cmprt_linear = (enclosing_cmprts[c].z + (volume_local.cmprt_amount.z * enclosing_cmprts[c].y)) + ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * enclosing_cmprts[c].x);

					for (unsigned short d = 0; d < enclosing_cmprts_linear_next_index; ++d)
					{
						if (current_cmprt_linear == enclosing_cmprts_linear[d])
						{
							goto is_dup;
						}
					}

					enclosing_cmprts_linear[enclosing_cmprts_linear_next_index] = current_cmprt_linear;
					enclosing_cmprts_linear_next_index += 1;


				is_dup:
					continue;
				}
			}


			/*Calculates update_local.unit/direction vector of each edge on the triangle*/
			/*-------------------------------------------------------------------------------------------------------------*/

			/*Gets update_local.unit/ direction vector from vert_0 to vert_1*/

			shared_type::coord_xyz_type dist_0_1;

			dist_0_1.x = tri_local->vert_1->x - tri_local->vert_0->x;
			dist_0_1.y = tri_local->vert_1->y - tri_local->vert_0->y;
			dist_0_1.z = tri_local->vert_1->z - tri_local->vert_0->z;

			tri_local->distance_0_1 = (dist_0_1.x * dist_0_1.x) + (dist_0_1.y * dist_0_1.y) + (dist_0_1.z * dist_0_1.z);
			tri_local->distance_0_1 = sqrt(tri_local->distance_0_1);

			tri_local->unit_0_1.x = dist_0_1.x / tri_local->distance_0_1;
			tri_local->unit_0_1.y = dist_0_1.y / tri_local->distance_0_1;
			tri_local->unit_0_1.z = dist_0_1.z / tri_local->distance_0_1;


			/*Gets tri_local->unit/ direction vector from tri_local->vert_0 to tri_local->vert_2*/

			shared_type::coord_xyz_type dist_0_2;

			dist_0_2.x = tri_local->vert_2->x - tri_local->vert_0->x;
			dist_0_2.y = tri_local->vert_2->y - tri_local->vert_0->y;
			dist_0_2.z = tri_local->vert_2->z - tri_local->vert_0->z;

			tri_local->distance_0_2 = (dist_0_2.x * dist_0_2.x) + (dist_0_2.y * dist_0_2.y) + (dist_0_2.z * dist_0_2.z);
			tri_local->distance_0_2 = sqrt(tri_local->distance_0_2);

			tri_local->unit_0_2.x = dist_0_2.x / tri_local->distance_0_2;
			tri_local->unit_0_2.y = dist_0_2.y / tri_local->distance_0_2;
			tri_local->unit_0_2.z = dist_0_2.z / tri_local->distance_0_2;


			/*Gets tri_local->unit/ direction vector from tri_local->vert_1 to tri_local->vert_2*/

			shared_type::coord_xyz_type dist_1_2;

			dist_1_2.x = tri_local->vert_2->x - tri_local->vert_1->x;
			dist_1_2.y = tri_local->vert_2->y - tri_local->vert_1->y;
			dist_1_2.z = tri_local->vert_2->z - tri_local->vert_1->z;

			tri_local->distance_1_2 = (dist_1_2.x * dist_1_2.x) + (dist_1_2.y * dist_1_2.y) + (dist_1_2.z * dist_1_2.z);
			tri_local->distance_1_2 = sqrt(tri_local->distance_1_2);


			tri_local->unit_1_2.x = dist_1_2.x / tri_local->distance_1_2;
			tri_local->unit_1_2.y = dist_1_2.y / tri_local->distance_1_2;
			tri_local->unit_1_2.z = dist_1_2.z / tri_local->distance_1_2;


			/*Calculates the triangles normal vector*/
			/*-------------------------------------------------------------------------------------------------------------*/

			tri_local->normal.x = (dist_0_1.y * dist_0_2.z) - (dist_0_1.z * dist_0_2.y);
			tri_local->normal.y = (dist_0_1.z * dist_0_2.x) - (dist_0_1.x * dist_0_2.z);
			tri_local->normal.z = (dist_0_1.x * dist_0_2.y) - (dist_0_1.y * dist_0_2.x);

			double normal_distance = (tri_local->normal.x * tri_local->normal.x) + (tri_local->normal.y * tri_local->normal.y) + (tri_local->normal.z * tri_local->normal.z);
			normal_distance = sqrt(normal_distance);

			tri_local->normal.x = tri_local->normal.x / normal_distance;
			tri_local->normal.y = tri_local->normal.y / normal_distance;
			tri_local->normal.z = tri_local->normal.z / normal_distance;

			current_tri.normal = tri_local->normal;



			/*Gets distance of plane created by tri from origin (is needed to get the distance from
			  the grid point to the plane), d = -(ax + by + cz)*/
			  /*-------------------------------------------------------------------------------------------------------------*/
			current_tri.d = ((current_tri.normal.x * tri_local->vert_0->x) +
				(current_tri.normal.y * tri_local->vert_0->y) +
				(current_tri.normal.z * tri_local->vert_0->z)) * -1.0;

			/*Creates an 2D array that contains each compartment that is relevent to the current tris enclosing compartment
			  and divides the grid points in each compartment into smaller groups of grid points, one for each job, and adds
			  each group to said groups enclosing relevent comparmtnets array entry into the 2D array*/
			  /*-------------------------------------------------------------------------------------------------------------*/

			unsigned short job_cmprt_table_x_amount = 1 + (thread_pool.thread_count * 2);

			unsigned long** job_cmprt_table = new unsigned long* [volume_local.cmprt_amount_total]();

			/*Fix relevancy distance and then calculate exactly how many compartments will be rlvnt*/

			/*Loops through each enclosing compartment*/
			for (unsigned short c = 0; c < enclosing_cmprts_linear_next_index; ++c)
			{
				unsigned long& current_enclosing_cmprt = enclosing_cmprts_linear[c];

				/*Loops through each compartment relevent to the current enclosing compartment*/
				for (unsigned long d = 0; d < volume_local.cmprt_meta[current_enclosing_cmprt].rlvncy_table_size; ++d)
				{
					unsigned long& current_rlvnt_cmprt = volume_local.cmprt_rlvncy_table[current_enclosing_cmprt][d];
					unsigned long& current_rlvnt_cmprt_size = volume_local.cmprt_meta[current_rlvnt_cmprt].size;

					bool cmprt_already_calced = false;

					for (unsigned long e = 0; e < tri_local->job_cmprt_table_next_index_y; ++e)
					{
						if (current_rlvnt_cmprt == job_cmprt_table[e][0])
						{
							goto already_calced;
						}
					}
					goto is_uniq;

				already_calced:
					continue;

				is_uniq:

					for (unsigned long e = 0; e < current_rlvnt_cmprt_size; ++e)
					{
						if (volume_local.cmprt_table[current_rlvnt_cmprt][e]->token == nullptr)
						{
							volume_local.cmprt_table[current_rlvnt_cmprt][e]->token = new std::mutex;
						}
					}

					job_cmprt_table[tri_local->job_cmprt_table_next_index_y] = new unsigned long[job_cmprt_table_x_amount];
					job_cmprt_table[tri_local->job_cmprt_table_next_index_y][0] = current_rlvnt_cmprt;

					unsigned long grid_points_per_job = (unsigned long)(current_rlvnt_cmprt_size / thread_pool.thread_count);
					unsigned long first_grid_point_index = 0;
					unsigned long last_grid_point_index = grid_points_per_job - 1;

					job_cmprt_table[tri_local->job_cmprt_table_next_index_y][1] = first_grid_point_index;
					job_cmprt_table[tri_local->job_cmprt_table_next_index_y][2] = last_grid_point_index;

					for (unsigned int e = 1; e < thread_pool.thread_count; ++e)
					{
						if (e != (thread_pool.thread_count - 1))
						{
							first_grid_point_index = last_grid_point_index + 1;
							last_grid_point_index = last_grid_point_index + grid_points_per_job;
						}
						else
						{
							first_grid_point_index = last_grid_point_index + 1;
							last_grid_point_index = current_rlvnt_cmprt_size - 1;
						}

						job_cmprt_table[tri_local->job_cmprt_table_next_index_y][1 + (e * 2)] = first_grid_point_index;
						job_cmprt_table[tri_local->job_cmprt_table_next_index_y][2 + (e * 2)] = last_grid_point_index;
					}

					tri_local->job_cmprt_table_next_index_y += 1;
				}
			}

			tri_local->job_cmprt_table_cropped = new unsigned long* [tri_local->job_cmprt_table_next_index_y];
			for (unsigned long c = 0; c < tri_local->job_cmprt_table_next_index_y; ++c)
			{
				tri_local->job_cmprt_table_cropped[c] = job_cmprt_table[c];
			}


			/*Prepare to send batch of jobs to thread pool*/

			tri_local->token = new std::mutex;

			delete[] job_cmprt_table;

			bool add_jobs_success;
			typedef void (*jobs)(void*, unsigned short);
			jobs jobs_ptr = &call_update_grid_points;

			/*Cretes a copy of the current tri_local state, each job batch gets a pointer to its respective tri_local state/ instance*/


			/*Attempts to sends batch of jobs to thread pool*/
		attempt_to_add_job_batch:
			add_jobs_success = thread_pool.add_jobs(jobs_ptr, thread_pool.thread_count, tri_local);


			/*Checks if attempt to add jobs was successfull*/
			if (add_jobs_success == true)
			{
				update_local.batches_sent += 1;

				goto batch_sent;
			}
			else
			{
				goto wait_for_space_in_job_stack;
			}


		wait_for_space_in_job_stack:
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(100000));

				if (thread_pool.job_amount < 10)
				{
					goto attempt_to_add_job_batch;
				}
			}

		batch_sent:

			continue;
		}

		/*	If the current mesh has not been flagged as changed nor new, then set the array of layer indices in the current mesh's entry in the dfc cache to equal that if it's legacy counterpart,
			as these layers have not been adjusted as the current mesh was never marked as changed or new	*/
		if ((has_changed == false) && (is_new_mesh == false))
		{
			dfc_cache.mesh_cache[update_local.dfc_index]->at(a)->layer_indices = dfc_cache.mesh_cache_legacy[dfc_legacy_index]->at(a)->layer_indices;
			dfc_cache.mesh_cache_legacy[dfc_legacy_index]->at(a)->layer_indices = nullptr;
			dfc_cache.mesh_cache_legacy[dfc_legacy_index]->at(a)->moved_from_legacy = true;
		}
	}

	/*Once all triangles in the current object have been iterated through, waits until all the jobs in each batch have been completed*/

recheck_if_batches_completed:
	if (update_local.batches_completed < update_local.batches_sent)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(100000));

		goto recheck_if_batches_completed;
	}

	for (unsigned short a = 0; a < volume_local.grid_amount.x; ++a)
	{
		for (unsigned short b = 0; b < volume_local.grid_amount.y; ++b)
		{
			for (unsigned short c = 0; c < volume_local.grid_amount.z; ++c)
			{
				if (volume_local.grid[a][b][c].token != nullptr)
				{
					delete volume_local.grid[a][b][c].token;
					volume_local.grid[a][b][c].token = nullptr;

					volume_local.grid[a][b][c].layers.shrink_to_fit();
				}
			}
		}
	}

	/*Deletes legacy dfc cache and stashes state if last dfc object (ie, if this is the last time this function will be called in the current update cycle)*/
	if (update_local.dfc_index == (update_local.dfc_cache.dfc_amount - 1u))
	{
		update_local.dfc_cache.clean_legacy();
	}

	return 0;
}

/*	This function is called before update_recipient is called, it does some prep work, namely cleaning state	*/
int df_type::pre_update_recipients(const unsigned long* dfrs, const unsigned long dfr_amount)
{
	update_recipients_local.clean();

	/*	Checks for deleted dfrs	*/

	remove_deleted_element_from_layer_system<std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>>(dfr_layers, &df_type::remove_dfrs_from_dfr_layer, dfrs, dfr_amount, nullptr, 0ul);

	return 0;
}


shared_type::coord_xyz_type df_type::wrld_space_to_grid_indx_space(const shared_type::coord_xyz_type coord)
{
	shared_type::coord_xyz_type index_space;
	index_space.x = (coord.x - volume_local.min_grid_coord.x) / volume_local.intern_grid_spacing;
	index_space.y = (coord.y - volume_local.min_grid_coord.y) / volume_local.intern_grid_spacing;
	index_space.z = (coord.z - volume_local.min_grid_coord.z) / volume_local.intern_grid_spacing;
	return index_space;
}


/*	This function will return the greatest interpolated value of layers belonging to the dfcs passed to it, ignoring those dfcs not listed	*/
float df_type::get_lerped_point_value(const shared_type::coord_xyz_type& vert_coord, const std::vector<unsigned long>& dfc_ids, const char mode, std::vector<shared_type::ncspline_type>& zaligned_splines, const int local_spline_length)
{
	grid_type***& grid = volume_local.grid;

	shared_type::coord_xyz_type current_vert_indx_space = wrld_space_to_grid_indx_space(vert_coord);

	/*	| "gp" here refers to grid point |	*/
	/*	V								 V	*/

	shared_type::index_xyz_type max_cell_gp;
	shared_type::index_xyz_type min_cell_gp;

	min_cell_gp.x = (unsigned short)current_vert_indx_space.x;
	min_cell_gp.y = (unsigned short)current_vert_indx_space.y;
	min_cell_gp.z = (unsigned short)current_vert_indx_space.z;

	max_cell_gp.x = min_cell_gp.x + 1;
	max_cell_gp.y = min_cell_gp.y + 1;
	max_cell_gp.z = min_cell_gp.z + 1;

	shared_type::coord_xyz_type current_vert_lerp_alpha;

	current_vert_lerp_alpha.x = current_vert_indx_space.x - min_cell_gp.x;
	current_vert_lerp_alpha.y = current_vert_indx_space.y - min_cell_gp.y;
	current_vert_lerp_alpha.z = current_vert_indx_space.z - min_cell_gp.z;

	grid_cell_value_type enclosing_cell_values;

	switch (mode)
	{
	case 0:
	{
		/*	Tricubic Interpolation	*/

		/*	The below block of code makes use of the splines created outside of this function (the z aligned splines),
			and creates splines for the other 2 remaining axis (these splines are local to this block, unlike the z ones).
			First, splines are created which are aligned in the y axis (the amount of splines created, as well as the number
			of knotts per spline, is dictated by "local_spline_length"), these splines are arranged next to each other along
			the x axis, with the current grid cell at the center, eg:
			o-------> x
			|
			|      |  |  |  |  |  |  |  |
			V      |  |  |  |  |  |  |  |
			y      |  |  |  |__|  |  |  |
			       |  |  |  |__|  |  |  |  <---- Splines (note the grid cell at the center)
			       |  |  |  |  |  |  |  |
			       |  |  |  |  |  |  |  |
			       |  |  |  |  |  |  |  |
			
			These spline's knotts sample from the existing z aligned splines at the current verts z coord (in index space)

			Once these y aligned splines are created, a single x aligned spline is created whose knotts sample from all the
			y aligned splines (at the current verts index space y coord), eg:

			o-------> x
			|
			|      |  |  |  |  |  |  |  |
			V      |  |  |  |  |  |  |  |
			y      |  |  |  |__|  |  |  |
			       o--o--o--o--o--o--o--o  <---- x aligned spline
			       |  |  |  |  |  |  |  |
			       |  |  |  |  |  |  |  |
			       |  |  |  |  |  |  |  |

			The final interpolated value is then gained by sampling from this spline at the current verts index spade x coord	*/

		/*	Sets up dynamically allocated objects	*/
		double** yaligned_splines_knotts = new double* [local_spline_length];
		shared_type::ncspline_type* yaligned_splines = new shared_type::ncspline_type[local_spline_length];
		/*	Defines the minimum knott, it should be noted that this does not represent a single point, but rather
			defines a minimum y value for the y aligned splines, and a minimum x coord for the x aligned spline.
			These minimum coords represent the 0 knott's position along a splines respective axis	*/
		shared_type::index_xy_signed_type min_knott;
		min_knott.x = min_cell_gp.x - ((local_spline_length / 2u) - 1);
		min_knott.y = min_cell_gp.y - ((local_spline_length / 2u) - 1);


		/*	Creates y aligned splines	*/

		/*	The below for loop iterates through each x coordinate in which a spline will sit	*/
		for (int a = 0u; a < local_spline_length; ++a)
		{
			/*	Gets coordinate of current x coord in index space ("a" is local to the index space of the arranged splines)	*/
			int a_grid_indx_space = min_knott.x + a;

			/*	Bound checks current x coord. If the current x coord sits outside the grid, then a y spline is simply
				not created at the current coord	*/
			if ((a_grid_indx_space >= 0) && (a_grid_indx_space < volume_local.grid_amount.x))
			{
				/*	Dynamically allocates and array to store the knotts for the new spline	*/
				yaligned_splines_knotts[a] = new double[local_spline_length] {};

				/*	The below for loop iterates through each y coord in which a knott in the new spline will sit
					(ie it loops through each knott)	*/
				for (int b = 0u; b < local_spline_length; ++b)
				{
					/*	Gets coordinate of current y coord in index space ("b" is local to the index space of the spline's knotts)	*/
					int b_grid_indx_space = min_knott.y + b;

					/*	Bound checks current current y coord. If the current y coord sits outside the grid, the value of the knott
						at the current y coord is left as 0 (the array of knotts was value initialized)	*/
					if ((b_grid_indx_space >= 0) && (b_grid_indx_space < volume_local.grid_amount.y))
					{
						/*	Samples the z aligned spline that sits at the current (x, y) coord, using the current verts index space z coord	*/
						yaligned_splines_knotts[a][b] = zaligned_splines[volume_local.grid[a_grid_indx_space][b_grid_indx_space][min_cell_gp.z].temp_spline_indx].sample(current_vert_indx_space.z);
					}
				}

				/*	Initializes spline at the current x coord using the gathered knott values	*/
				yaligned_splines[a].init_spline(yaligned_splines_knotts[a], local_spline_length, shared_type::index_xy_type());
			}
			else
			{
				yaligned_splines_knotts[a] = nullptr;
			}
		}


		/*	Creates x aligned spline	*/

		/*	Dynamically allocates an array to store the knotts for the x aligned spline	*/
		double* xaligned_knotts = new double[local_spline_length];

		/*	The below for loop iterates through each y aligned spline, sampling each at the current vert's index space y coord	*/
		for (int a = 0u; a < local_spline_length; ++a)
		{
			xaligned_knotts[a] = yaligned_splines[a].sample(current_vert_indx_space.y - min_knott.y);
		}
		/*	Creates	spline using the gathered knott values	*/
		shared_type::ncspline_type xaligned_spline(xaligned_knotts, local_spline_length, shared_type::index_xy_type());

		/*	Gets the final interpolated value by sampling from the x aligned spline at the current vert's index space x coord	*/
		float return_value = xaligned_spline.sample(current_vert_indx_space.x - min_knott.x);
		if (return_value < .0f)
		{
			return_value = .0f;
		}
		else if (return_value > 1.0f)
		{
			return_value = 1.0f;
		}
		
		/*	Deletes dynamically allocated memory	*/
		for (int a = 0u; a < local_spline_length; ++a)
		{
			if (yaligned_splines_knotts[a] != nullptr)
			{
				delete[] yaligned_splines_knotts[a];
			}
		}
		delete[] yaligned_splines_knotts;
		delete[] yaligned_splines;
		delete[] xaligned_knotts;

		return return_value;
	}

	case 1:
	{
		/*	Trilinear interpolation	*/

		/*   mxx o                    o mmx
		          \          x_z       \
		   mxx_xxx o__________o_________o mmx_xmx
		            \         |          \
		             \        |           \
		          xxx o       |            o xmx
		                      |
		                      o final
		                      |
		     mxm o            |        o mmm
		          \           |         \
		   mxm_xxm o__________o__________o mmm_xmm
		            \        m_z          \
		             \                     \
		          xxm o                     o xmm

			mmm is first lerped with xmm to get mmm_xmm, then mxm is lerped with xxm to get mxm_xxm,
			then mmm_xmm is lerp with mxm_xxm to get m_z,
			then mmx is lerped with xmx to get mmx_xmx, then mxx is lerped with xxx to get mxx_xxx,
			Then mmx_xmx is lerped with mxx_xxx to get x_z,
			The m_z is lerped with x_z to get the final value
		*/

		float mmm_xmm_value = lerp_btwn_grid_points_values(min_cell_gp.x, grid[min_cell_gp.x][min_cell_gp.y][min_cell_gp.z].get_max_value(dfc_ids), max_cell_gp.x, grid[max_cell_gp.x][min_cell_gp.y][min_cell_gp.z].get_max_value(dfc_ids), current_vert_lerp_alpha.x);
		float mxm_xxm_value = lerp_btwn_grid_points_values(min_cell_gp.x, grid[min_cell_gp.x][max_cell_gp.y][min_cell_gp.z].get_max_value(dfc_ids), max_cell_gp.x, grid[max_cell_gp.x][max_cell_gp.y][min_cell_gp.z].get_max_value(dfc_ids), current_vert_lerp_alpha.x);

		float m_z_value = lerp_btwn_grid_points_values(min_cell_gp.y, mmm_xmm_value, max_cell_gp.y, mxm_xxm_value, current_vert_lerp_alpha.y);

		float mmx_xmx_value = lerp_btwn_grid_points_values(min_cell_gp.x, grid[min_cell_gp.x][min_cell_gp.y][max_cell_gp.z].get_max_value(dfc_ids), max_cell_gp.x, grid[max_cell_gp.x][min_cell_gp.y][max_cell_gp.z].get_max_value(dfc_ids), current_vert_lerp_alpha.x);
		float mxx_xxx_value = lerp_btwn_grid_points_values(min_cell_gp.x, grid[min_cell_gp.x][max_cell_gp.y][max_cell_gp.z].get_max_value(dfc_ids), max_cell_gp.x, grid[max_cell_gp.x][max_cell_gp.y][max_cell_gp.z].get_max_value(dfc_ids), current_vert_lerp_alpha.x);

		float x_z_value = lerp_btwn_grid_points_values(min_cell_gp.y, mmx_xmx_value, max_cell_gp.y, mxx_xxx_value, current_vert_lerp_alpha.y);

		return lerp_btwn_grid_points_values(min_cell_gp.z, m_z_value, max_cell_gp.z, x_z_value, current_vert_lerp_alpha.z);
	}
	case 2:

	{
		/*	Nearest Neighbour Interpolation	*/

		shared_type::index_xyz_type nearest_gp;

		nearest_gp.x = (unsigned short)current_vert_indx_space.x;
		if ((current_vert_indx_space.x - nearest_gp.x) >= .5)
		{
			++nearest_gp.x;
		}
		nearest_gp.y = (unsigned short)current_vert_indx_space.y;
		if ((current_vert_indx_space.y - nearest_gp.y) >= .5)
		{
			++nearest_gp.y;
		}
		nearest_gp.z = (unsigned short)current_vert_indx_space.z;
		if ((current_vert_indx_space.z - nearest_gp.z) >= .5)
		{
			++nearest_gp.z;
		}

		return volume_local.grid[nearest_gp.x][nearest_gp.y][nearest_gp.z].get_max_value(dfc_ids);
	}

	default:

		return .0f;
	}
}


void df_type::call_get_lerped_point_value(void* args_ptr, unsigned short job_index)
{
	get_lerped_point_value_local_args_type* local_args = &((get_lerped_point_value_local_args_type*)args_ptr)[job_index];
	get_lerped_point_value_args_type* args = local_args->args;
	*local_args->value = this->get_lerped_point_value(*local_args->vert_coord, *args->dfc_ids, args->mode, *args->zaligned_splines, args->local_spline_length);
	if (args->gamma != 1.0f)
	{
		*local_args->value = std::pow(*local_args->value, args->gamma);
	}
	args->token->lock();
	++*args->jobs_completed;
	++args->jobs_completed_table[local_args->a];
	if (args->jobs_completed_table[local_args->a] == local_args->current_batch_size)
	{
		delete[] args_ptr;
	}
	args->token->unlock();
}


/*	Checks if specified coord is wihtin grid, returns true if so, false if outside	*/
bool df_type::grid_bounds_check(const shared_type::coord_xyz_type& coord)
{
	/*	Gets coord of the max grid point	*/
	shared_type::index_xyz_type max_grid_point_coord;
	max_grid_point_coord.x = volume_local.min_grid_coord.x + (volume_local.intern_grid_spacing * (double)volume_local.max_grid_point.x);
	max_grid_point_coord.y = volume_local.min_grid_coord.y + (volume_local.intern_grid_spacing * (double)volume_local.max_grid_point.y);
	max_grid_point_coord.z = volume_local.min_grid_coord.z + (volume_local.intern_grid_spacing * (double)volume_local.max_grid_point.z);

	if (((coord.x < volume_local.min_grid_coord.x) || (coord.x >= max_grid_point_coord.x)) ||
		((coord.y < volume_local.min_grid_coord.y) || (coord.y >= max_grid_point_coord.y)) ||
		((coord.z < volume_local.min_grid_coord.z) || (coord.z >= max_grid_point_coord.z)))
	{
		return false;
	}
	else
	{
		return true;
	}
}


shared_type::index_xyz_type df_type::get_enclsing_cmprt(const shared_type::coord_xyz_type& coord)
{
	shared_type::coord_xyz_type current_vert_indx_space = wrld_space_to_cmprt_indx_space(coord);

	shared_type::index_xyz_type enclsing_cmprt;
	enclsing_cmprt.x = (unsigned short)current_vert_indx_space.x;
	enclsing_cmprt.y = (unsigned short)current_vert_indx_space.y;
	enclsing_cmprt.z = (unsigned short)current_vert_indx_space.z;

	return enclsing_cmprt;
}


shared_type::index_xyz_type df_type::get_enclsing_cmprt_from_indx_space(const shared_type::coord_xyz_type& indx_space_coord)
{
	shared_type::index_xyz_type enclsing_cmprt;
	enclsing_cmprt.x = (unsigned short)indx_space_coord.x;
	enclsing_cmprt.y = (unsigned short)indx_space_coord.y;
	enclsing_cmprt.z = (unsigned short)indx_space_coord.z;

	return enclsing_cmprt;
}


/*	Updates a recipient, that is to say, projects the distance field values onto a mesh	*/
int df_type::update_recipient(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::shared_type::vert_info_type* verts_buffer, unsigned long& vert_amount, const int interp_mode, const float gamma)
{
	/*	The below chunk of code results in a vector containing of all the dfc_id within the dfc_layer passed to the function
		(at the python level, each dfr_layer only lists the index of the dfc_layer(s) that affect it, so that list of layer indices
		must now be converted into a list of all the dfc_ids within said layers). This vector of dfc ids is then passed to the
		"get_lerped_point_value" function when it is called (once for each vert) so that it knows which dfc ids should affect the
		return value for that call	*/

		/*	vector containing dfc ids	*/
	std::vector<unsigned long> dfc_ids;
	{
		/*	Objects used to keep track of which dfc layers have already been added/ calced	*/
		unsigned long* calced_dfc_layers = new unsigned long[dfc_layers_nxt_indx];
		unsigned long calced_dfc_layers_nxt_indx = 0u;
		for (unsigned long a = 0; a < dfc_layers_nxt_indx; ++a)
		{
			/*	Checks if the current dfc layers has already been added/ calced, if so skip	*/
			for (unsigned long b = 0; b < calced_dfc_layers_nxt_indx; ++b)
			{
				if (dfc_layers[a] == calced_dfc_layers[b])
				{
					goto skip_a;
				}
			}

			/*	Add current dfc layers to list of already added/ calced layers	*/
			calced_dfc_layers[calced_dfc_layers_nxt_indx] = dfc_layers[a];
			calced_dfc_layers_nxt_indx += 1;

			/*	The below for loop iterates through each dfc id in the current dfc layer, and for each one, checks if it is already
				within the vector of dfc ids (This may be possible as multple dfc layers can contain the same dfc id), if not, adds
				it to the vector	*/
			this->dfc_layers[dfc_layers[a]].calc_size();
			for (unsigned long b = 0; b < this->dfc_layers[dfc_layers[a]].size; ++b)
			{
				/*	Checks if already in dfc_ids vector	*/
				unsigned long dfc_ids_size = dfc_ids.size();
				for (unsigned long c = 0; c < dfc_ids_size; ++c)
				{
					if (this->dfc_layers[dfc_layers[a]].vector[b]->id == dfc_ids[c])
					{
						goto skip_b;
					}
				}

				/*	Adds dfc id to vector of dfc ids	*/
				dfc_ids.push_back(this->dfc_layers[dfc_layers[a]].vector[b]->id);

			skip_b:
				continue;
			}

		skip_a:
			continue;
		}
	}

	std::vector<shared_type::ncspline_type> zaligned_splines;
	const int local_spline_length = 8u;
	shared_type::index_xyz_type min_cmprt;
	shared_type::index_xyz_type max_cmprt_p1;

	/*	The below block of code is run if tricubic interpolation is the set interp method, it creates a set of splines along the first,
		out of 3, axis (the z axis specifically). As this is the the first axis, the value at each knott is simply the value of the grid
		point at said knott (the first axis splines do not sample from existing splines, as they are the first), therefore they are not
		effected by the position of a vertex, and are thus independent. As a result, these first splines are created here, as there's no
		point recreating them for each vert.

		The method in which the splines are created is to first obtain the bounding box (on the x and y axis) covering all verts within
		the grid (outside verts are ignored) + some padding. Once this bounding box is obtained, each grid (x, y) index enclosed within
		the bounding box is iterated through, and at each index, a spline passing through every grid point at that (x, y) index is created
		(the spline traverses the z axis through all the grid points that align vertically). This results in a 2D array of splines all aligned
		along the z axis.

		Once the verts are being iterated through further down in the current function, the function "get_lerped_point_value" will be
		called for each vert, and that function will create splines along the y whose knott values will be the result of sampling from
		these z aligned splines, before the function then samples from said y aligned splines to get an x aligned spline which will be
		sampled from to get the final interpolated value. As these splines are dependent on the position of the vertex (as they sample
		form other splines using the verts respective axis coord), they are local to the function, unlike the z splines.

		In order for the aformentioned per vert function to know which spline to sample from given a given position, if a grid point
		has a zaligned spline passing through it, then it's data member for it's respective spline will contain the index of said spline.
		(If no spline passes through the grid point then this data member will = 0 (the first entry in "zaligned_splines" is a dummy))	*/
	if (interp_mode == 0)
	{
		/*	Push back dummy entry in "zaligned_splines"	*/
		zaligned_splines.push_back(shared_type::ncspline_type(shared_type::index_xy_type()));

		/*	Creates objects to keep track of the minimum and maximum verts (only considering those within the grid)	*/
		double double_nummax = (std::numeric_limits<double>::max)();
		double double_nummin = (std::numeric_limits<double>::lowest)();
		shared_type::coord_xyz_type min_vert;
		min_vert.x = double_nummax;
		min_vert.y = double_nummax;
		shared_type::coord_xyz_type max_vert;
		max_vert.x = double_nummin;
		max_vert.y = double_nummin;
		/*	Gets minimum and maximum vert within the grid	*/
		for (unsigned long a = 1u; a < vert_amount; ++a)
		{
			/*	Checks if current vert sits outside of the grid, if so, skips	*/
			if (!grid_bounds_check(verts_buffer[a].coord))
			{
				continue;
			}

			if (verts_buffer[a].coord.x < min_vert.x)
			{
				min_vert.x = verts_buffer[a].coord.x;
			}
			if (verts_buffer[a].coord.y < min_vert.y)
			{
				min_vert.y = verts_buffer[a].coord.y;
			}

			if (verts_buffer[a].coord.x > max_vert.x)
			{
				max_vert.x = verts_buffer[a].coord.x;

			}
			if (verts_buffer[a].coord.y > max_vert.y)
			{
				max_vert.y = verts_buffer[a].coord.y;
			}
		}

		/*

			Below, the compartment that encloses the min vert, as well as the compartment that enclsoses the max vert, are gotten.
			Padding is also applied to the bounding box (so that any local spline is able to sample out along it's full distance
			(as defined by "local_spline_length")). The padding does not extend outside of the grid however, it is clamped
			within it	*/
		{
			unsigned short padding = (((local_spline_length / 2u) + 1u) / volume_local.intern_df_cmprt_size) + 1u;
			min_cmprt = get_enclsing_cmprt(min_vert);
			if (min_cmprt.x >= padding)
			{
				min_cmprt.x -= padding;
			}
			else
			{
				min_cmprt.x = 0u;
			}

			if (min_cmprt.y >= padding)
			{
				min_cmprt.y -= padding;
			}
			else
			{
				min_cmprt.y = 0u;
			}
			max_cmprt_p1 = get_enclsing_cmprt(max_vert);

			max_cmprt_p1.x += padding;
			if (max_cmprt_p1.x >= volume_local.cmprt_amount.x)
			{
				max_cmprt_p1.x = volume_local.cmprt_amount.x;
			}
			else
			{
				++max_cmprt_p1.x;
			}
			max_cmprt_p1.y += padding;
			if (max_cmprt_p1.y >= volume_local.cmprt_amount.y)
			{
				max_cmprt_p1.y = volume_local.cmprt_amount.y;
			}
			else
			{
				++max_cmprt_p1.y;
			}
		}
		/*	The below 2 for loops iterates through all (x, y)grid indices within the bounding box, and creates a spline at each which
			goes through each grid point sharing the same (x, y) indices (all vertially aligned grid points)	*/
		for (unsigned short a = min_cmprt.x; a < max_cmprt_p1.x; ++a)
		{
			for (unsigned short b = min_cmprt.y; b < max_cmprt_p1.y; ++b)
			{
				/*	Loop through points in cmprt	*/
				unsigned long enclsing_cmprts_linear = ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * a) + (volume_local.cmprt_amount.z * b) + 1;
				for (unsigned long c = 0u; c < volume_local.cmprt_meta[enclsing_cmprts_linear].size; ++c)
				{
					const grid_type& current_grid_point = *volume_local.cmprt_table[enclsing_cmprts_linear][c];

					if (current_grid_point.temp_spline_indx == 0)
					{
						unsigned long zaligned_splines_size = zaligned_splines.size();
						double* coords = new double[volume_local.grid_amount.z];

						/*	The below for loop iterates through each vertically aligned grid point	*/
						for (unsigned long d = 0u; d < volume_local.grid_amount.z; ++d)
						{
							/*	Adds spline index to grid point	*/
							volume_local.grid[current_grid_point.index.x][current_grid_point.index.y][d].temp_spline_indx = zaligned_splines_size;
							/*	Adds grid point's max value to coord (only considering layers belonging to the specified dfcs)	*/
							coords[d] = volume_local.grid[current_grid_point.index.x][current_grid_point.index.y][d].get_max_value(dfc_ids);
						}

						shared_type::index_xy_type point_indx;
						point_indx.x = current_grid_point.index.x;
						point_indx.y = current_grid_point.index.y;
						/*	Creates spline at (x, y) index using gathered knotts	*/
						zaligned_splines.push_back(shared_type::ncspline_type(coords, volume_local.grid_amount.z, point_indx));
						delete[] coords;
					}
				}
			}
		}
	}

	unsigned long internal_vert_amount = 0ul;
	/*	Checks if current vert sits outside of the grid	*/
	for (unsigned long a = 0u; a < vert_amount; ++a)
	{
		if (grid_bounds_check(verts_buffer[a].coord))
		{
			++internal_vert_amount;
		}
		else
		{
			verts_buffer[a].value = 2.0f;
		}
	}

	{
		unsigned long jobs_completed = 0ul;
		std::mutex token;
		thread_pool.set_jobs_per_iteration(10u);
		typedef void (*job)(void*, unsigned short);
		job job_ptr = &call_df_get_lerped_point_value;
		const unsigned long batch_max_size = 200;
		const unsigned long batch_amount = (internal_vert_amount / batch_max_size) + 1ul;
		get_lerped_point_value_args_type args;
		args.dfc_ids = &dfc_ids;
		args.mode = interp_mode;
		args.gamma = gamma;
		args.zaligned_splines = &zaligned_splines;
		args.local_spline_length = local_spline_length;
		args.jobs_completed = &jobs_completed;
		args.jobs_completed_table = new unsigned long[batch_amount] {};
		args.token = &token;
		unsigned long vert_index = 0ul;
		/*	The below for loop iterates through each vert passed to the function (which would be every vert in the current dfr)	*/
		for (unsigned long a = 0ul; a < batch_amount; ++a)
		{
			const unsigned long current_batch_size = (a == (batch_amount - 1ul)) ? (internal_vert_amount % batch_max_size) : batch_max_size;
			get_lerped_point_value_local_args_type* local_args = new get_lerped_point_value_local_args_type[current_batch_size];
			for (unsigned long b = 0ul; b < current_batch_size; ++vert_index)
			{
				if (verts_buffer[vert_index].value == 2.0f)
				{
					verts_buffer[vert_index].value = .0f;
					continue;
				}
				local_args[b].a = a;
				local_args[b].current_batch_size = current_batch_size;
				local_args[b].vert_coord = &verts_buffer[vert_index].coord;
				local_args[b].value = &verts_buffer[vert_index].value;
				local_args[b].args = &args;
				++b;
			}
			bool add_job_success = false;
			/*	Attempts to send job to thread pool	*/
		attempt_to_add_job:
			add_job_success = thread_pool.add_jobs(job_ptr, current_batch_size, (void*)local_args);
			/*	Checks if attempt was successfull	*/
			if (add_job_success)
			{
				goto job_sent;
			}
			else
			{
				goto wait_for_space_in_job_stack;
			}

		wait_for_space_in_job_stack:
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
				if (thread_pool.job_amount < (batch_max_size * 2))
				{
					goto attempt_to_add_job;
				}
			}
		job_sent:

			continue;
		}

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
			token.lock();
			if (jobs_completed == internal_vert_amount)
			{
				token.unlock();
				break;
			}
			token.unlock();
		}
		delete[] args.jobs_completed_table;
	}

	/*	Resets the "temp_spline_indx" data member in each grid point enclosed within the bounding box to 0	*/
	if (interp_mode == 0)
	{
		for (unsigned short a = min_cmprt.x; a < max_cmprt_p1.x; ++a)
		{
			for (unsigned short b = min_cmprt.y; b < max_cmprt_p1.y; ++b)
			{
				/*	Loop through points in cmprt	*/
				unsigned long enclsing_cmprts_linear = ((volume_local.cmprt_amount.z * volume_local.cmprt_amount.y) * a) + (volume_local.cmprt_amount.z * b) + 1;
				for (unsigned long c = 0u; c < volume_local.cmprt_meta[enclsing_cmprts_linear].size; ++c)
				{
					const grid_type& current_grid_point = *volume_local.cmprt_table[enclsing_cmprts_linear][c];
					if (current_grid_point.temp_spline_indx > 0u)
					{
						/*	The below for loop iterates through each vertically aligned grid point	*/
						for (unsigned long d = 0u; d < volume_local.grid_amount.z; ++d)
						{
							/*	Resets "temp_spline_indx" to 0	*/
							volume_local.grid[current_grid_point.index.x][current_grid_point.index.y][d].temp_spline_indx = 0u;
						}
					}
				}
			}
		}
	}
	return 0;
}


int df_type::df_map_map_texel(void* args_ptr, unsigned short job_index)
{
	df_map_map_texel_local_args_type* args_ptr_convrtd = (df_map_map_texel_local_args_type*)args_ptr;
	df_map_map_texel_local_args_type* local_args = &args_ptr_convrtd[job_index];
	df_map_map_texel_args_type* args = local_args->args;

	/*	Find enclosing triangle	*/
	unsigned long c;
	shared_type::coord_uvw_type* bc_coord_cache = new shared_type::coord_uvw_type[args->tri_amount];
	shared_type::coord_xyz_type normal(0, 0, 1.0);
	for (c = 0ul; c < args->tri_amount; ++c)
	{
		bc_coord_cache[c] = shared.cartesian_to_barycentric(args->tris_uv_buffer[c].uv_vert_0, args->tris_uv_buffer[c].uv_vert_1, args->tris_uv_buffer[c].uv_vert_2, local_args->texel_coord, normal);
		if (((bc_coord_cache[c].u >= 0) && (bc_coord_cache[c].v >= 0) && (bc_coord_cache[c].w >= 0)))
		{
			break;
		}
	}
	if (c != args->tri_amount)
	{
		args->df_map_linear[local_args->linear_index].coord = shared.barycentric_to_cartesian(args->verts_buffer[args->tris_buffer[c].vert_0], args->verts_buffer[args->tris_buffer[c].vert_1], args->verts_buffer[args->tris_buffer[c].vert_2], bc_coord_cache[c]);
		args->extern_texels_proj[local_args->linear_index] = shared_type::index_xy_type(2u, 2u);
	}
	else
	{
		shared_type::coord_xyz_type nearest_proj_point;
		unsigned long nearest_tri = 0ul;
		double min_dist_sqr = (std::numeric_limits<double>::max)();
		for (unsigned long d = 0u; d < args->tri_amount; ++d)
		{
			shared_type::coord_xyz_type* edge_vert_a = nullptr;
			shared_type::coord_xyz_type* edge_vert_b = nullptr;
			if (bc_coord_cache[d].u < .0)
			{
				edge_vert_a = &args->tris_uv_buffer[d].uv_vert_1;
				edge_vert_b = &args->tris_uv_buffer[d].uv_vert_2;
			}
			else if (bc_coord_cache[d].v < .0)
			{
				edge_vert_a = &args->tris_uv_buffer[d].uv_vert_0;
				edge_vert_b = &args->tris_uv_buffer[d].uv_vert_2;
			}
			else if (bc_coord_cache[d].w < .0)
			{
				edge_vert_a = &args->tris_uv_buffer[d].uv_vert_0;
				edge_vert_b = &args->tris_uv_buffer[d].uv_vert_1;
			}
			else
			{
				std::cout << "Invalid extern texel" << std::endl;
			}
			/*	"at_dist" is the distance vector from edge_vert_a to the current texel	*/
			shared_type::coord_xy_type at_dist(local_args->texel_coord.x - edge_vert_a->x, local_args->texel_coord.y - edge_vert_a->y);
			/*	"at_dist" is the distance vector from edge_vert_a to edge_vert_b	*/
			shared_type::coord_xy_type ab_dist(edge_vert_b->x - edge_vert_a->x, edge_vert_b->y - edge_vert_a->y);
			double ab_len = std::sqrt((ab_dist.x * ab_dist.x) + (ab_dist.y * ab_dist.y));
			shared_type::coord_xy_type ab_normal(ab_dist.x / ab_len, ab_dist.y / ab_len);
			double t = (ab_normal.x * at_dist.x) + (ab_normal.y * at_dist.y);
			shared_type::coord_xy_type ap_dist(ab_normal.x * t, ab_normal.y * t);
			double ab_dot_ab = (ab_dist.x * ab_dist.x) + (ab_dist.y * ab_dist.y);
			double ab_dot_ap = (ab_dist.x * ap_dist.x) + (ab_dist.y * ap_dist.y);
			shared_type::coord_xyz_type proj_point = (ab_dot_ap < .0) ? *edge_vert_a : ((ab_dot_ap > ab_dot_ab) ? *edge_vert_b : shared_type::coord_xyz_type(edge_vert_a->x + ap_dist.x, edge_vert_a->y + ap_dist.y, .0));
			shared_type::coord_xy_type pt_dist(local_args->texel_coord.x - proj_point.x, local_args->texel_coord.y - proj_point.y);
			double dist_sqr = (pt_dist.x * pt_dist.x) + (pt_dist.y * pt_dist.y);
			if (dist_sqr < min_dist_sqr)
			{
				min_dist_sqr = dist_sqr;
				nearest_proj_point = proj_point;
			}
		}
		args->extern_texels_proj[local_args->linear_index] = (std::sqrt(min_dist_sqr) <= args->padding) ? shared_type::index_xy_type(args->width * nearest_proj_point.x, args->height * nearest_proj_point.y) : shared_type::index_xy_type(2u, 2u);
		args->df_map_linear[local_args->linear_index].coord = volume_local.min_grid_coord;
		args->df_map_linear[local_args->linear_index].coord.x -= 1.0;
	}
	delete[] bc_coord_cache;
	args->token->lock();
	++*args->jobs_completed;
	++args->jobs_completed_table[local_args->a];
	if (args->jobs_completed_table[local_args->a] == args->width)
	{
		args->token->unlock();
		delete[] args_ptr;
		return 0;
	}
	args->token->unlock();

	return 0;
}



int df_type::update_recipient_df_map(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::coord_xyz_type* verts_buffer, const unsigned long vert_amount, shared_type::tri_info_type* tris_buffer, shared_type::tri_uv_info_type* tris_uv_buffer, const unsigned long tri_amount, const unsigned short height, const unsigned short width, const int interp_mode, const float gamma, const char* dir, const char* name, float padding)
{
	/*	Recently discovered single line conditions. Is big epic	*/
	padding = (padding < .0f) ? (padding = (std::numeric_limits<float>::max)()) : padding;
	unsigned long texel_amount_total = height * width;
	shared_type::coord_xy_type texel_dim;
	shared_type::coord_xy_type texel_half_dim;
	texel_dim.x = 1.0 / (double)width;
	texel_half_dim.x = texel_dim.x / 2.0;
	texel_dim.y = 1.0 / (double)height;
	texel_half_dim.y = texel_dim.y / 2.0;

	/*	"vert_info_type" is just being used here to represent the texels as that's to be the type expected by "update_recipient"
		(that function was made before df maps were implemented, back when only vert colors and vert groups were writted to.
		As a result it was assumed that only vertex info would be passed though it, though obvisously that is no longer the case)	*/

	std::cout << "BEGINNING OF DF MAP" << std::endl;

	shared_type::vert_info_type* df_map_linear = new shared_type::vert_info_type[texel_amount_total];
	shared_type::index_xy_type* extern_texels_proj = new shared_type::index_xy_type[texel_amount_total];
	shared_type::coord_xyz_type* texel_coord_cache = new shared_type::coord_xyz_type[texel_amount_total];
	{
		typedef void (*job)(void*, unsigned short);
		job job_ptr = &call_df_df_map_map_texel;;
		unsigned long jobs_completed = 0ul;
		std::mutex token;
		df_map_map_texel_args_type arg;
		arg.df_map_linear = df_map_linear;
		arg.extern_texels_proj = extern_texels_proj;
		arg.height = height;
		arg.width = width;
		arg.jobs_completed = &jobs_completed;
		arg.padding = padding;
		arg.token = &token;
		arg.tris_buffer = tris_buffer;
		arg.tris_uv_buffer = tris_uv_buffer;
		arg.tri_amount = tri_amount;
		arg.verts_buffer = verts_buffer;
		arg.jobs_completed_table = new unsigned short[height] {};

		/*	Number is based on profiling	*/
		thread_pool.set_jobs_per_iteration(60u);

		for (unsigned short a = 0u; a < height; ++a)
		{
			shared_type::coord_xyz_type texel_coord;
			texel_coord.y = texel_half_dim.y + (texel_dim.y * (double)a);
			df_map_map_texel_local_args_type* args = new df_map_map_texel_local_args_type[width];
			for (unsigned short b = 0u; b < width; ++b)
			{
				texel_coord.x = texel_half_dim.x + (texel_dim.x * (double)b);
				unsigned long linear_index = (a * width) + b;
				args[b].a = a;
				args[b].texel_coord = texel_coord;
				args[b].linear_index = linear_index;
				args[b].args = &arg;
			}
			/*	Attempts to send job to thread pool	*/
			bool add_job_success = false;
		attempt_to_add_job:
			add_job_success = thread_pool.add_jobs(job_ptr, width, (void*)args);
			/*	Checks if attempt was successfull	*/
			if (add_job_success)
			{
				goto job_sent;
			}
			else
			{
				goto wait_for_space_in_job_stack;
			}

		wait_for_space_in_job_stack:
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
				if (thread_pool.job_amount < (width * 2u))
				{
					goto attempt_to_add_job;
				}
			}
		job_sent:

			continue;
		}

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(100000));
			token.lock();
			if (jobs_completed == ((unsigned long)width * (unsigned long)height))
			{
				token.unlock();
				break;
			}
			token.unlock();
		}
	}

	

	std::cout << "CALLING UPDATE RECIPIENT FROM WITHIN DF MAP" << std::endl;
	this->update_recipient(dfc_layers, dfc_layers_nxt_indx, df_map_linear, texel_amount_total, interp_mode, gamma);

	std::cout << "UPDATE RECIPIENT FINISHED FROM WITHIN DF MAP" << std::endl;

	unsigned char** df_map = new unsigned char* [height];
	for (unsigned short a = 0u; a < height; ++a)
	{
		df_map[a] = new unsigned char[width];
		for (unsigned short b = 0u; b < width; ++b)
		{
			unsigned long linear_index = (a * width) + b;
			/*	if bc_coord is not all zeros then is external (internal texels are left as all zeros)
				(a valid bc coord of all zeros is impossible, so this is a safe test)	*/

			if (extern_texels_proj[linear_index] != shared_type::index_xy_type(2u, 2u))
			{
				shared_type::index_xy_type& proj_index = extern_texels_proj[linear_index];
				unsigned long proj_linear_index = (proj_index.y * width) + proj_index.x;
				if (extern_texels_proj[proj_linear_index] == shared_type::index_xy_type(2u, 2u))
				{
					df_map_linear[linear_index].value = df_map_linear[proj_linear_index].value;
					goto set_df_map_value;
				}
				else
				{
					for (short c = -1; c < 2; ++c)
					{
						short offset_y = ((c == -1) && (proj_index.y == 0u)) ? 0u : (((c == 1) && (proj_index.y == (height - 1u))) ? 0u : c);
						for (short d = -1; d < 2; ++d)
						{
							short offset_x = ((d == -1) && (proj_index.x == 0u)) ? 0u : (((d == 1) && (proj_index.y == (height - 1u))) ? 0u : d);
							shared_type::index_xy_type kernal_texel_index((short)proj_index.x + offset_x, (short)proj_index.y + offset_y);
							unsigned long kernal_linear_index = (kernal_texel_index.y * width) + kernal_texel_index.x;
							if (extern_texels_proj[kernal_linear_index] == shared_type::index_xy_type(2u, 2u))
							{
								df_map_linear[linear_index].value = df_map_linear[kernal_linear_index].value;
								goto set_df_map_value;
							}
						}
					}
				}
			}
		set_df_map_value:
			df_map[a][b] = df_map_linear[linear_index].value * 255.0f;
		}
	}

	png_code_type png_code(name, df_map, width, height);
	png_code.write_to_file(dir);

	/*	Deallocate dynamically allocated objects	*/
	delete[] df_map_linear;
	for (unsigned short a = 0u; a < height; ++a)
	{
		delete[] df_map[a];
	}
	delete[] df_map;
	delete[] texel_coord_cache;

	return 0;
}


/*	Currently nop	*/
int df_type::post_update_recipients()
{
	return 0;
}


int df_type::clean()
{
	volume_local.clean();
	update_local.deep_clean();
	clean_df_layers();
	return 0;
}


/*	The clean functions are seperated into distinct functions for certain classes in order to give more control over which data members
	are cleaned	(this was done when the dfcache and df stashing systems were being put in place as having a single function that nuked
	everything didn't work particularly well). For this function in particular, there are certain things that one wants to keep in memory
	while stashing, and those things are not cleaned in "df_type::clean", instead, one must call that AND the below function in order to
	remove everything including said things (one would wan't to do this when, for example, loading a new blend file).	*/
int df_type::clean_special()
{
	volume_local.clean_special();
	write_index = 0u;
	undo_history.clean();
	regions_buffer.clean();
	return 0;
}


/*Checks if positions of the volume verts passed to the function are equal to the positions that the volume verts were in when the volume was initialized*/
/*Is currently a poor implementation as it will return that they are not equal if the order of the verts is different, but the actual positions are the same*/
int df_type::check_volume(const shared_type::coord_xyz_type* volume_verts)
{
	bool same = true;

	for (short a = 0; a < 8; ++a)
	{
		if ((volume_local.intern_volume_verts[a].x != volume_verts[a].x) ||
			(volume_local.intern_volume_verts[a].y != volume_verts[a].y) ||
			(volume_local.intern_volume_verts[a].z != volume_verts[a].z))
		{
			same = false;

			break;
		}
	}

	return same;
}


/*	dfc_ids can become fragmented, in that, you could have 100 dfc ids, and then remove a bunch at random, and end up having a dfc with id 99, when there's only 10 dfcs remaining.
	This becomes an issue when you then try to go about determining which id to assign to a new dfc, as the current implementation just assigns the next id in a counter, which in the
	above case would still be 100, even though there are only 10 dfcs. To avoid this the below function derfags the dfc_ids, though it should only be called every so often (such as
	once the next index counter exceeds a certain threshold) to avoid a perf hit */
int df_type::defrag_dfc_ids(unsigned long* dfc_ids, const unsigned long& dfc_amount, int& greatest_id)
{
	greatest_id = 0;

	for (unsigned long a = 0; a < dfc_amount; ++a)
	{
		if (dfc_ids[a] > 0)
		{
			update_local.dfc_cache.dfc_ids.calc_size();
			for (unsigned long b = 0; b < update_local.dfc_cache.dfc_ids.size; ++b)
			{
				if (update_local.dfc_cache.dfc_ids.vector[b] != nullptr)
				{
					if (dfc_ids[a] == update_local.dfc_cache.dfc_ids.vector[b]->id)
					{
						update_local.dfc_cache.dfc_ids.vector[b]->id = (a + 1);
						break;
					}
				}
			}

			dfc_ids[a] = (a + 1);
			if (dfc_ids[a] > greatest_id)
			{
				greatest_id = dfc_ids[a];
			}	
		}	
	}

	return 0;
}


/*	Same as above function, but for dfr_ids instead of dfc_ids (probably look into merging these into a single "index_defragment" function to avoid code repitition)	*/
int df_type::defrag_dfr_ids(unsigned long* dfr_ids, const unsigned long& dfr_amount, int& greatest_id)
{
	greatest_id = 0;

	for (unsigned long a = 0; a < dfr_amount; ++a)
	{
		if (dfr_ids[a] > 0)
		{
			update_local.dfr_cache.dfr_ids.calc_size();
			for (unsigned long b = 0; b < update_local.dfr_cache.dfr_ids.size; ++b)
			{
				if (update_local.dfr_cache.dfr_ids.vector[b] != nullptr)
				{
					if (dfr_ids[a] == update_local.dfr_cache.dfr_ids.vector[b]->id)
					{
						update_local.dfr_cache.dfr_ids.vector[b]->id = (a + 1);
						break;
					}
				}
			}

			dfr_ids[a] = (a + 1);
			if (dfr_ids[a] > greatest_id)
			{
				greatest_id = dfr_ids[a];
			}
		}
	}

	return 0;
}


/*	Returns the amount of dfc ids	*/
unsigned long df_type::get_dfc_id_size()
{
	return update_local.dfc_cache.dfc_ids.vector.size();
}


/*	Returns the amount of dfr ids	*/
unsigned long df_type::get_dfr_id_size()
{
	return update_local.dfr_cache.dfr_ids.vector.size();
}


/*	Note that dfc and dfr layers refer to the layers visible in the UI, not the df grid point layers	*/	


/*	Returns the amount of dfc within the specified dfc layer	*/
unsigned long df_type::get_dfc_layer_size(const unsigned long& layer_indx)
{
	return dfc_layers[layer_indx].vector.size();
}


/*	Returns the amount of dfr within the specified dfr layer	*/
unsigned long df_type::get_dfr_layer_size(const unsigned long& layer_indx)
{
	return dfr_layers[layer_indx].vector.size();
}


/*	Adds a new dfc layers	*/
int df_type::add_dfc_layer()
{
	
	shared_type::byte_vec_type message;
	/*
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('W');
	message.char_vec.push_back('o');
	message.char_vec.push_back('r');
	message.char_vec.push_back('l');
	message.char_vec.push_back('d');
	message.char_vec.push_back('!');
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('W');
	message.char_vec.push_back('o');
	message.char_vec.push_back('r');
	message.char_vec.push_back('l');
	message.char_vec.push_back('d');
	message.char_vec.push_back('!');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('W');
	message.char_vec.push_back('o');
	message.char_vec.push_back('r');
	message.char_vec.push_back('l');
	message.char_vec.push_back('d');
	message.char_vec.push_back('!');
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('H');
	message.char_vec.push_back('e');
	message.char_vec.push_back('l');
	message.char_vec.push_back('l');
	message.char_vec.push_back('o');
	message.char_vec.push_back(' ');
	message.char_vec.push_back('W');
	*/

	/*
	std::string message_str("Test text");
	for (unsigned short a = 0u; a < message_str.size(); ++a)
	{
		message.char_vec.push_back(message_str[a]);
	}
	deflate_code_type deflate_code(message);
	*/
	/*
	unsigned char** image = new unsigned char*[512];
	for (unsigned short a = 0u; a < 512; ++a)
	{
		image[a] = new unsigned char[512] {};
		if ((a >= 100) && (a <= 300))
		{
			for (unsigned short b = 0; b < 512; ++b)
			{
				image[a][b] = 128;
			}
		}
	}
	std::string name("png_test");
	png_code_type png(name, image, 512u, 512u);
	png.write_to_file(std::string("E:/workshop_folders/lost_cosmonauts/LostCosmonautsTools/LostCosmonautsTools/df_png_export_test/"));
	*/

	dfc_layers.push_back(shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>());
	return 0;
}


/*	Adds a new dfr layer	*/
int df_type::add_dfr_layer()
{
	dfr_layers.push_back(shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>());
	return 0;
}


/*	Removes dfc layer	*/
int df_type::remove_dfc_layer(const unsigned long& layer_indx, unsigned long* expelled_dfcs, unsigned long& expelled_dfcs_nxt_indx)
{
	/*	Removes layer	*/
	dfc_layers.erase(dfc_layers.begin() + layer_indx);
	
	/*	Now that the layer is removed, tests to see if any of the dfcs in said layer exist in any other layers, if not, mark them as expelled	*/
	unsigned long dfc_layers_nxt_indx = dfc_layers.size();
	for (unsigned long a = 0; a < update_local.dfc_cache.dfc_ids.vector.size(); ++a)
	{
		for (unsigned long b = 0; b < dfc_layers_nxt_indx; ++b)
		{
			if (get_dfc_in_dfc_layer(b, update_local.dfc_cache.dfc_ids.vector[a]->id) != nullptr)
			{
				goto dont_expel;
			}
		}
		goto expel;

	dont_expel:
		continue;

	expel:
		
		/*	The list of expelled dfcs is for use on the python side	*/
		expelled_dfcs[expelled_dfcs_nxt_indx] = update_local.dfc_cache.dfc_ids.vector[a]->id;
		expelled_dfcs_nxt_indx += 1;
		
		/*	Deletes expelled dfc id (though currently only does so if cache is valid (make this not so, but test))	*/
		if (update_local.dfc_cache.dfc_id_to_indx(update_local.dfc_cache.dfc_ids.vector[a]->id, 0) < 0ll)
		{
			delete update_local.dfc_cache.dfc_ids.vector[a];
			dfc_id_indx_type** moved_dfc_id = update_local.dfc_cache.dfc_ids.remove_element(a);
			/*	Corrects index component of moved dfc id (remember that the dfc ids use an inverse jenga class) 	*/
			if (moved_dfc_id != nullptr)
			{
				(*moved_dfc_id)->index = a;
				a -= 1;
			}
		}
	}
	return 0;
}


/*	Removes dfr layer	*/
int df_type::remove_dfr_layer(const unsigned long& layer_indx, unsigned long* expelled_dfrs, unsigned long& expelled_dfrs_nxt_indx)
{
	/*	Removes layer	*/
	dfr_layers.erase(dfr_layers.begin() + layer_indx);

	unsigned long dfr_layers_nxt_indx = dfr_layers.size();
	for (unsigned long a = 0; a < update_local.dfr_cache.dfr_ids.vector.size(); ++a)
	{
		/*	Now that the layer is removed, tests to see if any of the dfrs in said layer exist in any other layers, if not, mark them as expelled	*/
		for (unsigned long b = 0; b < dfr_layers_nxt_indx; ++b)
		{
			if (get_dfr_in_dfr_layer(b, update_local.dfr_cache.dfr_ids.vector[a]->id) != nullptr)
			{
				goto dont_expel;
			}
		}
		goto expel;

	dont_expel:
		continue;

	expel:

		/*	The list of expelled dfrs is for use on the python side	*/
		expelled_dfrs[expelled_dfrs_nxt_indx] = update_local.dfr_cache.dfr_ids.vector[a]->id;
		expelled_dfrs_nxt_indx += 1;

		/*	Deletes expelled dfr id	*/
		delete update_local.dfr_cache.dfr_ids.vector[a];
		dfr_id_indx_type** moved_dfr_id = update_local.dfr_cache.dfr_ids.remove_element(a);
		/*	Corrects index component of moved dfr id (remember that the dfr ids use an inverse jenga class) 	*/
		if (moved_dfr_id != nullptr)
		{
			(*moved_dfr_id)->index = a;
			a -= 1;
		}
	}
	return 0;
}


/*	Assignes dfc's to specified dfc layer	*/
int df_type::assign_dfcs_to_dfc_layer(const unsigned long& layer_indx, const unsigned long* dfc_ids, const unsigned long& dfc_ids_nxt_indx)
{
	/*	Bound checks specified layer index	*/
	if (layer_indx < dfc_layers.size())
	{
		/*	The below for loop iterates through each dfc id to be added	*/
		for (unsigned long a = 0; a < dfc_ids_nxt_indx; ++a)
		{
			
			dfc_id_indx_type* intern_dfc_id = update_local.dfc_cache.get_dfc_id(dfc_ids[a]);

			/*	If the current dfc id does not exist yet in the list of dfc ids, creates one	*/
			if (intern_dfc_id == nullptr)
			{
				dfc_id_indx_type* dfc_id_to_add = new dfc_id_indx_type;
				dfc_id_to_add->id = dfc_ids[a];
				dfc_id_to_add->index = update_local.dfc_cache.dfc_ids.vector.size();
				update_local.dfc_cache.dfc_ids.vector.push_back(dfc_id_to_add);

				intern_dfc_id = dfc_id_to_add;
			}
			/*	Else, if the current dfc id does already exist in the list of dfc ids, checks if it already exists within the specified
				dfc layer, if so, skip	*/
			else
			{
				if (get_dfc_in_dfc_layer(layer_indx, dfc_ids[a]) != nullptr)
				{
					continue;
				}
			}

			/*	Adds dfc id to specified layer	*/
			dfc_layers[layer_indx].vector.push_back(intern_dfc_id);
		}
	}
	return 0;
}


/*	Assignes dfr's to specified dfr layer	*/
int df_type::assign_dfrs_to_dfr_layer(const unsigned long& layer_indx, const unsigned long* dfr_ids, const unsigned long& dfr_ids_nxt_indx)
{
	/*	Bound checks specified layer index	*/
	if (layer_indx < dfr_layers.size())
	{
		/*	The below for loop iterates through each dfr id to be added	*/
		for (unsigned long a = 0; a < dfr_ids_nxt_indx; ++a)
		{
			dfr_id_indx_type* intern_dfr_id = update_local.dfr_cache.get_dfr_id(dfr_ids[a]);

			/*	If the current dfr id does not exist yet in the list of dfr ids, creates one	*/
			if (intern_dfr_id == nullptr)
			{
				dfr_id_indx_type* dfr_id_to_add = new dfr_id_indx_type;
				dfr_id_to_add->id = dfr_ids[a];
				dfr_id_to_add->index = update_local.dfr_cache.dfr_ids.vector.size();
				update_local.dfr_cache.dfr_ids.vector.push_back(dfr_id_to_add);

				intern_dfr_id = dfr_id_to_add;
			}
			/*	Else, if the current dfr id does already exist in the list of dfr ids, checks if it already exists within the specified
				dfr layer, if so, skip	*/
			else
			{
				if (get_dfr_in_dfr_layer(layer_indx, dfr_ids[a]) != nullptr)
				{
					continue;
				}
			}

			/*	Adds dfr id to specified layer	*/
			dfr_layers[layer_indx].vector.push_back(intern_dfr_id);
		}
	}
	return 0;
}


/*	Removes specified dfc(s) from specified dfc layer	*/
int df_type::remove_dfcs_from_dfc_layer(const unsigned long& layer_indx, unsigned long* dfcs, unsigned long& dfcs_nxt_indx, const bool enable_expel)
{
	/*	Bound checks specified layer index	*/
	if (layer_indx < dfc_layers.size())
	{
		unsigned long expelled_dfcs_nxt_indx = 0;

		/*	The below for loop iterates through each speficied dfc	*/
		for (unsigned long a = 0; a < dfcs_nxt_indx; ++a)
		{
			dfc_id_indx_type* intern_dfc_id = nullptr;
			bool expel = true;
			unsigned long dfc_layers_nxt_indx = dfc_layers.size();
			/*	The below for loop iterates through each dfc layer	*/
			for (unsigned long b = 0; b < dfc_layers_nxt_indx; ++b)
			{
				unsigned long dfc_layer_indx = 0;
				/*	Checks if the current dfc exists within the current layer	*/
				intern_dfc_id = get_dfc_in_dfc_layer(dfc_layer_indx, b, dfcs[a]);

				/*	If the dfc does exist in the current layer, execute the below compound statement	*/
				if (intern_dfc_id != nullptr)
				{
					/*	If the current layer happens to be the layer the dfc is specified to be removed from, remove dfc id from layer	*/
					if (b == layer_indx)
					{
						dfc_layers[layer_indx].remove_element(dfc_layer_indx);
					}
					/*	Else, set expelled to false (as there is a layer (the current layer), other than the layer the dfc is specified
						to be removed from, that contains the dfc)	*/
					else
					{
						if (expel == true)
						{
							expel = false;
						}
					}
				}
			}

			if ((expel == true) && (enable_expel == true))
			{
				/*	Add dfc to list of expelled dfcs (reusing the parameter "dfcs"), this is used on the python side	*/
				unsigned long dfc_id = dfcs[a];
				dfcs[expelled_dfcs_nxt_indx] = a;
				expelled_dfcs_nxt_indx += 1;

				/*	Deletes expelled dfc id (though currently only does so if cache is not valid (make this not so, but test))	*/
				if (update_local.dfc_cache.dfc_id_to_indx(dfc_id, 0) < 0ll)
				{
					/*	Check that dfc id actually exists in list of dfc ids	*/
					//dfc_id_indx_type* intern_dfc_id = update_local.dfc_cache.get_dfc_id(dfcs[a]);
					if (intern_dfc_id != nullptr)
					{
						dfc_id_indx_type** moved_dfc_id = update_local.dfc_cache.dfc_ids.remove_element(intern_dfc_id->index);
						/*	Corrects index component of moved dfc id (remember that the dfc ids use an inverse jenga class) 	*/
						if (moved_dfc_id != nullptr)
						{
							(*moved_dfc_id)->index = intern_dfc_id->index;
						}
						delete intern_dfc_id;
					}
				}
			}
		}

		if (enable_expel == true)
		{
			/*	Caps "dfcs" next index (remember that "dfcs" is being reused)	*/
			dfcs_nxt_indx = expelled_dfcs_nxt_indx;
		}
	}
	return 0;
}


int df_type::remove_dfrs_from_dfr_layer(const unsigned long& layer_indx, unsigned long* dfrs, unsigned long& dfrs_nxt_indx, const bool enable_expel)
{
	/*	Bound checks specified layer index	*/
	if (layer_indx < dfr_layers.size())
	{
		unsigned long expelled_dfrs_nxt_indx = 0;

		/*	The below for loop iterates through each speficied dfr	*/
		for (unsigned long a = 0; a < dfrs_nxt_indx; ++a)
		{
			dfr_id_indx_type* intern_dfr_id = nullptr;

			bool expel = true;
			unsigned long dfr_layers_nxt_indx = dfr_layers.size();
			/*	The below for loop iterates through each dfr layer	*/
			for (unsigned long b = 0; b < dfr_layers_nxt_indx; ++b)
			{
				unsigned long dfr_layer_indx = 0;
				/*	Checks if the current dfr exists within the current layer	*/
				intern_dfr_id = get_dfr_in_dfr_layer(dfr_layer_indx, b, dfrs[a]);

				/*	If the dfr does exist in the current layer, execute the below compound statement	*/
				if (intern_dfr_id != nullptr)
				{
					/*	If the current layer happens to be the layer the dfr is specified to be removed from, remove dfr id from layer	*/
					if (b == layer_indx)
					{
						dfr_layers[layer_indx].remove_element(dfr_layer_indx);
					}
					/*	Else, set expelled to false (as there is a layer (the current layer), other than the layer the dfr is specified
						to be removed from, that contains the dfr)	*/
					else
					{
						if (expel == true)
						{
							expel = false;
						}
					}
				}
			}

			if ((expel == true) && (enable_expel == true))
			{
				/*	Add dfr to list of expelled dfrs (reusing the parameter "dfrs"), this is used on the python side	*/
				unsigned long dfr_id = dfrs[a];
				dfrs[expelled_dfrs_nxt_indx] = a;
				expelled_dfrs_nxt_indx += 1;

				if (intern_dfr_id != nullptr)
				{
					/*	Deletes expelled dfr id	*/
					dfr_id_indx_type** moved_dfr_id = update_local.dfr_cache.dfr_ids.remove_element(intern_dfr_id->index);
					/*	Corrects index component of moved dfr id (remember that the dfr ids use an inverse jenga class) 	*/
					if (moved_dfr_id != nullptr)
					{
						(*moved_dfr_id)->index = intern_dfr_id->index;
					}
					delete intern_dfr_id;
				}
			}
		}

		/*	Caps "dfrs" next index (remember that "dfrs" is being reused)	*/
		dfrs_nxt_indx = expelled_dfrs_nxt_indx;
	}
	return 0;
}


/*	Returns all dfcs in the specified dfc layer	*/
int df_type::get_all_dfcs_in_dfc_layer(const unsigned long& layer_indx, unsigned long* dfc_ids)
{
	dfc_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfc_layers[layer_indx].size; ++a)
	{
		dfc_ids[a] = dfc_layers[layer_indx].vector[a]->id;
	}
	return 0;
}


/*	Returns all dfrs in the specified dfr layer	*/
int df_type::get_all_dfrs_in_dfr_layer(const unsigned long& layer_indx, unsigned long* dfr_ids)
{
	dfr_layers[layer_indx].calc_size();
	for (unsigned long a = 0; a < dfr_layers[layer_indx].size; ++a)
	{
		dfr_ids[a] = dfr_layers[layer_indx].vector[a]->id;
	}
	return 0;
}


int df_type::get_all_layers_with_dfc(const unsigned long dfc_id, unsigned long* layers, unsigned long& layers_nxt_indx)
{
	unsigned long dfc_layer_amount = dfc_layers.size();
	for (unsigned long a = 0u; a < dfc_layer_amount; ++a)
	{
		dfc_layers[a].calc_size();
		for (unsigned long b = 0u; b < dfc_layers[a].size; ++b)
		{
			if (dfc_layers[a].vector[b] != nullptr)
			{
				if (dfc_id == dfc_layers[a].vector[b]->id)
				{
					layers[layers_nxt_indx] = a;
					++layers_nxt_indx;
				}
			}
		}
	}

	return 0;
}


int df_type::get_all_layers_with_dfr(const unsigned long dfr_id, unsigned long* layers, unsigned long& layers_nxt_indx)
{
	unsigned long dfr_layer_amount = dfr_layers.size();
	for (unsigned long a = 0u; a < dfr_layer_amount; ++a)
	{
		dfr_layers[a].calc_size();
		for (unsigned long b = 0u; b < dfr_layers[a].size; ++b)
		{
			if (dfr_layers[a].vector[b] != nullptr)
			{
				if (dfr_id == dfr_layers[a].vector[b]->id)
				{
					layers[layers_nxt_indx] = a;
					++layers_nxt_indx;
				}
			}
		}
	}

	return 0;
}


int df_type::expel_nonexistant_dfcs_from_layers(const unsigned long* dfc_ids, const unsigned long dfc_amount)
{
	unsigned long layers_size = dfc_layers.size();
	for (unsigned long a = 0ul; a < layers_size; ++a)
	{
		dfc_layers[a].calc_size();
		for (unsigned long b = 0ul; b < dfc_layers[a].vector.size(); ++b)
		{
			for (unsigned long c = 0u; c < dfc_amount; ++c)
			{
				if (dfc_layers[a].vector[b]->id == dfc_ids[c])
				{
					goto exists;
				}
			}
			goto doesnt_exist;

		exists:

			continue;

		doesnt_exist:

			unsigned long dfc_array_wrapper[1] = { dfc_layers[a].vector[b]->id };
			unsigned long dfc_array_wrapper_size = 1ul;
			remove_dfcs_from_dfc_layer(a, dfc_array_wrapper, dfc_array_wrapper_size, true);
		}
	}

	return 0;
}


int df_type::expel_nonexistant_dfrs_from_layers(const unsigned long* dfr_ids, const unsigned long dfr_amount)
{
	unsigned long layers_size = dfr_layers.size();
	for (unsigned long a = 0ul; a < layers_size; ++a)
	{
		dfr_layers[a].calc_size();
		for (unsigned long b = 0ul; b < dfr_layers[a].vector.size(); ++b)
		{
			for (unsigned long c = 0u; c < dfr_amount; ++c)
			{
				if (dfr_layers[a].vector[b]->id == dfr_ids[c])
				{
					goto exists;
				}
			}
			goto doesnt_exist;

		exists:

			continue;

		doesnt_exist:

			unsigned long dfr_array_wrapper[1] = { dfr_layers[a].vector[b]->id };
			unsigned long dfr_array_wrapper_size = 1ul;
			remove_dfrs_from_dfr_layer(a, dfr_array_wrapper, dfr_array_wrapper_size, true);
		}
	}

	return 0;
}


/*	This doesn't seem to be called anywhere, can't recall what it was for exactly	*/
int df_type::load_dfc_dfr_layers_from_buffer(std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers, std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers)
{
	this->dfc_layers = dfc_layers;
	std::vector<shared_type::invrse_jenga_type<df_type::dfc_id_indx_type*, unsigned long>>().swap(dfc_layers);
	this->dfr_layers = dfr_layers;
	std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>().swap(dfr_layers);
	return 0;
}


/*	Copy's df state to buffer (usually either in preperation for writing to dfcache file, or to stash state)	*/
int df_type::copy_to_buffer()
{
	regions_buffer.prep_grid_is_valid(volume_local);
	regions_buffer.prep_write_id(write_id_buffer);
	regions_buffer.prep_grid_min_coord(volume_local);
	regions_buffer.prep_grid_spacing(volume_local);
	regions_buffer.prep_df_distance(volume_local);
	regions_buffer.prep_cmprt_size(volume_local);
	regions_buffer.prep_volume_verts(volume_local);
	regions_buffer.prep_grid_point_amount(volume_local);
	regions_buffer.prep_max_grid_point(volume_local);
	regions_buffer.prep_grid_meta(volume_local); //
	regions_buffer.prep_cmprt_data(volume_local); //
	regions_buffer.prep_dfc_amount(update_local);
	regions_buffer.prep_dfc_ids(update_local);
	regions_buffer.prep_dfc_cache_is_valid(update_local);
	regions_buffer.prep_vert_amount_total(update_local);
	regions_buffer.prep_vert_cache_next_index(update_local);
	regions_buffer.prep_vert_cache(update_local); //
	regions_buffer.prep_tri_cache_size(update_local);
	regions_buffer.prep_tri_cache(update_local); //
	regions_buffer.prep_mesh_cache(volume_local, update_local); //
	regions_buffer.prep_existing_dfcs(update_local);
	regions_buffer.prep_dfc_id_index_table(update_local);
	regions_buffer.prep_dfr_ids(update_local);
	regions_buffer.prep_dfc_layers(dfc_layers);
	regions_buffer.prep_dfr_layers(dfr_layers);
	regions_buffer.is_valid = true;

	return 0;
}


/*	Returns current "write_id_index" (this represents the number of times this df state has been written to a dfcache file),
	used for ensuring python and dfcache state are synced upon loading a dfcache file (ie, that the dfcache file being loaded
	is the correct one). Used in tandom with the "write_id_rand"	*/
int df_type::get_write_id_index()
{
	write_id_buffer.index = write_index;
	++write_index;
	return write_id_buffer.index;
}


/*	Returns a random number, is used for ensuring python and dfcache state are synced upon loading a dfcache file (ie, that the dfcache
	file being loaded is the correct one). Used in tandom with the "write_id_index"	*/
int df_type::get_write_id_rand()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	write_id_buffer.rand = mt();
	return write_id_buffer.rand;
}

/*	Stashes "write_id", only called if state stashing is enabled	*/
int df_type::stash_write_id()
{
	regions_buffer.prep_write_id(write_id_buffer);
	return 0;
}


/*	Handles the loading of a new blend file, specifically ensuring state consistency with the state saved within the blend file (the state
	on the python side). This involves loading, and validating, a dfcache file if one exists, and syncing state if one does not. Returns
	0 if cache file could not be opened, returns 1 if cache file was successfully opened and loaded to buffer, but was not in sync,
	returns 2 if cache file was in sync and successfully loaded to df state, returns 3 if df cache was disabled	*/
int df_type::new_blend_handler(const char* dir, const char* file_name, const shared_type::write_id_type& write_id, const int** dfc_layers, const int** dfr_layers, const bool df_cache_enabled)
{
	int return_code = 0;
	
	/*	Attempts to open cache file	*/
	df_loader_type loader(std::string(dir), std::string(file_name) + std::string(".dfcache"), volume_local, update_local, *this);

	if (!df_cache_enabled)
	{
		return_code = 3;
		goto load_layers_from_params;
	}

	/*	If attempt is unsuccessfull, returns 0	*/
	if (loader.file.is_open() == false)
	{
		return_code = 4;
		goto load_layers_from_params;
	}

	/*	If attempt is successful, loads contents of file into buffer	*/
	loader.director.load_to_buffer();

	/*	The purpose of the below if and compound statement is to check if the cache (which has now been loaded to buffer) is in sync
		with the state on the python (that is to say, that the cache is actually the right cache file). The first step is seen in the
		below condition, which checks that the write id on the python side matches that stored within the cache	*/
	if (write_id == *loader.director.write_id_ptr)
	{
		/*If the write ids match, the next step is to make sure that the dfc layers and dfr layers match	*/

		/*	Checks if amount of dfc layers on python side match	the amount in the cache	*/
		/*	Note that dfc_layers[0][0] is the amount of dfc_layers	*/
		if (loader.director.dfc_layers_ptr->size() != dfc_layers[0][0])
		{
			goto not_in_sync;
		}

		/*	Checks if the contents of the dfc layers match	*/
		{
			int param_size_full_0 = dfc_layers[0][0] + 1;
			for (int a = 1; a < param_size_full_0; ++a)
			{
				/*	Note thate dfc_layers[a][0] is the amount of dfc_ids in the layer	*/
				int param_size_full_1 = dfc_layers[a][0] + 1;
				for (int b = 1; b < param_size_full_1; ++b)
				{
					(*loader.director.dfc_layers_ptr)[a - 1].calc_size();
					if ((*loader.director.dfc_layers_ptr)[a - 1].size != dfc_layers[a][0])
					{
						goto not_in_sync;
					}

					for (int c = 0; c < (*loader.director.dfc_layers_ptr)[a - 1].size; ++c)
					{
						if (dfc_layers[a][b] == (*loader.director.dfc_layers_ptr)[a - 1].vector[b - 1]->id)
						{
							goto dfc_exists;
						}
					}
					goto dfc_doesnt_exist;

				dfc_exists:

					continue;

				dfc_doesnt_exist:

					goto not_in_sync;
				}
			}
		}

		/*	Checks if amount of dfr layers on python side match	the amount in the cache	*/
		/*	Note that dfr_layers[0] is the amount of dfr_layers	*/
		if (loader.director.dfr_layers_ptr->size() != *dfr_layers[0])
		{
			goto not_in_sync;
		}

		/*	Checks if the contents of the dfr layers match	*/
		{
			int param_size_full_0 = dfr_layers[0][0] + 1;
			for (int a = 1; a < param_size_full_0; ++a)
			{
				/*	Note that dfr_layers[a][0] is the amount of dfr_ids in the layer	*/
				int param_size_full_1 = dfr_layers[a][0] + 1;
				for (int b = 1; b < param_size_full_1; ++b)
				{
					(*loader.director.dfr_layers_ptr)[a - 1].calc_size();
					if ((*loader.director.dfr_layers_ptr)[a - 1].size != dfr_layers[a][0])
					{
						goto not_in_sync;
					}

					for (int c = 0; c < (*loader.director.dfr_layers_ptr)[a - 1].size; ++c)
					{
						if (dfr_layers[a][b] == (*loader.director.dfr_layers_ptr)[a - 1].vector[b - 1]->id)
						{
							goto dfr_exists;
						}
					}
					goto dfr_doesnt_exist;

				dfr_exists:
					continue;

				dfr_doesnt_exist:

					goto not_in_sync;
				}
			}
		}
	}
	else
	{
	not_in_sync:

		return_code = 1;
		goto load_layers_from_params;
	}

	/*	If flow has reached this statement, then it is assumed that the cache is in sync with the state on the python side	*/
	loader.director.load_to_df_state();
	return_code = 2;
	incrmt_undo_step(0);
	return return_code;

load_layers_from_params:

	/*	Constructs dfc and dfr layers from parameters (this would be done in the event that the dfcache is not found, disabled,
		or out of sync)	*/

	/*	Setups up dfc layers	*/
	if (dfc_layers != nullptr)
	{
		int param_size_full_0 = dfc_layers[0][0] + 1;
		for (int a = 1; a < param_size_full_0; ++a)
		{
			add_dfc_layer();

			int param_size_full_1 = dfc_layers[a][0] + 1;
			unsigned long* dfc_layers_no_meta = new unsigned long[dfc_layers[a][0]];
			unsigned long dfc_layers_no_meta_next_index = 0ul;
			for (int b = 1; b < param_size_full_1; ++b)
			{
				dfc_layers_no_meta[dfc_layers_no_meta_next_index] = dfc_layers[a][b];
				++dfc_layers_no_meta_next_index;
			}
			assign_dfcs_to_dfc_layer((a - 1), dfc_layers_no_meta, dfc_layers[a][0]);
			delete[] dfc_layers_no_meta;

		}
	}

	/*	Sets up dfr layers	*/
	if (dfr_layers != nullptr)
	{
		int param_size_full_0 = dfr_layers[0][0] + 1;
		for (int a = 1; a < param_size_full_0; ++a)
		{
			add_dfr_layer();

			int param_size_full_1 = dfr_layers[a][0] + 1;
			unsigned long* dfr_layers_no_meta = new unsigned long[dfr_layers[a][0]];
			unsigned long dfr_layers_no_meta_next_index = 0ul;
			for (int b = 1; b < param_size_full_1; ++b)
			{
				dfr_layers_no_meta[dfr_layers_no_meta_next_index] = dfr_layers[a][b];
				++dfr_layers_no_meta_next_index;
			}
			assign_dfrs_to_dfr_layer((a - 1), dfr_layers_no_meta, dfr_layers[a][0]);
			delete[] dfr_layers_no_meta;

		}
	}

	incrmt_undo_step(0);
	return return_code;
}


/*	Writes contents of "regions_buffer" to dfcache file (obviously this would be called after "copy_to_buffer" has been called)	*/
int df_type::write_cache(const char* dir, const char* file_name)
{
	df_writer_type writer(std::string(dir), std::string(file_name) + std::string(".dfcache"), regions_buffer);
	writer.write_to_file();
	return 0;
}

/*	Stashing state copies the current df state to the regions buffer, but doesn't write to file, as the purpose of copying over is
	to compress the current state through serialization. Because of this, the function also clears the df state after copying it.	
	It should be noted that this process takes a while, and doesn't really reduce memory usage that much, so in practice doesn't
	have much of a use. Weak stash would be preferable in virtually all cases (see description of "weak_stash_state" for more info")	*/
int df_type::stash_state()
{
	copy_to_buffer();
	clean();
	return 0;
}


/*	Unstashes state (see description of "stash_state" for more info)	*/
int df_type::unstash_state()
{
	if (regions_buffer.is_valid)
	{
		df_loader_type loader(regions_buffer, volume_local, update_local, *this);
		loader.director.load_to_buffer();
		loader.director.load_to_df_state();
	}
	return 0;
}


/*	Similiar concept to regular stash, but instead of compressing through serialization, it justs compresses by copying everything
	into more size efficient data structures and clearing the existing df state (though it should be noted that these data structures
	likely wouldn't be opitimal for acutally doing operations with, and so weak unstashing is still neccessary before doing anything
	with the data (such as initializing or updating the grid)). Weak stashing is significantly faster than regular stashing	*/
int df_type::weak_stash_volume_local()
{
	volume_local.write_to_buffer(volume_local.buffer);
	volume_local.clean();
	return 0;
}

/*	Weak unstashes state (see description of "weak_stash_state" for more info")	*/
int df_type::weak_unstash_volume_local()
{
	volume_local.load_from_buffer(volume_local.buffer, update_local.dfc_cache.dfc_ids);
	volume_local.clean_internal_buffer();
	return 0;
}


/*	This functions exists to be called from the thread pool (the deletion of rlvncy table buffers a the end of "initialize_volume"
	is multithreaded as iirc it provided a minor speed increase)	*/
int df_type::delete_rlvncy_buffers(void* args_ptr, unsigned short job_index)
{
	volume_local_type::delete_rlvncy_buffers_shared_local_type* args_ptr_casted = (volume_local_type::delete_rlvncy_buffers_shared_local_type*)args_ptr;
	volume_local_type::delete_rlvncy_buffers_local_type& local_state = *args_ptr_casted->args[job_index];
	for (unsigned long long a = local_state.start_index; a < local_state.end_next_index; ++a)
	{
		delete local_state.vector_arr[a];
	}
	args_ptr_casted->token.lock();
	++args_ptr_casted->jobs_completed;
	args_ptr_casted->token.unlock();
	return 0;
}


void df_type::clean_df_layers()
{
	{
		unsigned long size = dfc_layers.size();
		for (unsigned long a = 0u; a < size; ++a)
		{
			dfc_layers[a].clean();
		}
	}
	{
		unsigned long size = dfr_layers.size();
		for (unsigned long a = 0u; a < size; ++a)
		{
			dfr_layers[a].clean();
		}
	}
}


int df_type::validate_undo_step(const int python_index, const bool delete_further_steps)
{
	if (python_index == undo_history.active_undo_step->undo_index)
	{
		return 0;
	}
	else
	{
		undo_history.jump_to_closest_undo_step(python_index, delete_further_steps);
		undo_step_type temp(*undo_history.active_undo_step);

		/*	Update state	*/

		bool restash = false;
		if (volume_local.weak_stashed)
		{
			weak_unstash_volume_local();
			restash = true;
		}

		if (temp.grid_is_valid)
		{
			if (volume_local.grid != nullptr)
			{
				bool verts_match = true;
				for (unsigned short a = 0u; a < 8u; ++a)
				{
					if (volume_local.intern_volume_verts[a] != temp.intern_volume_verts[a])
					{
						verts_match = false;
						break;
					}
				}
				if ((verts_match) &&
					(volume_local.intern_df_distance == temp.intern_df_distance) &&
					(volume_local.intern_df_cmprt_size == temp.intern_df_cmprt_size) &&
					(volume_local.intern_grid_spacing == temp.intern_grid_spacing))
				{
					if (update_local.dfc_cache.is_valid)
					{
						for (unsigned long a = 0ul; a < update_local.dfc_cache.dfc_amount; ++a)
						{
							unsigned long mesh_amount = update_local.dfc_cache.mesh_cache[a]->size();
							for (unsigned long b = 0ul; b < mesh_amount; ++b)
							{
								update_local.dfc_cache.remove_mesh_layers(update_local.dfc_cache.dfc_indx_to_id_table[a]->id, b, false, volume_local);
							}
						}

						update_local.dfc_cache.clean();
					}
				}
				else
				{
					initialize_volume(temp.intern_volume_verts, temp.intern_df_distance, temp.intern_df_cmprt_size, temp.intern_grid_spacing, true);
				}
			}
		}
		else if (volume_local.grid != nullptr)
		{
			volume_local.clean();
			if (update_local.dfc_cache.is_valid)
			{
				update_local.dfc_cache.clean();
			}
		}
		
		if (restash)
		{
			weak_stash_volume_local();
		}
		
		clean_df_layers();
		update_local.dfc_cache.deep_clean_dfc_ids();
		update_local.dfr_cache.deep_clean_dfr_ids();

		update_local.dfc_cache.dfc_ids = temp.dfc_ids;
		temp.dfc_ids = shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>();
		update_local.dfr_cache.dfr_ids = temp.dfr_ids;
		temp.dfr_ids = shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>();
		dfc_layers = temp.dfc_layers;
		temp.dfc_layers = std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>();
		dfr_layers = temp.dfr_layers;
		temp.dfr_layers = std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>();
		
		return 0;
	}
}


int df_type::incrmt_undo_step(const int python_index)
{
	bool grid_is_valid = false;
	shared_type::coord_xyz_type* intern_volume_verts = nullptr;
	float intern_df_distance = .0f;
	unsigned short intern_df_cmprt_size = 0u;
	double intern_grid_spacing = .0;
	if (volume_local.weak_stashed)
	{
		if (volume_local.buffer.grid_is_valid)
		{
			grid_is_valid = true;
			intern_volume_verts = volume_local.buffer.volume_verts;
			intern_df_distance = volume_local.buffer.df_distance;
			intern_df_cmprt_size = volume_local.buffer.cmprt_size;
			intern_grid_spacing = volume_local.buffer.grid_spacing;
		}
	}
	else
	{
		if (volume_local.grid != nullptr)
		{
			grid_is_valid = true;
			intern_volume_verts = volume_local.intern_volume_verts;
			intern_df_distance = volume_local.intern_df_distance;
			intern_df_cmprt_size = volume_local.intern_df_cmprt_size;
			intern_grid_spacing = volume_local.intern_grid_spacing;
		}
	}

	undo_history.add_undo_step(	update_local.dfc_cache.dfc_ids,
								update_local.dfr_cache.dfr_ids,
								dfc_layers,
								dfr_layers,
								grid_is_valid,
								intern_volume_verts,
								intern_df_distance,
								intern_df_cmprt_size,
								intern_grid_spacing,
								python_index);

	return 0;
}


/*df_type::dfc_id_indx_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::dfc_id_indx_type::clean()
{
	id = 0u;
	index = 0u;
}


/*df_type::dfr_id_indx_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::dfr_id_indx_type::clean()
{
	id = 0u;
	index = 0u;
}


/*df_type::mesh_indx_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::mesh_indx_type::clean()
{
	dfc_id = nullptr;
	mesh_index = 0u;
}


bool df_type::mesh_indx_type::operator==(const mesh_indx_type& operand)
{
	if ((this->dfc_id == operand.dfc_id) && (this->mesh_index == operand.mesh_index))
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*df_type::mesh_layer_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*df_type::grid_type*/
/*-------------------------------------------------------------------------------------------------------------*/


df_type::grid_type::grid_type()
{
	if (this->do_once == 0)
	{
		this->do_once = 1;
	}
}



/*	Returns amount of layers in specified grid point	*/
void df_type::grid_type::calc_layer_size()
{
	layers_size = layers.size();
}


/*	Returns max value of all layers in the current grid point	*/
float df_type::grid_type::get_max_value()
{
	calc_layer_size();
	if (layers_size > 0)
	{
		float max_value = .0f;
		for (unsigned short a = 0; a < layers_size; ++a)
		{
			if (layers[a]->value > max_value)
			{
				max_value = layers[a]->value;
			}
		}

		return max_value;
	}
	else
	{
		return .0f;
	}
}


/*	Returns max value of only layers within the current grid point belonging to the specified dfc(s) */
float df_type::grid_type::get_max_value(const std::vector<unsigned long>& dfc_ids)
{
	calc_layer_size();
	if (layers_size > 0)
	{
		float max_value = .0f;
		for (unsigned short a = 0; a < layers_size; ++a)
		{
			unsigned long dfc_ids_nxt_indx = dfc_ids.size();
			for (unsigned long b = 0; b < dfc_ids_nxt_indx; ++b)
			{
				if (*(layers[a]->mesh_indx.dfc_id) == dfc_ids[b])
				{
					goto compare;
				}
			}
			continue;

		compare:

			if (layers[a]->value > max_value)
			{
				max_value = layers[a]->value;
			}
		}

		return max_value;
	}
	else
	{
		return .0f;
	}
}


/*	Returns the value of the layer belonging to the specified mesh indx, if none exist, returns -1 */
float df_type::grid_type::get_mesh_value(const mesh_indx_type& mesh_indx)
{
	/*	The below for loop iterates through all layers in the current grid point	*/
	calc_layer_size();
	for (unsigned short a = 0; a < layers_size; ++a)
	{
		/*	Tests if the mesh index of the current layers matches the specified mesh index	*/
		if (layers[a]->mesh_indx == mesh_indx)
		{
			last_layer_indx_cnvrsn.mesh_indx = mesh_indx;
			last_layer_indx_cnvrsn.layer_index = a;

			/*	If so, returns value belonging to current layer	*/
			return layers[a]->value;
		}
	}

	return -1.0f;
}


/*	Sets the value in the layer belonging to the specified mesh, if no such layer exists, returns 1	*/
int df_type::grid_type::set_mesh_value(const float& value, const mesh_indx_type& mesh_indx)
{
	/*	Checks if mesh layer exists in cache	*/
	unsigned short layer_indx = 0u;
	if (last_layer_indx_cnvrsn.mesh_indx == mesh_indx)
	{
		/*	If hit, use cache	*/
		layer_indx = last_layer_indx_cnvrsn.layer_index;
	}
	else
	{
		/*	If miss, find layer	manually	*/

		/*	The below for loop iterates through all layers in the current grid point	*/
		calc_layer_size();
		for (unsigned short a = 0; a < layers_size; ++a)
		{
			if (layers[a]->mesh_indx == mesh_indx)
			{
				last_layer_indx_cnvrsn.mesh_indx = mesh_indx;
				last_layer_indx_cnvrsn.layer_index = a;
				layer_indx = a;
				goto indx_found;
			}
		}

		return 1;
	}
indx_found:

	/*	Sets value of layer belonging to the specified mesh to equal the specified value	*/
	layers[layer_indx]->value = value;
	return 0;
}


/*	Removes layer and rearranges vector, returns information regarding what was moved (if a layer was moved) so a handling function
	can update state accordingly. Returns object will have return code 0 if removal was successful and a layer was moved, will return
	1 if removal was sucessfull but a layer wasn't moved (this would occur if the specified layer to be removed was the end layer
	in the vector), will return -1 if the specified layer index is out of bounds	*/
df_type::grid_type::remove_layer_return_type  df_type::grid_type::remove_layer(const unsigned short& layer_index)
{
	remove_layer_return_type return_obj;

	/*	If the current grid point has a token, locks it	*/
	if (token != nullptr)
	{
		token->lock();
	}

	/*	Deletes specified layer after performing bounds and nullptr checks	*/
	calc_layer_size();
	if (layer_index < layers_size)
	{
		if (layers[layer_index] != nullptr)
		{
			delete layers[layer_index];
		}
	}
	else
	{
		return_obj.return_code = -1;
		return return_obj;
	}

	/*	Simulates the "remove_layer" function within the inverse jenga class (I think I authored this before the inverse jenga
		class was made)	*/
	unsigned short layers_last_index = layers.size() - 1u;
	/*	Checks if the specified layer was the the end layer	*/
	if (layer_index == layers_last_index)
	{
		/*	If so, no other layers need to be moved	*/
		/*	Checks if the specified layer was the only layer in the vector	*/
		if (layers_last_index == 0)
		{
			/*	If so destroys and replaces vector (to clean dynamically allocated memory as vector does not (at least in most
				cases I'm aware of) implicitly reduce capacity, nor does it have a forced method to explicitly reduce it)	*/
			std::vector<mesh_layer_type*>().swap(layers);
		}
		else
		{
			/*	If there are other layers in the vector, simply pop back	*/
			layers.pop_back();
		}

		/*	Unlocks token if it exists	*/
		if (token != nullptr)
		{
			token->unlock();
		}

		return_obj.return_code = 1;
		return return_obj;
	}
	else
	{
		/*	If the specified layer was not at the end, moves the layer that is at the end to the spot previously occupied
			by the specified layer	*/

		layers[layer_index] = layers[layers_last_index];
		layers.pop_back();

		return_obj.mesh_indx = &layers[layer_index]->mesh_indx;
		return_obj.mesh_layer_table_index = &layers[layer_index]->mesh_layer_table_index;
		return_obj.return_code = 0;

		/*Updates index stored in last_layer_indx_cnvrsn if the moved layer was within it*/
		if (layers[layer_index]->mesh_indx == last_layer_indx_cnvrsn.mesh_indx)
		{
			last_layer_indx_cnvrsn.layer_index = layer_index;
		}

		/*	Unlocks token if it exists	*/
		if (token != nullptr)
		{
			token->unlock();
		}

		return return_obj;
	}
}


void df_type::grid_type::clean()
{
	/*Deletes all layers and clears layer vector*/
	calc_layer_size();
	for (unsigned short a = 0; a < layers_size; ++a)
	{
		if (layers[a] != nullptr)
		{
			delete layers[a];
		}
	}
	std::vector<mesh_layer_type*>().swap(layers);

	if (token != nullptr)
	{
		delete token;
	}

	layers_size = 0u;
	index.clean();
	token = nullptr;
}


/*df_type::grid_type::last_layer_indx_convrsn_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*df_type::grid_type::remove_layer_return_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*df_type::grid_cell_value_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::grid_cell_value_type::clean()
{
	mmm = 0;
	mmx = 0;
	mxm = 0;
	mxx = 0;
	xmm = 0;
	xmx = 0;
	xxm = 0;
	xxx = 0;
}


/*df_type::cmprt_meta_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::cmprt_meta_type::clean()
{
	size = 0;
	rlvncy_table_size = 0;
}


/*df_type::volume_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


df_type::volume_local_type::volume_local_type()
{

}


void df_type::volume_local_type::clean()
{
	if (grid != nullptr)
	{
		/*Deletes grid*/
	
		for (unsigned short a = 0u; a < grid_amount.x; ++a)
		{
			for (unsigned short b = 0u; b < grid_amount.y; ++b)
			{
				for (unsigned short c = 0u; c < grid_amount.z; ++c)
				{
					grid[a][b][c].clean();
				}
				delete[] grid[a][b];
			}
			delete[] grid[a];
		}
		delete[] grid;
	}

	/*Deletes compartment lookup table*/
	if (cmprt_table != nullptr)
	{
		for (unsigned long a = 0; a < cmprt_amount_total; ++a)
		{
			if (cmprt_table[a] != nullptr)
			{
				delete[] cmprt_table[a];
			}
		}
		delete[] cmprt_table;
	}

	/*Deletes compartment meta info*/
	if (cmprt_meta != nullptr)
	{
		delete[] cmprt_meta;
	}

	/*Deletes compartment relevency lookup table*/
	if (cmprt_rlvncy_table != nullptr)
	{
		for (unsigned long a = 0; a < cmprt_amount_total; ++a)
		{
			if (cmprt_rlvncy_table[a] != nullptr)
			{
				delete[] cmprt_rlvncy_table[a];
			}
		}
		delete[] cmprt_rlvncy_table;
	}

	grid = nullptr;
	cmprt_table = nullptr;
	cmprt_meta = nullptr;
	cmprt_rlvncy_table = nullptr;

	volume_verts_max.clean();

	grid_amount.clean();
	max_grid_point.clean();
	grid_scale.clean();

	cmprt_amount.clean();
	cmprt_amount_total = 0;

	intern_df_distance = .0f;
	intern_df_cmprt_size = 0;
	intern_grid_spacing = .0;
}


/*	Alternate clean function (see description for "df_type::clean_special" for more info on why these alternate functions exist)	*/
void df_type::volume_local_type::clean_special()
{
	for (unsigned short a = 0; a < 8; ++a)
	{
		intern_volume_verts[a].clean();
	}
}


/*	Writes volume local state to buffer (as of current, this is only called when weak stashing)	*/
void df_type::volume_local_type::write_to_buffer(buffer_type& buffer)
{
	/*	Buffers misc objects	*/
	buffer.grid_is_valid = grid != nullptr;
	buffer.df_distance = intern_df_distance;
	buffer.cmprt_size = intern_df_cmprt_size;
	buffer.grid_spacing = intern_grid_spacing;


	/*	Checks that the grid is valid (everything buffered below will only be buffered if the grid is valid), the only objects that
		are buffered even if the grid is invalid are the objects above	*/
	if (grid != nullptr)
	{
		/*	Buffer misc objects	*/
		buffer.grid_min_coord = min_grid_coord;
		buffer.max_grid_point = max_grid_point;
		buffer.grid_point_amount = grid_amount;

		/*	Buffers "grid_point_meta" and "grid_point_indices", only grid points that contain layers have their data added to the buffer,
			one of the purposes of the below for loop is to determine which grid points have layers	*/
		unsigned long grid_index = 0u;
		unsigned long non_empty_index = 0u;
		std::vector<shared_type::index_xyz_type> indices_nonlinear;
		for (unsigned short a = 0u; a < grid_amount.x; ++a)
		{
			for (unsigned short b = 0u; b < grid_amount.y; ++b)
			{
				for (unsigned short c = 0u; c < grid_amount.z; ++c)
				{
					if (grid[a][b][c].layers.size() > 0)
					{
						buffer.grid_points.grid_point_meta.push_back(grid[a][b][c].layers.size());
						buffer.grid_points.grid_point_indices.push_back(grid_index);
						shared_type::index_xyz_type nonlinear_index;
						nonlinear_index.x = a;
						nonlinear_index.y = b;
						nonlinear_index.z = c;
						indices_nonlinear.push_back(nonlinear_index);
						++non_empty_index;
					}
					++grid_index;
				}
			}
		}

		/*	Buffers the "value", "mesh_indx", and "mesh_layer_table_indx" for each layer in the grid points, only looping through grid
		points that actually have layers (using the data gleaned in the above for loop)	*/
		buffer.grid_points.grid_point_values = new df_type::mesh_layer_type * [non_empty_index];
		for (unsigned long a = 0u; a < non_empty_index; ++a)
		{
			buffer.grid_points.grid_point_values[a] = new df_type::mesh_layer_type[buffer.grid_points.grid_point_meta[a]];
			for (unsigned long b = 0u; b < buffer.grid_points.grid_point_meta[a]; ++b)
			{
				buffer.grid_points.grid_point_values[a][b].value = grid[indices_nonlinear[a].x][indices_nonlinear[a].y][indices_nonlinear[a].z].layers[b]->value;
				buffer.grid_points.grid_point_values[a][b].mesh_indx = grid[indices_nonlinear[a].x][indices_nonlinear[a].y][indices_nonlinear[a].z].layers[b]->mesh_indx;
				buffer.grid_points.grid_point_values[a][b].mesh_layer_table_index = grid[indices_nonlinear[a].x][indices_nonlinear[a].y][indices_nonlinear[a].z].layers[b]->mesh_layer_table_index;
			}
		}

		/*	Buffers cmprt data	*/
		buffer.cmprt_data.cmprt_amount = cmprt_amount;
		buffer.cmprt_data.cmprts = new cmprt_data_type::cmprt_type[cmprt_amount_total];
		for (unsigned long a = 0u; a < cmprt_amount_total; ++a)
		{
			buffer.cmprt_data.cmprts[a].grid_points = new unsigned long[cmprt_meta[a].size];
			buffer.cmprt_data.cmprts[a].size = cmprt_meta[a].size;
			for (unsigned long b = 0u; b < cmprt_meta[a].size; ++b)
			{
				unsigned long linear_index = (cmprt_table[a][b]->index.z + (grid_amount.z * cmprt_table[a][b]->index.y)) + ((grid_amount.z * grid_amount.y) * cmprt_table[a][b]->index.x);
				buffer.cmprt_data.cmprts[a].grid_points[b] = linear_index;

			}

			buffer.cmprt_data.cmprts[a].rlvncy_table_size = cmprt_meta[a].rlvncy_table_size;
			buffer.cmprt_data.cmprts[a].rlvncy_table = cmprt_rlvncy_table[a];
			cmprt_rlvncy_table[a] = nullptr;
		}

		/*	Buffers volume verts (afaik this is kind of pointless as "intern_volume_verts" is not cleaned when weak stashing so that
			frequent volume checks don't causes constant weak unstashing and weak stashing)	*/
		for (unsigned short a = 0u; a < 8u; ++a)
		{
			buffer.volume_verts[a] = intern_volume_verts[a];
		}
	}

	weak_stashed = true;
}


/*	Loads volume local state from buffer (currently this function would be called when loading a dfcache file as well as when unstashing
	and weak unstashing)	*/
void df_type::volume_local_type::load_from_buffer(const buffer_type& buffer, const shared_type::invrse_jenga_type<df_type::dfc_id_indx_type*, unsigned long> dfc_ids)
{
	/*	Unbuffer misc objects	*/
	intern_df_distance = buffer.df_distance;
	intern_df_cmprt_size = buffer.cmprt_size;
	intern_grid_spacing = buffer.grid_spacing;
	min_grid_coord = buffer.grid_min_coord;

	/*	Check if grid is valid not that the flag has been buffered (everything unbuffered in the below compound statement is only
		unbuffered if the grid is valid, as it is assumed that none of it was buffered if the grid is marked as invalid)	*/
	if (buffer.grid_is_valid == true)
	{
		/*	Unbuffers grid	*/
		{
			/*	Unbuffers misc objects	*/
			grid_amount = buffer.grid_point_amount;
			max_grid_point = buffer.max_grid_point;

			unsigned long grid_next_index = 0u;
			unsigned long buffer_next_index = 0u;
			unsigned long grid_point_indices_size = buffer.grid_points.grid_point_indices.size();
			/*	The below for loops reconstruct the grid, adding new columns, rows, etc as they go	*/
			grid = new grid_type * *[grid_amount.x];
			for (unsigned short a = 0u; a < grid_amount.x; ++a)
			{
				grid[a] = new grid_type * [grid_amount.y];
				for (unsigned short b = 0u; b < grid_amount.y; ++b)
				{
					grid[a][b] = new grid_type[grid_amount.z];
					for (unsigned short c = 0u; c < grid_amount.z; ++c)
					{
						/*	Checks if the index counter has exceeded the number of grid points listed in "grid_point_indices"
							(remember that this array represents the number of grid points that contain layers)	*/
						if (buffer_next_index < grid_point_indices_size)
						{
							/*	Checks if the current grid point is the next grid point listed in "grid_point_indices"
								(remember that we only process the grid points that have layers, and as mentioned above, the
								only ones that have layers are those listed within "grid_point_indices")	*/
							if (buffer.grid_points.grid_point_indices[buffer_next_index] == grid_next_index)
							{
								/*	If so, unbuffers the current grid points respective data	*/
								grid[a][b][c].layers_size = buffer.grid_points.grid_point_meta[buffer_next_index];
								for (unsigned long d = 0u; d < grid[a][b][c].layers_size; ++d)
								{
									mesh_layer_type* mesh_layer = new mesh_layer_type;
									mesh_layer->value = buffer.grid_points.grid_point_values[buffer_next_index][d].value;
									{
										unsigned long dfc_amount = dfc_ids.vector.size();
										for (unsigned long e = 0u; e < dfc_amount; ++e)
										{
											if (&dfc_ids.vector[e]->id == buffer.grid_points.grid_point_values[buffer_next_index][d].mesh_indx.dfc_id)
											{
												mesh_layer->mesh_indx.dfc_id = &dfc_ids.vector[e]->id;
												break;
											}
										}
									}
									mesh_layer->mesh_indx.mesh_index = buffer.grid_points.grid_point_values[buffer_next_index][d].mesh_indx.mesh_index;
									mesh_layer->mesh_layer_table_index = buffer.grid_points.grid_point_values[buffer_next_index][d].mesh_layer_table_index;
									grid[a][b][c].layers.push_back(mesh_layer);
								}
								++buffer_next_index;
							}
						}

						/*	Updates the current grid point's copy of it's own index	*/
						grid[a][b][c].index.x = a;
						grid[a][b][c].index.y = b;
						grid[a][b][c].index.z = c;

						++grid_next_index;
					}
				}
			}
		}


		/*	Unbuffers cmprt data	*/
		{
			cmprt_amount = buffer.cmprt_data.cmprt_amount;
			cmprt_amount_total = cmprt_amount.x * cmprt_amount.y * cmprt_amount.z;
			cmprt_table = new grid_type * *[cmprt_amount_total];
			cmprt_meta = new cmprt_meta_type[cmprt_amount_total];
			cmprt_rlvncy_table = new unsigned long* [cmprt_amount_total];
			for (unsigned long a = 0u; a < cmprt_amount_total; ++a)
			{
				cmprt_meta[a].size = buffer.cmprt_data.cmprts[a].size;
				cmprt_meta[a].rlvncy_table_size = buffer.cmprt_data.cmprts[a].rlvncy_table_size;

				cmprt_table[a] = new grid_type * [cmprt_meta[a].size];
				for (unsigned long b = 0u; b < cmprt_meta[a].size; ++b)
				{
					unsigned long linear_index = buffer.cmprt_data.cmprts[a].grid_points[b];
					shared_type::index_xyz_type xyz_index;
					unsigned long grid_points_xy_amount = grid_amount.z * grid_amount.y;
					xyz_index.x = linear_index / grid_points_xy_amount;
					unsigned long grid_points_xy_amount_mx = grid_points_xy_amount * xyz_index.x;
					xyz_index.y = (linear_index - grid_points_xy_amount_mx) / grid_amount.z;
					xyz_index.z = (linear_index - grid_points_xy_amount_mx) - (grid_amount.z * xyz_index.y);

					cmprt_table[a][b] = &grid[xyz_index.x][xyz_index.y][xyz_index.z];
				}

				cmprt_rlvncy_table[a] = buffer.cmprt_data.cmprts[a].rlvncy_table;
				buffer.cmprt_data.cmprts[a].rlvncy_table = nullptr;
				/*cmprt_rlvncy_table[a] = new unsigned long[cmprt_meta[a].rlvncy_table_size];
				for (unsigned long b = 0u; b < cmprt_meta[a].rlvncy_table_size; ++b)
				{
					cmprt_rlvncy_table[a][b] = buffer.cmprt_data.cmprts[a].rlvncy_table[b];
				}*/
			}
		}
	}

	/*	Unbuffers volume verts	*/
	for (unsigned short a = 0u; a < 8u; ++a)
	{
		intern_volume_verts[a] = buffer.volume_verts[a];
	}

	weak_stashed = false;
}


void df_type::volume_local_type::clean_internal_buffer()
{
	buffer.deep_clean();
}


/*df_type::volume_local_type::grid_points_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::volume_local_type::grid_points_type::clean()
{
	/*	Both grid_point_meta or grid_point_indices would have worked here (they have the same size)	*/
	unsigned long size = grid_point_indices.size();
	if (grid_point_values != nullptr)
	{
		for (unsigned long a = 0u; a < size; ++a)
		{
			delete[] grid_point_values[a];
		}
		delete[] grid_point_values;
		grid_point_values = nullptr;
	}

	std::vector<unsigned long>().swap(grid_point_meta);
	std::vector<unsigned long>().swap(grid_point_indices);
}


df_type::volume_local_type::grid_points_type::~grid_points_type()
{
	clean();
}


/*df_type::volume_local_type::cmprt_data_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::volume_local_type::cmprt_data_type::clean()
{
	if (cmprts != nullptr)
	{
		delete[] cmprts;
		cmprts = nullptr;
	}
	cmprt_amount.clean();
}


df_type::volume_local_type::cmprt_data_type::~cmprt_data_type()
{
	clean();
}


/*df_type::volume_local_type::cmprt_data_type::cmprt_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::volume_local_type::cmprt_data_type::cmprt_type::clean()
{
	if (grid_points != nullptr)
	{
		delete[] grid_points;
		grid_points = nullptr;
	}
	if (rlvncy_table != nullptr)
	{
		delete[] rlvncy_table;
		rlvncy_table = nullptr;
	}
	size = 0u;
	rlvncy_table_size = 0u;
}

df_type::volume_local_type::cmprt_data_type::cmprt_type::~cmprt_type()
{
	clean();
}


/*df_type::volume_local_type::buffer_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::volume_local_type::buffer_type::clean()
{
	grid_is_valid = false;
	grid_min_coord.clean();
	grid_spacing = .0;
	df_distance = .0f;
	cmprt_size = 0u;

	for (unsigned short a = 0u; a < 8u; ++a)
	{
		volume_verts[a].clean();
	}

	grid_point_amount.clean();
	max_grid_point.clean();
}


void df_type::volume_local_type::buffer_type::deep_clean()
{
	grid_points.clean();
	cmprt_data.clean();
	clean();
}


df_type::volume_local_type::buffer_type::~buffer_type()
{
	clean();
}


/*df_type::volume_local_type::delete_rlvncy_buffers_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*df_type::volume_local_type::delete_rlvncy_buffers_shared_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*df_type::update_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::update_local_type::clean()
{
	dfc_index = 0;
	batches_sent = 0;
	batches_completed = 0;
}


void df_type::update_local_type::deep_clean()
{
	dfc_cache.clean();
	//dfc_cache.clean_dfc_ids();
	//dfr_cache.clean_dfr_ids();
	clean();
}


/*	Loads update local state from buffer	*/
void df_type::update_local_type::load_from_buffer(buffer_type& buffer)
{
	dfc_cache.is_valid = buffer.dfc_cache_is_valid;

	/*	Checks if the dfc cache is value (the objects unbuffered in the below compound statement are only unbuffered if the
		dfc cache is valid, as it is assumed that these objects were not buffered if the dfc cache is marked as invalid)	*/
	if (dfc_cache.is_valid == true)
	{
		dfc_cache.vert_amount_total = buffer.vert_amount_total;
		dfc_cache.vert_cache_next_index = buffer.vert_cache_next_index;
		dfc_cache.vert_cache = buffer.vert_cache;
		buffer.vert_cache = nullptr;
		dfc_cache.tri_cache_size = buffer.tri_cache_size;
		buffer.tri_cache_size = nullptr;
		dfc_cache.tri_cache = buffer.tri_cache;
		buffer.tri_cache = nullptr;
		dfc_cache.mesh_cache = buffer.mesh_cache;
		buffer.mesh_cache = nullptr;
		dfc_cache.existing_dfcs = buffer.existing_dfcs;
		buffer.existing_dfcs = nullptr;
	}

	dfc_cache.dfc_indx_to_id_table = buffer.dfc_id_index_table;
	buffer.dfc_id_index_table = nullptr;
	dfc_cache.dfc_amount = buffer.dfc_amount;
	dfc_cache.dfc_ids = buffer.dfc_ids;
	buffer.dfc_ids.clean();
	dfr_cache.dfr_ids = buffer.dfr_ids;
	buffer.dfr_ids.clean();
}


/*df_type::update_local_type::dfc_cache_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Checks for triangles adjacent to the specified vertex (see "df_type::update_local::dfc_cache::add_dfc_to_dfc_cache" for more info)	*/
/*__forceinline*/ void df_type::update_local_type::dfc_cache_type::check_for_uniq_adjcnt_tris(const unsigned long& vert_index, unsigned long& base, unsigned long& top, unsigned long& top_last_base_incrmt, std::vector<unsigned long>& calced_verts, mesh_info_type* mesh, const unsigned long& dfc_index)
{
	/*	Adds specified vert to list of already processed/ calced verts	*/
	calced_verts.push_back(vert_index);
	
	/*	The below for loop iterates through each tri that reference the specified vert	*/
	for (unsigned short a = 1; a < vert_cache[vert_index].tris[0]; ++a)
	{
		/*	The below for loop iterates through each tri within the current mesh	*/
		unsigned long dfc_mesh_cache_x_size = mesh->tris.size();
		for (unsigned long b = 0; b < dfc_mesh_cache_x_size; ++b)
		{
			/*	If the current tri already exists within the current mesh, skip tri	*/
			if (vert_cache[vert_index].tris[a] == mesh->tris[b])
			{
				goto skip;
			}
		}

		/*	Adds current tri to current mesh and increments "top"	*/
		mesh->tris.push_back(vert_cache[vert_index].tris[a]);
		top += 1;

	skip:
		continue;
	}
}


/*	When a layer within a grid point is removed, another layer may be moved to the index occupied by the removed layer. If a layer was
	moved, this function updates data structures that referenced that layer to take into account its new index */
void df_type::update_local_type::dfc_cache_type::remove_layers_return_handler(const grid_type::remove_layer_return_type& return_obj, const unsigned short& remove_layers_indx_param)
{
	/*	Checks if return code == 0 (a return code of 0 indicates that a layer was moved)	*/
	if (return_obj.return_code == 0)
	{
		/*	Gets index of the dfc id (the dfc id the layer belonged to) in the legacy list of dfc ids	*/
		long long moved_layer_dfc_indx = dfc_id_to_indx(*return_obj.mesh_indx->dfc_id, 2);
		if (moved_layer_dfc_indx >= 0)
		{
			/*	Bound checks index against size of legacy list of dfc ids	*/
			if (return_obj.mesh_indx->mesh_index < mesh_cache_legacy[moved_layer_dfc_indx]->size())
			{
				/*	If the current mesh has already been moved from legacy, update current dfc cache, otherwise update legacy cache	*/
				if (mesh_cache_legacy[moved_layer_dfc_indx]->at(return_obj.mesh_indx->mesh_index)->moved_from_legacy == true)
				{
					goto update_current_cache;
				}
				else
				{
					goto update_legacy_cache;
				}
			}
			else
			{
				/*	If index is out of bounds of legacy list of dfc ids, the current dfc cache is updated	*/
				goto update_current_cache;
			}
		}
		else
		{
			/*	If the dfc id cannot be found in the list of legacy dfc ids, or the legacy cache is invalid, update current cache	*/
			goto update_current_cache;
		}


	update_current_cache:

		/*	Gets index of the dfc id (the dfc id the layer belonged to) in the current list of dfc ids	*/
		moved_layer_dfc_indx = dfc_id_to_indx(*return_obj.mesh_indx->dfc_id, 1);
		if (moved_layer_dfc_indx >= 0)
		{
			/*	If the dfc id exists in the current cache, updates current mesh cache accordingly	*/
			mesh_cache[moved_layer_dfc_indx]->at(return_obj.mesh_indx->mesh_index)->layer_indices->at(*return_obj.mesh_layer_table_index).w = remove_layers_indx_param;
		}
		return;

	update_legacy_cache:

		/*	Updates legacy mesh cache accordingly	*/
		mesh_cache_legacy[moved_layer_dfc_indx]->at(return_obj.mesh_indx->mesh_index)->layer_indices->at(*return_obj.mesh_layer_table_index).w = remove_layers_indx_param;
	}
}


/*	Removes all mesh layers in the grid belonging to the specidied mesh	*/
int df_type::update_local_type::dfc_cache_type::remove_mesh_layers(const unsigned long& dfc_id, const unsigned long& mesh_indx, const bool& legacy, const volume_local_type& volume_local)
{
	mesh_info_type* current_mesh = nullptr;

	if (legacy == true)
	{
		unsigned long dfc_index = 0;
		/*	Gets index of specified dfc id in legacy list of dfc ids	*/
		long long return_index = dfc_id_to_indx(dfc_id, 2);
		if (return_index >= 0)
		{
			dfc_index = (unsigned long)return_index;
		}
		else
		{
			/*	If specified dfc id cannot be found within the legacy list of dfc ids, returns 1	*/
			return 1;
		}

		current_mesh = mesh_cache_legacy[dfc_index]->at(mesh_indx);
	}
	else
	{
		unsigned long dfc_index = 0;
		/*	Gets index of specified dfc id in current list of dfc ids	*/
		long long return_index = dfc_id_to_indx(dfc_id, 1);
		if (return_index >= 0)
		{
			dfc_index = (unsigned long)return_index;
		}
		else
		{
			/*	If specified dfc id cannot be found within the current list of dfc ids, returns 1	*/
			return 1;
		}

		current_mesh = mesh_cache[dfc_index]->at(mesh_indx);
	}

	/*	Gets amount of mesh layers belonging to the specified mesh	*/
	unsigned long mesh_cache_x_size = current_mesh->layer_indices->size();
	/*	The below for loop iterates through each mesh layer belonging to the specified mesh	*/
	for (unsigned long a = 0; a < mesh_cache_x_size; ++a)
	{
		/*	Calls "df_type::grid_type::remove_layer", whose return value is fed directly into the argument list of a call to
			"df_type::update_local_type::dfc_cache_type::remove_layers_return_handler". See descriptions of both functions
			for more info	*/
		remove_layers_return_handler(volume_local.grid[current_mesh->layer_indices->at(a).x][current_mesh->layer_indices->at(a).y][current_mesh->layer_indices->at(a).z].remove_layer(current_mesh->layer_indices->at(a).w), current_mesh->layer_indices->at(a).w);
	}

	return 0;
}


/*	Converts dfc id to the index of the dfc ids "df_type::dfc_id_indx_type" object within the list of dfc ids. The 2nd parameter
	is the legacy mode, a value of 0 searches in both the legacy and current caches, a value of 1 searches in the current cache only,
	and a value of 2 searches in the legacy cache only. Returns index if successfull, returns -1 if cannot find dfc id, and returns -2
	if the 2nd argument passed is invalid	*/
long long df_type::update_local_type::dfc_cache_type::dfc_id_to_indx(const unsigned long& dfc_id, const int& legacy_setting)
{
	if ((legacy_setting == 2) || (legacy_setting == 0))
	{
		/*	Searches in legacy cache	*/
		if (legacy_is_valid == true)
		{
			for (unsigned long a = 0; a < dfc_amount_legacy; ++a)
			{
				if (dfc_indx_to_id_table_legacy[a] != nullptr)
				{
					if (dfc_id == dfc_indx_to_id_table_legacy[a]->id)
					{
						return a;
					}
				}
			}
		}

		/*	Jumps to searching in current cache if "legacy_mode" is 0 */
		if (legacy_setting == 0)
		{
			goto check_current_cache;
		}
	}
	else if (legacy_setting == 1)
	{
	check_current_cache:

		/*	Searches in current cache	*/
		if (is_valid == true)
		{
			for (unsigned long a = 0; a < dfc_amount; ++a)
			{
				if (dfc_indx_to_id_table[a] != nullptr)
				{
					if (dfc_id == dfc_indx_to_id_table[a]->id)
					{
						return a;
					}
				}
			}
		}
	}
	else
	{
		return -2;
	}

	return -1;
}

/*	Returns a pointer to the actual "dfc_id_index_type" object of the speficied dfc id, returns	nullptr if canot be found	*/
df_type::dfc_id_indx_type* df_type::update_local_type::dfc_cache_type::get_dfc_id(const unsigned long& dfc_id)
{
	dfc_ids.calc_size();
	for (unsigned long a = 0; a < dfc_ids.size; ++a)
	{
		if (dfc_id == dfc_ids.vector[a]->id)
		{
			return dfc_ids.vector[a];
		}
	}

	return nullptr;
}


/*	Initializes and dynamically allocates dfc cache objects (this must be called before any calls are made to
	"df_type::update_local_type::dfc_cache_type::add_dfc_to_cache")	*/
void df_type::update_local_type::dfc_cache_type::initialize_cache(const unsigned long* obj_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount)
{
	vert_cache = new shared_type::vert_tri_ref_type[vert_amount];
	vert_amount_total = vert_amount;
	tri_cache = new shared_type::tri_info_type * [dfc_amount];
	tri_cache_size = new unsigned long[dfc_amount];
	mesh_cache = new std::vector<mesh_info_type*>*[dfc_amount];
	this->dfc_amount = dfc_amount;
	existing_dfcs = new bool[dfc_amount]();

	/*	Sets up "dfc_id_to_index_table" and creates new "dfc_id_indx_type" objects for new dfc(s)	*/
	dfc_indx_to_id_table = new dfc_id_indx_type * [dfc_amount];
	for (unsigned long a = 0; a < dfc_amount; ++a)
	{
		dfc_ids.calc_size();
		for (unsigned long b = 0; b < dfc_ids.size; ++b)
		{
			if (obj_ids[a] == dfc_ids.vector[b]->id)
			{
				dfc_indx_to_id_table[a] = dfc_ids.vector[b];
				goto skip;
			}
		}
		goto create_new_id;

	skip:
		continue;

	create_new_id:

		dfc_id_indx_type* id_to_add = new dfc_id_indx_type;
		id_to_add->id = obj_ids[a];
		id_to_add->index = dfc_ids.size;
		dfc_ids.vector.push_back(id_to_add);
		dfc_indx_to_id_table[a] = id_to_add;
	}
}


void df_type::update_local_type::dfc_cache_type::add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc)
{
	if (tri_amount != 0)
	{
		/*	Dynamically allocates array	at the current dfc's entry in the tri cache to store it's verts	*/
		tri_cache[dfc_index] = new shared_type::tri_info_type[tri_amount];
		/*	Sets the current dfc's entry in the list of tri cache sizes to equal the amount of triangles	*/
		tri_cache_size[dfc_index] = tri_amount;

		/*	Sets and alias for the index of the vert cache that will act as the beginning of the current dfcs
			region within the vert cache (the vertices for all dfc are stored in a single array)	*/
		unsigned long current_vert_cache_start = vert_cache_next_index;
		
		/*		*/
		for (unsigned long a = 0; a < tri_amount; ++a)
		{
			/*	Adds the current tris data to the cache	(this includes the indices of its 3 vertices)	*/
			tri_cache[dfc_index][a] = tris[a];

			/*	Offsets the vert indices to take into account to offset of the current dfc in the vert cache (remember
				that all dfc's vertices are stored in the same array)	*/
			tri_cache[dfc_index][a].vert_0 += vert_cache_next_index;
			tri_cache[dfc_index][a].vert_1 += vert_cache_next_index;
			tri_cache[dfc_index][a].vert_2 += vert_cache_next_index;
		}

		/*	The below for loop iterates through each vertex passed to the function, and both adds each to the vert cache	*/
		for (unsigned long a = 0; a < vert_amount; ++a)
		{
			/*	Adds the dfc index and coord of the current vert	*/
			vert_cache[vert_cache_next_index].dfc_index = dfc_index;
			vert_cache[vert_cache_next_index].coord = verts[a];

			/*	The below compound statement is only executed is split dfc is enabled	*/
			if (split_dfc == true)
			{
				/*	Dynamically allocated a buffer	*/
				unsigned long* tris_buffer = new unsigned long[tri_amount];
				unsigned long tris_buffer_next_index = 0;

				/*	The below for loop iterates through each triangle, and for each, checks if it has the current vertex as one
					of its vertices, if so, adds to the buffer	*/
				for (unsigned long b = 0; b < tri_amount; ++b)
				{
					if ((tris[b].vert_0 == a) || (tris[b].vert_1 == a) || (tris[b].vert_2 == a))
					{
						tris_buffer[tris_buffer_next_index] = b;
						tris_buffer_next_index += 1;
					}
				}

				/*	Checks if the current vert referenced by any triangles	*/
				if (tris_buffer_next_index != 0)
				{
					/*	If so, adds tri buffer to list of tris in the current verts entry in the vert buffer	*/

					unsigned long current_vert_tris_amount = tris_buffer_next_index + 1;
					vert_cache[vert_cache_next_index].tris = new unsigned long[current_vert_tris_amount];
					vert_cache[vert_cache_next_index].tris[0] = current_vert_tris_amount;
					for (unsigned long d = 1; d < current_vert_tris_amount; ++d)
					{
						vert_cache[vert_cache_next_index].tris[d] = tris_buffer[d - 1];
					}

				}

				delete[] tris_buffer;
			}

			vert_cache_next_index += 1;
		}

		/*	Dynamically allocates a vector to store to meshes contained within the current dfc at said dfc's entry in
			the mesh cache	*/
		mesh_cache[dfc_index] = new std::vector<mesh_info_type*>;		

		if (split_dfc == false)
		{	
			/*	If split dfc is DISABLED, dynamically allocates and adds only a single mesh to the vector, and adds the indices
				of each tri to said mesh	*/
			mesh_info_type* mesh_to_add = new mesh_info_type;
			mesh_to_add->tris.reserve(tri_cache_size[dfc_index]);
			mesh_cache[dfc_index]->push_back(mesh_to_add);

			for (unsigned long a = 0; a < tri_cache_size[dfc_index]; ++a)
			{
				mesh_to_add->tris.push_back(a);
			}

			mesh_cache[dfc_index]->at(0)->tris_size = tri_cache_size[dfc_index];
		}
		else
		{
		check_for_remaining_tris:

			/*	If split dfc is ENABLED, seperates the dfc into loose parts (with each loose part becomning a seperate mesh
				entry the vector allocated above)	*/

			/*	The y size of the mesh cache refers to the number of meshes in the current dfc's entry in the mesh cache (ie,
				the number of meshes in the vector allocated above). The x size of the mesh cache refers to the number of tris
				in each mesh. Each dfc entry in the mesh cache is though of as a 2D table, eg:
				
				--------------------------------------------
				| mesh_0 | tri_0, tri_1, tri_2, tri_3, ... |
				--------------------------------------------
				| mesh_1 | tri_0, tri_1, tri_2, tri_3, ... |
				--------------------------------------------
				| mesh_0 | tri_0, tri_1, tri_2, tri_3, ... |
				--------------------------------------------
				
				*/
			unsigned long mesh_cache_y_size = mesh_cache[dfc_index]->size();

			unsigned long strting_tri = 0;
			bool valid_strting_tri = false;
			/*	The body of the below for loop test wether or not there are any tris in the dfc's entry in the tri cache which are not
				already part of a mesh. If no starting tri can be found, then the mesh seperation process is completed.
				The first for loop iterates through each tri in the current dfc's entry in the tri cache.	*/
			for (unsigned long a = 0; a < tri_cache_size[dfc_index]; ++a)
			{
				bool is_in_mesh = false;
				/*	The below for loop iterates through each mesh in the current dfc's entry in the mesh cache	*/
				for (unsigned long b = 0; b < mesh_cache_y_size; ++b)
				{
					unsigned long mesh_cache_x_size = mesh_cache[dfc_index]->at(b)->tris.size();
					/*	The below for loop iterates through each tri in the current mesh	*/
					for (unsigned long c = 0; c < mesh_cache_x_size; ++c)
					{
						if (a == mesh_cache[dfc_index]->at(b)->tris.at(c))
						{
							is_in_mesh = true;
							goto break_from_mesh_search;
						}
					}
				}
			break_from_mesh_search:

				/*	If the current tri was not found in any of the meshes, it is used as the starting tri	*/
				if (is_in_mesh == false)
				{
					strting_tri = a;
					valid_strting_tri = true;
					break;
				}
			}

			if (valid_strting_tri == true)
			{
				/*	Dynamically allocates new mesh	*/
				mesh_info_type* mesh_to_add = new mesh_info_type;
				/*	Adds starting tri as first tri in the mesh	*/
				mesh_to_add->tris.push_back(strting_tri);
				/*	Adds mesh ot the current dfc's entry in the mesh cache	*/
				mesh_cache[dfc_index]->push_back(mesh_to_add);


				/*	The below chunk of code uses a stack in order to find each vertex connected to the starting tri. Triangles
					connected to the vertices of the starting tri are searched for, and added to the current mesh if found,
					the vertices of these triangles are added to the stack, and the proces repeates (searching for triangles
					connected to these newly found verts). Each vert is only processed once, and each triangle is only added once,
					no dups.
					The stack is explained further below (note that top_last_base_incrmt is abbreviated to tlbi in the below diagram):
					
					At the very start the top and tlbi = 1, and the base = 0

					 |					
					1|	top---- tlbi----		
					0|	base----

					The vertices from base to tlbi are then iterated through in a for loop. While this happens, the body of the loop
					is searching for the vertices of adjacent triangles, and any it finds are added to the stack, "top" is also at
					the top of the stack, and so top increases as the loop iterates (though "tlbi" and "base" remain fixed).
					
					 |
					4|	top----
					3|
					2|
					1|	tlbi----
					0|	base----	
				
					Once the for loop reaches "tlbi", the base is moved up to where "tlbi" is, and then "tlbi" is moved up to where
					"top" sits

					 |
					4|	top---- tlbi----
					3|
					2|
					1|	base----
					0|

					The process then continues until no more adjacent triangles can be found, and thus no more verts (ie, until top
					does not increase any further), as it is then assumed that the entire mesh has been traversed, and that any remaining
					triangles/verts must not be connected to the current mesh. When this occurs flow jumps back up to
					"check_for_remaning_tris" to see if any other meshes exist	*/

				std::vector<unsigned long> calced_verts;
				unsigned long base = 0;
				unsigned long top = 1;
				unsigned long top_last_base_incrmt = 1;

				/*	The below while loop does not exit until flow is manually redirected to "check_for_remaining_tris" (located above)	*/
				while (true)
				{
					/*	Checks if top has incremented (if not then its assumed the end of the mesh has been reached)	*/
					if (base != top)
					{
						/*	The below for loop iterates through each entry in the stack from "base" to ("top_last_base_incrmt" - 1)	*/
						for (unsigned short a = base; a < top_last_base_incrmt; ++a)
						{
							/*	The below chunk of code checks if the current vert in the stack has already been processed/ calced*/
							bool vert_0_already_calced = false;
							bool vert_1_already_calced = false;
							bool vert_2_already_calced = false;
							unsigned long calced_verts_size = calced_verts.size();
							for (unsigned long c = 0; c < calced_verts_size; ++c)
							{
								if (tri_cache[dfc_index][mesh_to_add->tris[a]].vert_0 == calced_verts[c])
								{
									vert_0_already_calced = true;
								}
								if (tri_cache[dfc_index][mesh_to_add->tris[a]].vert_1 == calced_verts[c])
								{
									vert_1_already_calced = true;
								}
								if (tri_cache[dfc_index][mesh_to_add->tris[a]].vert_2 == calced_verts[c])
								{
									vert_2_already_calced = true;
								}
							}

							/*	Checks for adjacent triangles	*/
							if (vert_0_already_calced == false)
							{
								check_for_uniq_adjcnt_tris(tri_cache[dfc_index][mesh_to_add->tris[a]].vert_0, base, top, top_last_base_incrmt, calced_verts, mesh_to_add, dfc_index);
							}
							if (vert_1_already_calced == false)
							{
								check_for_uniq_adjcnt_tris(tri_cache[dfc_index][mesh_to_add->tris[a]].vert_1, base, top, top_last_base_incrmt, calced_verts, mesh_to_add, dfc_index);
							}
							if (vert_2_already_calced == false)
							{
								check_for_uniq_adjcnt_tris(tri_cache[dfc_index][mesh_to_add->tris[a]].vert_2, base, top, top_last_base_incrmt, calced_verts, mesh_to_add, dfc_index);
							}
						}
					}
					else
					{
						/*	If end of mesh, update tris size counter in mesh and jump to the appropirate label	*/
						mesh_cache[dfc_index]->at(mesh_cache_y_size)->calc_tris_size();
						goto check_for_remaining_tris;
					}

					base = top_last_base_incrmt;
					top_last_base_incrmt = top;
				}
			}
		}

		/*	Mark "dfc_cache" as valid	*/
		is_valid = true;
	}
}


/*	Moves contents of dfc cache to legacy cache	*/
int df_type::update_local_type::dfc_cache_type::move_to_legacy()
{
	if (is_valid == true)
	{
		vert_cache_legacy = vert_cache;
		vert_cache = nullptr;
		vert_cache_next_index_legacy = vert_cache_next_index;
		vert_cache_next_index = 0;
		vert_amount_total_legacy = vert_amount_total;
		vert_amount_total = 0;
		tri_cache_legacy = tri_cache;
		tri_cache = nullptr;
		tri_cache_size_legacy = tri_cache_size;
		tri_cache_size = nullptr;
		mesh_cache_legacy = mesh_cache;
		mesh_cache = nullptr;
		dfc_amount_legacy = dfc_amount;
		dfc_amount = 0;
		delete[] existing_dfcs;
		existing_dfcs = nullptr;
		dfc_indx_to_id_table_legacy = dfc_indx_to_id_table;
		dfc_indx_to_id_table = nullptr;

		legacy_is_valid = true;

		return 0;
	}
	else
	{
		return 1;
	}
}


void df_type::update_local_type::dfc_cache_type::clean_legacy()
{
	if (tri_cache_legacy != nullptr)
	{
		for (unsigned long a = 0; a < dfc_amount_legacy; ++a)
		{
			if (tri_cache_legacy[a] != nullptr)
			{
				delete[] tri_cache_legacy[a];
			}
		}
		delete[] tri_cache_legacy;
	}

	if (tri_cache_size_legacy != nullptr)
	{
		delete[] tri_cache_size_legacy;
	}

	if (vert_cache_legacy != nullptr)
	{
		for (unsigned long a = 0; a < vert_amount_total_legacy; ++a)
		{
			if (vert_cache_legacy[a].tris != nullptr)
			{
				delete[] vert_cache_legacy[a].tris;
			}
		}
		delete[] vert_cache_legacy;
	}

	if (mesh_cache_legacy != nullptr)
	{
		for (unsigned long a = 0; a < dfc_amount_legacy; ++a)
		{
			if (mesh_cache_legacy[a] != nullptr)
			{
				unsigned long mesh_cache_legacy_y_size = mesh_cache_legacy[a]->size();
				for (unsigned long b = 0; b < mesh_cache_legacy_y_size; ++b)
				{
					if (mesh_cache_legacy[a]->at(b) != nullptr)
					{
						delete mesh_cache_legacy[a]->at(b);
					}
				}
				delete mesh_cache_legacy[a];
			}
		}
		delete[] mesh_cache_legacy;
	}

	for (unsigned long a = 0; a < dfc_amount_legacy; ++a)
	{
		for (unsigned long b = 0; b < dfc_amount; ++b)
		{
			if (dfc_indx_to_id_table_legacy[a] != nullptr)
			{
				if (dfc_indx_to_id_table_legacy[a]->id == dfc_indx_to_id_table[b]->id)
				{
					goto dont_remove;
				}
			}
		}
		goto remove;

	dont_remove:
		continue;

	remove:

		if (dfc_indx_to_id_table_legacy[a] != nullptr)
		{
			dfc_id_indx_type** moved_element = dfc_ids.remove_element(dfc_indx_to_id_table_legacy[a]->index);
			if (moved_element != nullptr)
			{
				(*moved_element)->index = dfc_indx_to_id_table_legacy[a]->index;
			}
			delete dfc_indx_to_id_table_legacy[a];
		}
	}

	if (dfc_indx_to_id_table_legacy != nullptr)
	{
		delete[] dfc_indx_to_id_table_legacy;
	}

	vert_cache_legacy = nullptr;
	vert_cache_next_index_legacy = 0;
	vert_amount_total_legacy = 0;
	tri_cache_legacy = nullptr;
	tri_cache_size_legacy = nullptr;
	mesh_cache_legacy = nullptr;
	dfc_amount_legacy = 0;
	dfc_indx_to_id_table_legacy = nullptr;

	legacy_is_valid = false;
}


/*	Cleans dfc cache (note that this function also calls "clean_legacy")	*/
void df_type::update_local_type::dfc_cache_type::clean()
{
	if (tri_cache != nullptr)
	{
		for (unsigned long a = 0; a < dfc_amount; ++a)
		{
			if (tri_cache[a] != nullptr)
			{
				delete[] tri_cache[a];
			}
		}
		delete[] tri_cache;
	}

	if (tri_cache_size != nullptr)
	{
		delete[] tri_cache_size;
	}

	if (vert_cache != nullptr)
	{
		for (unsigned long a = 0; a < vert_amount_total; ++a)
		{
			if (vert_cache[a].tris != nullptr)
			{
				delete[] vert_cache[a].tris;
			}
		}
		delete[] vert_cache;
	}

	if (mesh_cache != nullptr)
	{
		for (unsigned long a = 0; a < dfc_amount; ++a)
		{
			if (mesh_cache[a] != nullptr)
			{
				unsigned long mesh_cache_y_size = mesh_cache[a]->size();
				for (unsigned long b = 0; b < mesh_cache_y_size; ++b)
				{
					if (mesh_cache[a]->at(b) != nullptr)
					{
						delete mesh_cache[a]->at(b);
					}
				}
				delete mesh_cache[a];
			}
		}
		delete[] mesh_cache;
	}

	if (dfc_indx_to_id_table != nullptr)
	{
		delete[] dfc_indx_to_id_table;
	}

	if (existing_dfcs != nullptr)
	{
		delete[] existing_dfcs;
	}

	vert_cache = nullptr;
	vert_cache_next_index = 0;
	vert_amount_total = 0;
	tri_cache = nullptr;
	tri_cache_size = nullptr;
	mesh_cache = nullptr;
	dfc_amount = 0;
	dfc_indx_to_id_table = nullptr;
	existing_dfcs = nullptr;

	is_valid = false;

	clean_legacy();
}

void df_type::update_local_type::dfc_cache_type::clean_dfc_ids()
{
	dfc_ids.clean();
}


void df_type::update_local_type::dfc_cache_type::deep_clean_dfc_ids()
{
	dfc_ids.calc_size();
	for (unsigned long a = 0u; a < dfc_ids.size; ++a)
	{
		delete dfc_ids.vector[a];
	}
	dfc_ids.clean();
}


/*df_type::update_local_type::dfc_cache_type::mesh_info_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::update_local_type::dfc_cache_type::mesh_info_type::calc_tris_size()
{
	tris_size = tris.size();
}


void df_type::update_local_type::dfc_cache_type::mesh_info_type::clean()
{
	std::vector<unsigned long>().swap(tris);
	tris_size = 0u;
	if (layer_indices != nullptr)
	{
		delete layer_indices;
		layer_indices = nullptr;
	}
	moved_from_legacy = false;
}


df_type::update_local_type::dfc_cache_type::mesh_info_type::~mesh_info_type()
{
	clean();
}


/*df_type::update_local_type::dfr_cache_type*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Returns a pointer to the actual "dfr_id_index_type" object of the speficied dfr id, returns	nullptr if canot be found	*/
df_type::dfr_id_indx_type* df_type::update_local_type::dfr_cache_type::get_dfr_id(const unsigned long& dfr_id)
{
	dfr_ids.calc_size();
	for (unsigned long a = 0; a < dfr_ids.size; ++a)
	{
		if (dfr_id == dfr_ids.vector[a]->id)
		{
			return dfr_ids.vector[a];
		}
	}

	return nullptr;
}


void df_type::update_local_type::dfr_cache_type::clean_dfr_ids()
{
	dfr_ids.clean();
}


void df_type::update_local_type::dfr_cache_type::deep_clean_dfr_ids()
{
	dfr_ids.calc_size();
	for (unsigned long a = 0u; a < dfr_ids.size; ++a)
	{
		delete dfr_ids.vector[a];
	}
	dfr_ids.clean();
}


/*df_type::update_local::buffer_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::update_local_type::buffer_type::clean()
{
	dfc_ids.calc_size();
	for (unsigned long a = 0u; a < dfc_ids.size; ++a)
	{
		if (dfc_ids.vector[a] != nullptr)
		{
			delete dfc_ids.vector[a];
		}
	}
	dfc_ids.clean();

	if (vert_cache != nullptr)
	{
		delete[] vert_cache;
		vert_cache = nullptr;
	}

	if (tri_cache_size != nullptr)
	{
		delete[] tri_cache_size;
		tri_cache_size = nullptr;
	}

	if (tri_cache != nullptr)
	{
		for (unsigned long a = 0u; a < dfc_amount; ++a)
		{
			if (tri_cache[a] != nullptr)
			{
				delete[] tri_cache[a];
			}
		}
		delete[] tri_cache;
		tri_cache = nullptr;
	}

	if (mesh_cache != nullptr)
	{
		for (unsigned long a = 0u; a < dfc_amount; ++a)
		{
			unsigned long size = mesh_cache[a]->size();
			for (unsigned long b = 0u; b < size; ++b)
			{
				delete (*mesh_cache[a])[b];
			}
			delete mesh_cache[a];
		}
		delete[] mesh_cache;
		mesh_cache = nullptr;
	}

	if (existing_dfcs != nullptr)
	{
		delete[] existing_dfcs;
		existing_dfcs = nullptr;
	}

	if (dfc_id_index_table != nullptr)
	{
		delete[] dfc_id_index_table;
		dfc_id_index_table = nullptr;
	}

	dfr_ids.calc_size();
	for (unsigned long a = 0u; a < dfr_ids.size; ++a)
	{
		if (dfr_ids.vector[a] != nullptr)
		{
			delete dfr_ids.vector[a];
		}
	}
	dfr_ids.clean();

	dfc_layers.clear();
	dfr_layers.clear();
}


df_type::update_local_type::buffer_type::~buffer_type()
{
	clean();
}


/*df_type::mesh_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::mesh_local_type::clean()
{
	dfc_indx = 0u;
	mesh_indx.clean();
	is_new_mesh = false;
	token = nullptr;
	mesh = nullptr;
}


/*df_type::tri_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::tri_local_type::clean()
{
	tri_index = 0;
	job_cmprt_table_cropped = nullptr;
	job_cmprt_table_next_index_y = 0;
	normal.clean();
	vert_0 = 0;
	vert_1 = 0;
	vert_2 = 0;
	unit_0_1.clean();
	distance_0_1 = .0;
	unit_0_2.clean();
	distance_0_2 = .0;
	unit_1_2.clean();
	distance_1_2 = .0;
	token = nullptr;
	jobs_completed = 0;
	mesh_local_state = nullptr;
}


/*df_type::update_recipients_local_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::update_recipients_local_type::clean()
{
	return;
}


/*	df_type::undo_step	*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::undo_step_type::init(	const shared_type::invrse_jenga_type<dfc_id_indx_type*,
									unsigned long>& dfc_ids, const shared_type::invrse_jenga_type<dfr_id_indx_type*,
									unsigned long>& dfr_ids, const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>&dfc_layers,
									const std::vector < shared_type::invrse_jenga_type < dfr_id_indx_type*, unsigned long>>&dfr_layers,
									const bool grid_is_valid,
									const shared_type::coord_xyz_type* intern_volume_verts,
									const float intern_df_distance,
									const unsigned short intern_df_cmprt_size,
									const double intern_grid_spacing,
									const int undo_index	)
{
	this->undo_index = undo_index;
	this->dfc_ids = dfc_ids;
	this->dfc_layers = dfc_layers;
	this->dfc_ids.calc_size();
	for (unsigned long a = 0u; a < this->dfc_ids.size; ++a)
	{
		if (this->dfc_ids.vector[a] != nullptr)
		{
			this->dfc_ids.vector[a] = new dfc_id_indx_type;
			this->dfc_ids.vector[a]->id = dfc_ids.vector[a]->id;
			this->dfc_ids.vector[a]->index = dfc_ids.vector[a]->index;

			unsigned long layer_amount = this->dfc_layers.size();
			for (unsigned long b = 0u; b < layer_amount; ++b)
			{
				this->dfc_layers[b].calc_size();
				for (unsigned long c = 0u; c < this->dfc_layers[b].size; ++c)
				{
					if (this->dfc_ids.vector[a]->id == this->dfc_layers[b].vector[c]->id)
					{
						this->dfc_layers[b].vector[c] = this->dfc_ids.vector[a];
					}
				}
			}
		}
	}

	this->dfr_ids = dfr_ids;
	this->dfr_layers = dfr_layers;
	this->dfr_ids.calc_size();
	for (unsigned long a = 0u; a < this->dfr_ids.size; ++a)
	{
		if (this->dfr_ids.vector[a] != nullptr)
		{
			this->dfr_ids.vector[a] = new dfr_id_indx_type;
			this->dfr_ids.vector[a]->id = dfr_ids.vector[a]->id;
			this->dfr_ids.vector[a]->index = dfr_ids.vector[a]->index;

			unsigned long layer_amount = this->dfr_layers.size();
			for (unsigned long b = 0u; b < layer_amount; ++b)
			{
				this->dfr_layers[b].calc_size();
				for (unsigned long c = 0u; c < this->dfr_layers[b].size; ++c)
				{
					if (this->dfr_ids.vector[a]->id == this->dfr_layers[b].vector[c]->id)
					{
						this->dfr_layers[b].vector[c] = this->dfr_ids.vector[a];
					}
				}
			}
		}
	}

	this->grid_is_valid = grid_is_valid;
	if (grid_is_valid)
	{
		for (unsigned short a = 0u; a < 8u; ++a)
		{
			this->intern_volume_verts[a] = intern_volume_verts[a];
		}
		this->intern_df_distance = intern_df_distance;
		this->intern_df_cmprt_size = intern_df_cmprt_size;
		this->intern_grid_spacing = intern_grid_spacing;
	}
}


df_type::undo_step_type::undo_step_type(	const shared_type::invrse_jenga_type<dfc_id_indx_type*,
											unsigned long>& dfc_ids,
											const shared_type::invrse_jenga_type<dfr_id_indx_type*,
											unsigned long>& dfr_ids,
											const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>&dfc_layers,
											const std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>&dfr_layers,
											const bool grid_is_valid,
											const shared_type::coord_xyz_type* intern_volume_verts,
											const float intern_df_distance,
											const unsigned short intern_df_cmprt_size,
											const double intern_grid_spacing,
											const int undo_index	)
{
	this->init(dfc_ids, dfr_ids, dfc_layers, dfr_layers, grid_is_valid, intern_volume_verts, intern_df_distance, intern_df_cmprt_size, intern_grid_spacing, undo_index);
}


df_type::undo_step_type::undo_step_type(const undo_step_type& undo_step)
{
	this->init(undo_step.dfc_ids, undo_step.dfr_ids, undo_step.dfc_layers, undo_step.dfr_layers, undo_step.grid_is_valid, undo_step.intern_volume_verts, undo_step.intern_df_distance, undo_step.intern_df_cmprt_size, undo_step.intern_grid_spacing, undo_step.undo_index);
}


df_type::undo_step_type df_type::undo_step_type::operator=(const undo_step_type& undo_step)
{
	this->init(undo_step.dfc_ids, undo_step.dfr_ids, undo_step.dfc_layers, undo_step.dfr_layers, undo_step.grid_is_valid, undo_step.intern_volume_verts, undo_step.intern_df_distance, undo_step.intern_df_cmprt_size, undo_step.intern_grid_spacing, undo_step.undo_index);

	return *this;
}


df_type::undo_step_type::~undo_step_type()
{
	this->dfc_ids.calc_size();
	for (unsigned long a = 0u; a < this->dfc_ids.size; ++a)
	{
		if (this->dfc_ids.vector[a] != nullptr)
		{
			delete this->dfc_ids.vector[a];
		}
	}

	this->dfr_ids.calc_size();
	for (unsigned long a = 0u; a < this->dfr_ids.size; ++a)
	{
		if (this->dfr_ids.vector[a] != nullptr)
		{
			delete this->dfr_ids.vector[a];
		}
	}
}


/*	df_type::undo_history_type	*/
/*-------------------------------------------------------------------------------------------------------------*/


void df_type::undo_history_type::add_undo_step(const shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>& dfc_ids, const shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>& dfr_ids, const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers, const std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers, const bool grid_is_valid, const shared_type::coord_xyz_type* intern_volume_verts, const float intern_df_distance, const unsigned short intern_df_cmprt_size, const double intern_grid_spacing, const int undo_index)
{
	this->intern_undo_history.push_back(undo_step_type(dfc_ids, dfr_ids, dfc_layers, dfr_layers, grid_is_valid, intern_volume_verts, intern_df_distance, intern_df_cmprt_size, intern_grid_spacing, undo_index));
	this->intern_active_undo_step = &this->intern_undo_history.back();
	if (this->intern_undo_history.size() > 1)
	{
		this->intern_previous_undo_step = &this->intern_undo_history[this->intern_undo_history.size() - 2];
	}
}


int df_type::undo_history_type::jump_to_closest_undo_step(const int undo_index, const bool delete_further_steps)
{
	int undo_history_size = this->intern_undo_history.size();
	for (int a = 0u; a < undo_history_size; ++a)
	{
		if (this->intern_undo_history[a].undo_index >= undo_index)
		{
			this->intern_active_undo_step = &this->intern_undo_history[a];
			if (delete_further_steps)
			{
				this->intern_undo_history.erase(this->intern_undo_history.begin() + (a + 1), this->intern_undo_history.begin() + undo_history_size);
			}
			if (this->intern_undo_history.size() > 1)
			{
				this->intern_previous_undo_step = &this->intern_undo_history[this->intern_undo_history.size() - 2];
			}
			return 0;
		}
	}

	return -1;
}


void df_type::undo_history_type::clean()
{
	std::vector<undo_step_type> temp = std::vector<undo_step_type>();
	this->intern_undo_history.swap(temp);
	this->intern_active_undo_step = nullptr;
	this->intern_previous_undo_step = nullptr;
}