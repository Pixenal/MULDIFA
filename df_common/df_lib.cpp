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
#include "../df_common/thread_pool.h"


/*Creates main class type objects*/
inline shared_type shared;
thread_pool_type thread_pool;
df_type df;

std::string lib_dir;

/*Exported functions*/
/*-------------------------------------------------------------------------------------------------------------*/

extern "C"
{
	EXPORT int pass_lib_dir(const char* dir)
	{
		lib_dir = std::string(dir);
		return 0;
	}

	EXPORT int call_df_stash_state()
	{
		return df.stash_state();
	}

	EXPORT int call_df_unstash_state()
	{
		return df.unstash_state();
	}

	EXPORT int call_df_clean()
	{
		
		df.clean();

		return 0;
	}


	EXPORT int call_df_clean_special()
	{
		df.clean_special();

		return 0;
	}


	EXPORT int call_df_initialize_volume(const shared_type::coord_xyz_type* verts_buffer, const float df_distance, const unsigned short df_cmprt_size, const double grid_spacing, const bool skip_sync_check)
	{
		
		int return_value = df.initialize_volume(verts_buffer, df_distance, df_cmprt_size, grid_spacing, skip_sync_check);
		

		return return_value;
	}


	EXPORT int call_df_pre_update(const unsigned long* dfc_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount_total, const unsigned long* ignored_dfcs, const unsigned long ignored_dfcs_nxt_indx)
	{
		
		df.pre_update(dfc_ids, dfc_amount, vert_amount_total, ignored_dfcs, ignored_dfcs_nxt_indx);
		return 0;
	}


	EXPORT int call_df_update(const unsigned long& dfc_id, const unsigned long& dfc_index)
	{
		df.update_per_tri(dfc_id, dfc_index);
		return 0;
	}


	EXPORT int call_df_add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc)
	{
		df.add_dfc_to_cache(verts, vert_amount, tris, tri_amount, dfc_index, split_dfc);
		return 0;
	}


	EXPORT int call_df_pre_update_recipients(const unsigned long* dfrs, const unsigned long dfr_amount)
	{
		df.pre_update_recipients(dfrs, dfr_amount);

		return 0;
	}


	EXPORT int call_df_update_recipient(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::vert_info_type* verts_buffer, unsigned long& vert_amount, const int interp_mode, const float gamma)
	{
		df.update_recipient(dfc_layers, dfc_layers_nxt_indx, verts_buffer, vert_amount, interp_mode, gamma);

		return 0;
	}


	int call_df_update_recipient_df_map(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::coord_xyz_type* verts_buffer, const unsigned long vert_amount, shared_type::tri_info_type* tris_buffer, shared_type::tri_uv_info_type* tris_uv_buffer, const unsigned long tri_amount, const unsigned short height, const unsigned short width, const int interp_mode, const float gamma, const char* dir, const char* name, float padding)
	{
		df.update_recipient_df_map(dfc_layers, dfc_layers_nxt_indx, verts_buffer, vert_amount, tris_buffer, tris_uv_buffer, tri_amount, height, width, interp_mode, gamma, dir, name, padding);
		return 0;
	}


	EXPORT int call_df_post_update_recipients()
	{
		df.post_update_recipients();
		return 0;
	}


	EXPORT int call_df_check_volume(shared_type::coord_xyz_type* verts_buffer)
	{
		return int(df.check_volume(verts_buffer));
	}


	EXPORT int call_df_defrag_dfc_ids(unsigned long* dfc_ids, const unsigned long& dfc_amount, int& greatest_id)
	{
		
		df.defrag_dfc_ids(dfc_ids, dfc_amount, greatest_id);
		

		return 0;
	}


	EXPORT int call_df_defrag_dfr_ids(unsigned long* dfr_ids, const unsigned long& dfr_amount, int& greatest_id)
	{
		
		df.defrag_dfr_ids(dfr_ids, dfr_amount, greatest_id);
		

		return 0;
	}


	EXPORT unsigned long call_df_get_dfc_ids_size()
	{
		
		unsigned long return_value = df.get_dfc_id_size();
		
		return return_value;
	}


	EXPORT unsigned long call_df_get_dfr_ids_size()
	{
		
		unsigned long return_value = df.get_dfr_id_size();
		
		return return_value;
	}


	EXPORT unsigned long call_df_get_dfc_layer_size(const unsigned long& layer_indx)
	{
		
		unsigned long return_value = df.get_dfc_layer_size(layer_indx);
		
		return return_value;
	}


	EXPORT unsigned long call_df_get_dfr_layer_size(const unsigned long& layer_indx)
	{
		
		unsigned long return_value =  df.get_dfr_layer_size(layer_indx);
		
		return return_value;
	}


	EXPORT int call_df_add_dfc_layer()
	{
		return df.add_dfc_layer();	
	}


	EXPORT int call_df_add_dfr_layer()
	{
		
		return df.add_dfr_layer();
	}


	EXPORT int call_df_remove_dfc_layer(const unsigned long& layer_indx, unsigned long* expelled_dfcs, unsigned long& expelled_dfcs_nxt_indx)
	{
		
		return df.remove_dfc_layer(layer_indx, expelled_dfcs, expelled_dfcs_nxt_indx);
	}


	EXPORT int call_df_remove_dfr_layer(const unsigned long& layer_indx, unsigned long* expelled_dfrs, unsigned long& expelled_dfrs_nxt_indx)
	{
		
		return df.remove_dfr_layer(layer_indx, expelled_dfrs, expelled_dfrs_nxt_indx);
	}


	EXPORT int call_df_assign_dfcs_to_dfc_layer(const unsigned long& layer_indx, const unsigned long* dfc_ids, const unsigned long& dfc_ids_nxt_indx)
	{
		
		return df.assign_dfcs_to_dfc_layer(layer_indx, dfc_ids, dfc_ids_nxt_indx);
	}


	EXPORT int call_df_assign_dfrs_to_dfr_layer(const unsigned long& layer_indx, const unsigned long* dfr_ids, const unsigned long& dfr_ids_nxt_indx)
	{
		
		return df.assign_dfrs_to_dfr_layer(layer_indx, dfr_ids, dfr_ids_nxt_indx);
	}


	EXPORT int call_df_remove_dfcs_from_dfc_layer(const unsigned long& layer_indx, unsigned long* dfcs, unsigned long& dfcs_nxt_indx)
	{
		
		return df.remove_dfcs_from_dfc_layer(layer_indx, dfcs, dfcs_nxt_indx, true);
	}


	EXPORT int call_df_remove_dfrs_from_dfr_layer(const unsigned long& layer_indx, unsigned long* dfrs, unsigned long& dfrs_nxt_indx)
	{
		
		return df.remove_dfrs_from_dfr_layer(layer_indx, dfrs, dfrs_nxt_indx, true);
	}


	EXPORT int call_df_get_all_dfcs_in_dfc_layer(const unsigned long& layer_indx, unsigned long* dfc_ids)
	{
		
		return df.get_all_dfcs_in_dfc_layer(layer_indx, dfc_ids);
	}


	EXPORT int call_df_get_all_dfrs_in_dfr_layer(const unsigned long& layer_indx, unsigned long* dfr_ids)
	{
		
		return df.get_all_dfrs_in_dfr_layer(layer_indx, dfr_ids);
	}


	EXPORT int call_df_get_all_layers_with_dfc(const unsigned long dfc_id, unsigned long* layers, unsigned long& layers_nxt_indx)
	{
		return df.get_all_layers_with_dfc(dfc_id, layers, layers_nxt_indx);
	}


	EXPORT int call_df_get_all_layers_with_dfr(const unsigned long dfr_id, unsigned long* layers, unsigned long& layers_nxt_indx)
	{
		return df.get_all_layers_with_dfr(dfr_id, layers, layers_nxt_indx);
	}


	EXPORT int call_df_expel_nonexistant_dfcs_from_layers(const unsigned long* dfc_ids, const unsigned long dfc_amount)
	{
		return df.expel_nonexistant_dfcs_from_layers(dfc_ids, dfc_amount);
	}


	EXPORT int call_df_expel_nonexistant_dfrs_from_layers(const unsigned long* dfr_ids, const unsigned long dfr_amount)
	{
		return df.expel_nonexistant_dfrs_from_layers(dfr_ids, dfr_amount);
	}


	EXPORT int call_df_copy_to_buffer()
	{
		return df.copy_to_buffer();
	}


	EXPORT int call_df_get_write_id_index()
	{
		return df.get_write_id_index();
	}


	EXPORT int call_df_get_write_id_rand()
	{
		return df.get_write_id_rand();
	}


	EXPORT int call_df_stash_write_id()
	{
		return df.stash_write_id();
	}


	EXPORT int call_df_write_cache(const char* dir, const char* file_name)
	{
		return df.write_cache(dir, file_name);
	}


	EXPORT int call_df_new_blend_handler(const char* dir, const char* file_name, const shared_type::write_id_type& write_id, const int** dfc_layers, const int** dfr_layers, const bool df_cache_enabled)
	{
		int return_value = df.new_blend_handler(dir, file_name, write_id, dfc_layers, dfr_layers, df_cache_enabled);
		return return_value;
	}

	EXPORT int call_df_weak_stash_volume_local()
	{
		return df.weak_stash_volume_local();
	}

	EXPORT int call_df_weak_unstash_volume_local()
	{
		return df.weak_unstash_volume_local();
	}

	EXPORT int call_df_validate_undo_step(const int python_index, const bool delete_further_steps)
	{
		df.validate_undo_step(python_index, delete_further_steps);
		return 0;
	}

	EXPORT int call_df_incrmt_undo_step(const int python_index)
	{
		df.incrmt_undo_step(python_index);
		return 0;
	}

	EXPORT shared_type::write_id_type test_return()
	{
		shared_type::write_id_type test;
		test.index = 1056;
		return test;
	}
}


/*Encapsulation.exe stopped working*/
void call_update_grid_points(void* arg_ptr, unsigned short job_index)
{
	df.update_grid_points(arg_ptr, job_index);
}

void call_delete_rlvncy_buffers(void* args_ptr, unsigned short job_index)
{
	df.delete_rlvncy_buffers(args_ptr, job_index);
}

