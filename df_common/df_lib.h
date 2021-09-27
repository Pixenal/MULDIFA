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


#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <bitset>
#include <fstream>
#include <random>
#include <memory>
#include <cstring>

class shared_type
{
public:

	/*Types*/

	struct coord_xyz_type
	{
		/*Data Members*/

		double x = .0;
		double y = .0;
		double z = .0;

		/*Member Functions*/

		bool operator==(const coord_xyz_type& operand);
		bool operator==(coord_xyz_type& operand);
		void clean();
	};


	struct coord_xy_type
	{
		/*Data Members*/

		double x = .0;
		double y = .0;

		/*Member Functions*/

		void clean();
	};


	struct coord_uvw_type
	{
		/*Data Members*/

		double u = .0;
		double v = .0;
		double w = .0;

		/*Member Functions*/

		void clean();
	};


	struct vert_info_type
	{
		/*Data Members*/
		
		coord_xyz_type coord;
		float value = .0f;

		/*Member Functions*/

		void clean();
	};


	struct tri_info_type
	{
		/*Data Members*/

		unsigned long vert_0 = 0;
		unsigned long vert_1 = 0;
		unsigned long vert_2 = 0;
		coord_xyz_type normal;
		double d = .0;

		/*Member Functions*/

		void clean();
	};


	struct tri_uv_info_type
	{
		/*Data Members*/

		coord_xyz_type uv_vert_0;
		coord_xyz_type uv_vert_1;
		coord_xyz_type uv_vert_2;

		/*Member Functions*/

		void clean();
	};


	struct loop_info_type
	{
		/*Data Members*/

		unsigned long vert_index = 0;

		/*Member Functions*/

		void clean();
	};


	struct vert_tri_ref_type
	{
		/*Data Members*/

		unsigned long dfc_index = 0;
		coord_xyz_type coord;
		unsigned long* tris = nullptr;

		/*Member Functions*/

		void clean();
	};


	struct scale_xyz_type
	{
		/*Data Members*/

		double x = .0;
		double y = .0;
		double z = .0;

		/*Member Functions*/

		void clean();
	};


	struct index_xyzw_type
	{
		/*Data Members*/

		unsigned short x = 0;
		unsigned short y = 0;
		unsigned short z = 0;
		unsigned short w = 0;

		/*Member Functions*/

		void clean();
	};


	struct index_xyz_type
	{
		/*Data Members*/

		unsigned short x = 0;
		unsigned short y = 0;
		unsigned short z = 0;

		/*Member Functions*/

		void clean();

		bool operator==(const index_xyz_type& operand)
		{
			if (this->x != operand.x || this->y != operand.y || this->z != operand.z)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	};


	struct index_xy_type
	{
		/*Data Members*/

		unsigned short x = 0;
		unsigned short y = 0;

		/*Member Functions*/

		bool operator==(const index_xy_type& operand);
		void clean();
	};


	struct index_xyz_signed_type
	{
		/*Data Members*/

		int x = 0;
		int y = 0;
		int z = 0;

		/*Member Functions*/

		void clean();
	};


	struct index_xy_signed_type
	{
		/*Data Members*/

		int x = 0;
		int y = 0;

		/*Member Functions*/

		void clean();
	};


	struct amount_xyz_type
	{
		/*Data Members*/

		unsigned short x = 0;
		unsigned short y = 0;
		unsigned short z = 0;

		/*Member Functions*/

		void clean();
	};


	struct amount_xy_type
	{
		/*Data Members*/

		unsigned short x = 0;
		unsigned short y = 0;

		/*Member Functions*/

		void clean();
	};


	struct write_id_type
	{
		/*Data Members*/

		int index = 0u;
		int rand = 0u;

		write_id_type(const int index, const int rand)
		{
			this->index = index;
			this->rand = rand;
		}

		write_id_type()
		{

		}

		bool operator==(const write_id_type& write_id);
	};


	struct byte_arr_type
	{
		/*Data Members*/

		std::bitset<8>** arr = nullptr;
		unsigned short size = 0u;
		unsigned long next_bit_index = 0u;
		unsigned short next_byte_index = 0u;

		/*Member Functions*/

		byte_arr_type(const unsigned short& size)
		{
			arr = new std::bitset<8>*[size];
			this->size = size;
		}

		byte_arr_type()
		{

		}

		~byte_arr_type()
		{
			clean();
		}

		void initialize(const unsigned short& size)
		{
			arr = new std::bitset<8>*[size];
			this->size = size;

			for (unsigned short a = 0u; a < size; ++a)
			{
				arr[a] = nullptr;
			}
		}

		void clean()
		{
			if (arr != nullptr)
			{
				for (unsigned long long a = 0u; a < size; ++a)
				{
					if (arr[a] != nullptr)
					{
						delete arr[a];
					}
				}

				delete[] arr;
			}

			size = 0u;
			next_bit_index = 0u;
			next_byte_index = 0u;
		}
	};


	class byte_vec_type
	{
		/*Data Members*/

	public:

		/*Data Members*/

		std::vector<char> char_vec;
		std::bitset<8> byte_buffer = std::bitset<8>();
		unsigned long next_bit_index = 0u;
		unsigned long long next_byte_index = 0u;

		/*Member Functions*/

		byte_vec_type()
		{

		}

		~byte_vec_type()
		{
			clean();
		}

		void incrmt()
		{
			unsigned long long size = char_vec.size();
			if (size > 0)
			{
				char_vec[size - 1] = (char)byte_buffer.to_ulong();
			}
			
			char_vec.push_back(0);
			byte_buffer.reset();
		}
		void buffer_to_char_vec()
		{
			unsigned long long size = char_vec.size();
			char_vec[size - 1] = (char)byte_buffer.to_ulong();
		}
		int buffer_char(const unsigned long long& index)
		{
			if (index < char_vec.size())
			{
				byte_buffer = std::bitset<8>(char_vec[index]);
				return 0;
			}
			else
			{
				return 1;
			}
		}
		void clean()
		{
			byte_buffer.reset();

			if (char_vec.capacity() > 0)
			{
				std::vector<char>().swap(char_vec);
			}

			next_bit_index = 0u;
			next_byte_index = 0u;
		}
	};


	class ncspline_type
	{
		/*	Data Members	*/
	
		unsigned short knott_amount = 0u;
		double* knott_z_values = nullptr;
		index_xy_type spline_prpndclr_coord;
		bool intern_is_valid = false;
		double* knott_values = nullptr;
		const double hi = 1.0f;

	public:

		/*	Data Members	*/

		const bool& is_valid = intern_is_valid;

		/*	Member Functions	*/

		void init_spline(const double* knott_values, const unsigned short knott_values_nxt_indx, const index_xy_type& spline_prpndclr_coord);
		ncspline_type(const double* knott_values, const unsigned short knott_values_nxt_indx, const index_xy_type& spline_prpndclr_coord);
		ncspline_type(const index_xy_type& spline_prpndclr_coord);
		ncspline_type(const ncspline_type& ncspline);
		ncspline_type operator=(const ncspline_type& ncspline);
		~ncspline_type();
		ncspline_type();
		double sample(const double indx_coord);
	};


	template<typename Tintegral_type>
	class overflow_integral_type
	{
	private:

		/*Data Members*/

		Tintegral_type integer;

	public:

		/*Member Functions*/

		const Tintegral_type& value = integer;
		Tintegral_type max_value;
		Tintegral_type min_value;

		/*Member Functions*/

		overflow_integral_type(const Tintegral_type& initial_value = 0, const Tintegral_type& max_value = 1, const Tintegral_type& min_value = 0);
		/*__forceinline*/ void incrmt(const Tintegral_type& incrmt_amount);
		/*__forceinline*/ void decrmt(const Tintegral_type& decrmt_amount);
	};

	template<typename Tvector_type, typename Tindex_type>
	struct invrse_jenga_type
	{
		/*Data Members*/

		std::vector<Tvector_type> vector;
		Tindex_type size = 0;

		/*Member Functions*/

		Tvector_type* remove_element(const Tindex_type& index);
		void calc_size();
		void clean()
		{
			std::vector<Tvector_type>().swap(vector);
			size = 0;
		}
	};


	/*Data members*/

		const double pi = 3.141592653589793;
		const unsigned short sv_meta_size = 6;

	/*Member functions*/
	
		coord_uvw_type cartesian_to_barycentric(const coord_xyz_type& tri_vertex_0, const coord_xyz_type& tri_vertex_1, const coord_xyz_type& tri_vertex_2, const coord_xyz_type& point, const coord_xyz_type& tri_normal);
		coord_xyz_type barycentric_to_cartesian(const coord_xyz_type& tri_vertex_0, const coord_xyz_type& tri_vertex_1, const coord_xyz_type& tri_vertex_2, const coord_uvw_type& point);
		double lerp(const double& value_0, const double& value_1, const double& lerp_alpha);
		coord_xyz_type get_dist_vector(const coord_xyz_type& point_0, const coord_xyz_type& point_1);
		coord_xy_type get_dist_vector(const coord_xy_type& point_0, const coord_xy_type& point_1);
		double pythag(const coord_xyz_type& set);
		double pythag(const coord_xy_type& set);
		void invert_bit_order(unsigned long long& number);
		void invert_bit_order(unsigned long& number);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const unsigned long long number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const unsigned long number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const unsigned short number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const double number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const float number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const bool number, unsigned short bit_size);
		void feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const int number, unsigned short bit_size);
		void write_byte_vec(shared_type::byte_vec_type& byte_vec, std::ofstream& file, const bool& byte_endianness);
};


/*df class definition*/
/*-------------------------------------------------------------------------------------------------------------*/

class df_type
{
private:

	/*Types*/

	struct dfc_id_indx_type
	{
		unsigned long id = 0;
		unsigned long index = 0;

		void clean();
	};

	struct dfr_id_indx_type
	{
		unsigned long id = 0;
		unsigned long index = 0;

		void clean();
	};

	struct mesh_indx_type
	{
		/*Data Members*/

		unsigned long* dfc_id = nullptr;
		unsigned long mesh_index = 0;

		/*Member Functions*/

		void clean();

		/*Operator Overloads*/

		bool operator==(const mesh_indx_type& operand);
	};

	
	struct mesh_layer_type
	{
		/*Data Members*/

		float value = .0f;
		mesh_indx_type mesh_indx;
		unsigned long mesh_layer_table_index = 0;
	};


	class grid_type
	{
	private:

		/*Types*/
		
		struct last_layer_indx_cnvrsn_type
		{
			/*Data Members*/

			mesh_indx_type mesh_indx;
			unsigned short layer_index = 0;
		};

		/*Data Members*/
		
		last_layer_indx_cnvrsn_type last_layer_indx_cnvrsn;

	public:

		/*Types*/

		struct remove_layer_return_type
		{
			/*Data Members*/

			const mesh_indx_type* mesh_indx;
			const unsigned long* mesh_layer_table_index = 0;
			int return_code = 0;
		};

		/*Data Members*/

		std::vector<mesh_layer_type*> layers;
		unsigned short layers_size = 0;
		shared_type::index_xyz_type index;
		std::mutex* token = nullptr;
		inline static int do_once = 0;
		unsigned long temp_spline_indx = 0u;

		grid_type();

		/*Member Functions*/

		void calc_layer_size();
		float get_max_value();
		float get_max_value(const std::vector<unsigned long>& dfc_ids);
		float get_mesh_value(const mesh_indx_type& mesh_indx);
		int set_mesh_value(const float& value, const mesh_indx_type& mesh_indx);
		remove_layer_return_type remove_layer(const unsigned short& layer_indx);
		void clean();
	};


	struct grid_cell_value_type
	{
		/*Data Members*/

		float mmm = 0;
		float mmx = 0;
		float mxm = 0;
		float mxx = 0;
		float xmm = 0;
		float xmx = 0;
		float xxm = 0;
		float xxx = 0;

		/*Member Functions*/

		void clean();
	};


	struct cmprt_meta_type
	{
		/*Data members*/

		unsigned long size = 0;
		unsigned long rlvncy_table_size = 0;

		/*Member Functions*/

		void clean();
	};
	

	struct volume_local_type
	{
		/*Types*/

		struct grid_points_type
		{
			std::vector<unsigned long> grid_point_meta;
			std::vector<unsigned long> grid_point_indices;
			df_type::mesh_layer_type** grid_point_values = nullptr;

			void clean();
			~grid_points_type();
		};

		struct cmprt_data_type
		{
			/*Types*/

			struct cmprt_type
			{
				unsigned long size = 0u;
				unsigned long* grid_points = nullptr;
				unsigned long rlvncy_table_size = 0u;
				unsigned long* rlvncy_table = nullptr;

				void clean();
				~cmprt_type();
			};

			shared_type::amount_xyz_type cmprt_amount;
			cmprt_type* cmprts = nullptr;

			/*Member Functions*/

			void clean();

			~cmprt_data_type();
		};
		
		struct buffer_type
		{
			bool grid_is_valid = false;
			shared_type::coord_xyz_type grid_min_coord;
			double grid_spacing = .0;
			float df_distance = .0f;
			unsigned short cmprt_size = 0u;
			shared_type::coord_xyz_type volume_verts[8];
			shared_type::amount_xyz_type grid_point_amount;
			shared_type::index_xyz_type max_grid_point;
			grid_points_type grid_points;
			cmprt_data_type cmprt_data;

			void clean();
			void deep_clean();
			~buffer_type();
		};

		struct delete_rlvncy_buffers_local_type
		{
			unsigned long start_index = 0u;
			unsigned long end_next_index = 0u;
			std::vector<shared_type::index_xyz_type>** vector_arr = nullptr;
		};


		struct delete_rlvncy_buffers_shared_local_type
		{
			delete_rlvncy_buffers_local_type** args = nullptr;
			unsigned long jobs_completed = 0u;
			std::mutex token;
		};

		/*Data Members*/


		grid_type*** grid = nullptr;	//
		grid_type*** cmprt_table = nullptr;	//
		cmprt_meta_type* cmprt_meta = nullptr;	//
		unsigned long** cmprt_rlvncy_table = nullptr;	//

		shared_type::coord_xyz_type volume_verts_max;

		shared_type::amount_xyz_type grid_amount;	//
		shared_type::index_xyz_type max_grid_point;	//
		shared_type::scale_xyz_type grid_scale;
		shared_type::coord_xyz_type min_grid_coord;

		shared_type::amount_xyz_type cmprt_amount;	//
		unsigned long cmprt_amount_total = 0;	//

		float intern_df_distance = .0f;	//
		unsigned short intern_df_cmprt_size = 0;	//
		double intern_grid_spacing = .0;	//

		shared_type::coord_xyz_type intern_volume_verts[8];	//

		buffer_type buffer;
		bool weak_stashed = false;



		/*Member Functions*/

		void clean();
		void clean_special();
		void write_to_buffer(buffer_type& buffer);
		void load_from_buffer(const buffer_type& buffer, shared_type::invrse_jenga_type<df_type::dfc_id_indx_type*, unsigned long> dfc_ids);
		void clean_internal_buffer();
		volume_local_type();
	};


	struct update_local_type
	{
		/*Types*/

		class dfc_cache_type
		{
		private:

			/*Member Functions*/

			
			//##################################################################################################
		public:

			/*Types*/

			struct mesh_info_type
			{
				/*Data Members*/

				std::vector<unsigned long> tris;
				unsigned long tris_size = 0u;
				std::vector<shared_type::index_xyzw_type>* layer_indices = nullptr;
				bool moved_from_legacy = false;

				/*Mmeber Functions*/

				void calc_tris_size();
				void clean();
				~mesh_info_type();
			};

			/*Data Members*/

			bool is_valid = false;	//
			bool legacy_is_valid = false;
			shared_type::vert_tri_ref_type* vert_cache = nullptr; //
			shared_type::vert_tri_ref_type* vert_cache_legacy = nullptr; //
			unsigned long vert_cache_next_index = 0; //
			unsigned long vert_cache_next_index_legacy = 0; //
			unsigned long vert_amount_total = 0; //
			unsigned long vert_amount_total_legacy = 0; //
			shared_type::tri_info_type** tri_cache = nullptr;
			shared_type::tri_info_type** tri_cache_legacy = nullptr;
			unsigned long* tri_cache_size = nullptr;
			unsigned long* tri_cache_size_legacy = nullptr;
			std::vector<mesh_info_type*>** mesh_cache = nullptr;
			std::vector<mesh_info_type*>** mesh_cache_legacy = nullptr;
			unsigned long dfc_amount = 0;
			bool* existing_dfcs = nullptr;
			unsigned long dfc_amount_legacy = 0;
			shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long> dfc_ids;
			dfc_id_indx_type** dfc_indx_to_id_table = nullptr;
			dfc_id_indx_type** dfc_indx_to_id_table_legacy = nullptr;

			/*Member Functions*/

			/*__forceinline*/ void check_for_uniq_adjcnt_tris(const unsigned long& vert_index, unsigned long& base, unsigned long& top, unsigned long& top_last_base_incrmt, std::vector<unsigned long>& calced_verts, mesh_info_type* mesh, const unsigned long& dfc_index);
			void remove_layers_return_handler(const grid_type::remove_layer_return_type& return_obj, const unsigned short& remove_layers_indx_param);
			int remove_mesh_layers(const unsigned long& dfc_id, const unsigned long& mesh_indx, const bool& legacy, const volume_local_type& volume_local);
			long long dfc_id_to_indx(const unsigned long& dfc_id, const int& legacy_setting);
			dfc_id_indx_type* get_dfc_id(const unsigned long& dfc_id);
			void initialize_cache(const unsigned long* obj_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount);
			void add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc);
			int move_to_legacy();
			void clean_legacy();
			void clean();
			void clean_dfc_ids();
			void deep_clean_dfc_ids();
		};


		struct dfr_cache_type
		{
			/*Data Members*/

			shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long> dfr_ids;

			/*Member Functions*/

			dfr_id_indx_type* get_dfr_id(const unsigned long& dfr_id);
			void clean_dfr_ids();
			void deep_clean_dfr_ids();
		};

		struct buffer_type
		{
			unsigned long dfc_amount = 0u;
			shared_type::invrse_jenga_type<df_type::dfc_id_indx_type*, unsigned long> dfc_ids;
			bool dfc_cache_is_valid = false;
			unsigned long vert_amount_total = 0u;
			unsigned long vert_cache_next_index;
			shared_type::vert_tri_ref_type* vert_cache = nullptr;
			unsigned long* tri_cache_size = nullptr;
			shared_type::tri_info_type** tri_cache = nullptr;
			std::vector<df_type::update_local_type::dfc_cache_type::mesh_info_type*>** mesh_cache = nullptr;
			bool* existing_dfcs = nullptr;
			dfc_id_indx_type** dfc_id_index_table = nullptr;
			shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long> dfr_ids;
			std::vector<shared_type::invrse_jenga_type<df_type::dfc_id_indx_type*, unsigned long>> dfc_layers;
			std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>> dfr_layers;

			void clean();
			~buffer_type();
		};

		/*Data Members*/

		unsigned long dfc_index = 0;
		std::mutex token;
		unsigned long batches_sent = 0;
		unsigned long batches_completed = 0;
		dfc_cache_type dfc_cache;
		dfr_cache_type dfr_cache;

		/*Member Functions*/

		void clean();
		void deep_clean();
		void load_from_buffer(buffer_type& buffer);
	};


	struct mesh_local_type
	{
		/*Data Members*/

		unsigned long dfc_indx = 0u;
		mesh_indx_type mesh_indx;
		bool is_new_mesh = false;
		bool has_changed = false;
		unsigned long tris_completed = 0;
		std::mutex* token = nullptr;
		update_local_type::dfc_cache_type::mesh_info_type* mesh = nullptr;

		/*Member Functions*/

		void clean();
	};


	struct tri_local_type
	{
		/*Data Members*/

		unsigned long tri_index = 0;
		unsigned long** job_cmprt_table_cropped = nullptr;
		unsigned long job_cmprt_table_next_index_y = 0;
		shared_type::coord_xyz_type normal;
		const shared_type::coord_xyz_type* vert_0 = nullptr;
		const shared_type::coord_xyz_type* vert_1 = nullptr;
		const shared_type::coord_xyz_type* vert_2 = nullptr;
		shared_type::coord_xyz_type unit_0_1;
		double distance_0_1 = .0;
		shared_type::coord_xyz_type unit_0_2;
		double distance_0_2 = .0;
		shared_type::coord_xyz_type unit_1_2;
		double distance_1_2 = .0;
		std::mutex* token = nullptr;
		unsigned int jobs_completed = 0;
		mesh_local_type* mesh_local_state = nullptr;

		/*Member Functions*/

		void clean();
	};


	struct update_recipients_local_type
	{
		/*Data Members*/

		/*Member Functions*/

		void clean();
	};


	class regions_buffer_type
	{
		/*Types*/



	public:



		/*Data Members*/

		shared_type::byte_vec_type grid_is_valid;
		shared_type::byte_vec_type write_id;
		shared_type::byte_vec_type grid_min_coord;
		/*intern*/
		shared_type::byte_vec_type grid_spacing;
		shared_type::byte_vec_type df_distance;
		shared_type::byte_vec_type cmprt_size;
		shared_type::byte_vec_type volume_verts;
		/*intern*/
		shared_type::byte_vec_type grid_point_amount;
		shared_type::byte_vec_type max_grid_point;
		shared_type::byte_vec_type grid_meta;
		shared_type::byte_vec_type grid_values;
		shared_type::byte_vec_type cmprt_data;
		shared_type::byte_vec_type dfc_amount;
		shared_type::byte_vec_type dfc_ids;
		shared_type::byte_vec_type dfc_cache_is_valid;
		shared_type::byte_vec_type vert_amount_total;
		shared_type::byte_vec_type vert_cache_next_index;
		shared_type::byte_vec_type vert_cache;
		shared_type::byte_vec_type tri_cache_size;
		shared_type::byte_vec_type tri_cache;
		shared_type::byte_vec_type mesh_cache;
		shared_type::byte_vec_type existing_dfcs;
		shared_type::byte_vec_type dfc_id_index_table;
		shared_type::byte_vec_type dfr_ids;
		shared_type::byte_vec_type dfc_layers;
		shared_type::byte_vec_type dfr_layers;

		bool is_valid = false;



		/*Member Functions*/

		void prep_grid_is_valid(const volume_local_type& volume_local);
		void prep_write_id(const shared_type::write_id_type write_id);
		void prep_grid_min_coord(const volume_local_type& volume_local);
		void prep_grid_spacing(const volume_local_type& volume_local);
		void prep_df_distance(const volume_local_type& volume_local);
		void prep_cmprt_size(const volume_local_type& volume_local);
		void prep_volume_verts(const volume_local_type& volume_local);
		void prep_grid_point_amount(const volume_local_type& volume_local);
		void prep_max_grid_point(const volume_local_type& volume_local);
		void prep_grid_meta(const volume_local_type& volume_local);
		void prep_cmprt_data(const volume_local_type& volume_local);
		void prep_dfc_amount(update_local_type& update_local);
		void prep_dfc_ids(update_local_type& update_local);
		void prep_dfc_cache_is_valid(update_local_type& update_local);
		void prep_vert_amount_total(update_local_type& update_local);
		void prep_vert_cache_next_index(update_local_type& update_local);
		void prep_vert_cache(update_local_type& update_local);
		void prep_tri_cache_size(update_local_type& update_local);
		void prep_tri_cache(update_local_type& update_local);
		void prep_mesh_cache(volume_local_type& volume_local, update_local_type& update_local);
		void prep_existing_dfcs(update_local_type& update_local);
		void prep_dfc_id_index_table(update_local_type& update_local);
		void prep_dfr_ids(update_local_type& update_local);
		void prep_dfc_layers(std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers);
		void prep_dfr_layers(std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers);

	public:

		/*Types*/

		class virtual_stream_type
		{
			/*Types*/

			struct index_type
			{
				short region = 0u;
				long long byte = 0u;
			};

			/*Data Members*/

			bool intern_end_of_stream = false;

		public:

			index_type index;
		private:

			unsigned long long current_region_size = 0u;
			regions_buffer_type* regions_buffer_ptr = nullptr;
			const unsigned short regions_size = 26u;
			shared_type::byte_vec_type* regions_arr[26u] = {};

			/*Member Functions*/

			int incrmt_index();

		public:

			/*Data Members*/

			const bool& end_of_stream = intern_end_of_stream;

			/*Member Functions*/

			virtual_stream_type(regions_buffer_type& regions_buffer);
			virtual_stream_type();
			std::bitset<8> get_byte();

			virtual_stream_type operator=(const virtual_stream_type& old);
		};

		/*Member Functions*/

		virtual_stream_type get_virtual_stream();
		
		void clean();
	};

	class df_writer_type
	{
		/*Types*/

	public:

		/*Data Members*/

		regions_buffer_type* regions_buffer;
		std::ofstream file;

		/*Member Functions*/

		df_writer_type(const std::string& file_dir, const std::string& file_name, regions_buffer_type& regions_buffer);
		void write_to_file();
		~df_writer_type();
	};

	/*Data members*/

	volume_local_type volume_local;
	update_local_type update_local;
	update_recipients_local_type update_recipients_local;
	regions_buffer_type regions_buffer;
	unsigned long write_index = 0u;

	/*Types*/

	class df_loader_type
	{
		/*Data Members*/

		bool file_mode = false;

	public:

		class director_type
		{
			/*Types*/

			class buffers_type
			{
				/*Data Members*/

			public:
				
				shared_type::write_id_type write_id;
				df_type::volume_local_type::buffer_type volume_local;
				df_type::update_local_type::buffer_type update_local;

			private:

				df_loader_type* df_loader_ptr = nullptr;

				/*Member Functions*/

				unsigned short get_ushort();
				unsigned short get_ushort(const unsigned short bit_size);
				unsigned long get_ulong();
				unsigned long get_ulong(const unsigned short bit_size);
				unsigned long long get_ullong();
				float get_float();
				double get_double();
				bool get_bool();
				bool get_bool_byte();
				int get_int();

			public:

				void set_df_loader_ptr(df_loader_type& df_loader);
				void buffer_grid_is_valid();
				void buffer_write_id();
				void buffer_grid_min_coord();
				void buffer_grid_space();
				void buffer_df_distance();
				void buffer_cmprt_size();
				void buffer_volume_verts();
				void buffer_grid_point_amount();
				void buffer_max_grid_point();
				void buffer_grid_points();
				void buffer_cmprt_data();
				void buffer_dfc_amount();
				void buffer_dfc_ids();
				void buffer_dfc_cache_is_valid();
				void buffer_vert_amount_total();
				void buffer_vert_cache_next_index();
				void buffer_vert_cache();
				void buffer_tri_cache_size();
				void buffer_tri_cache();
				void buffer_mesh_cache();
				void buffer_existing_dfcs();
				void buffer_dfc_id_index_table();
				void buffer_dfr_ids();
				void buffer_dfc_layers();
				void buffer_dfr_layers();
			};

			/*Data Members*/

			df_loader_type* df_loader_ptr = nullptr;
			buffers_type macro_regions_buffer;
			bool file_mode = false;

			

		public:

			/*Data Members*/

			shared_type::write_id_type * write_id_ptr;
			std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>* dfc_layers_ptr;
			std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>* dfr_layers_ptr;

			/*Member Functions*/

			void set_df_loader_ptr(df_loader_type& df_loader);
			void call_volume_local_load();
			void call_update_local_load();
			void load_to_buffer();
			void load_to_df_state();
			void set_file_mode(bool file_mode);
		};

		class reader_type
		{
			/*Data Member*/

			df_type* df_ptr = nullptr;
			bool byte_buffer_occupied = false;
			std::bitset<8> byte_buffer;
			std::ifstream* file = nullptr;
			//Next bit index is garenteed to be below 8, as this is always checked for when incrememting the index
			unsigned short next_bit_index = 0u;
			bool file_mode = false;
			regions_buffer_type::virtual_stream_type virtual_stream;

			/*Member Functions*/

			void validate_byte_buffer();

		public:

			/*Data Members*/

			shared_type::byte_vec_type read_byte_vec_buffer;

			/*Member Functions*/

			reader_type(std::ifstream& file);
			reader_type();
			void read_spec_bits(const unsigned short bit_size);
			void read_sv();
			bool read_bit();
			void iterate_byte_buffer();
			void clear_byte_buffer();
			void skip_region();
			void set_file_mode(bool file_mode);
			void set_df_ptr(df_type& df);
			void get_virtual_stream();
		};

		class bin_interpretor_type
		{
			/*Data Members*/

			df_loader_type* df_loader_ptr = nullptr;
			bool file_mode = false;

		public:


			unsigned short intrp_as_ushort();
			unsigned long intrp_as_ulong();
			unsigned long long intrp_as_ullong();
			float intrp_as_float();
			double intrp_as_double();
			int intrp_as_int();
			void set_file_mode(bool file_mode);
			void set_df_loader_ptr(df_loader_type& df_loader_ptr);
		};

		/*Data Members*/

		std::ifstream file;
		director_type director;
		reader_type reader;
		bin_interpretor_type bin_interpretor;
		volume_local_type* volume_local_ptr = nullptr;
		update_local_type* update_local_ptr = nullptr;
		regions_buffer_type* regions_buffer_ptr = nullptr;
		df_type* df_ptr;

		/*Member Functions*/

		df_loader_type(const std::string& dir, const std::string& file_name, volume_local_type& volume_local, update_local_type& update_local, df_type& df);
		df_loader_type(regions_buffer_type& regions_buffer, volume_local_type& volume_local, update_local_type& update_local, df_type& df);
		~df_loader_type();
	};


	class undo_step_type
	{
		/*	Member Functions	*/

		void init(	const shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>& dfc_ids,
					const shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>& dfr_ids,
					const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers,
					const std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers,
					const bool grid_is_valid,
					const shared_type::coord_xyz_type* intern_volume_verts,
					const float intern_df_distance,
					const unsigned short intern_df_cmprt_size,
					const double intern_grid_spacing,
					const int undo_index	);

	public:

		/*	Data Members	*/

		int undo_index = 0u;
		shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long> dfc_ids;
		shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long> dfr_ids;
		std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>> dfc_layers;
		std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>> dfr_layers;

		bool grid_is_valid;
		shared_type::coord_xyz_type intern_volume_verts[8];
		float intern_df_distance = .0f;
		unsigned short intern_df_cmprt_size = 0;
		double intern_grid_spacing = .0;

		/*	Member Functions	*/

		undo_step_type(	const shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>& dfc_ids,
						const shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>& dfr_ids,
						const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers,
						const std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers,
						const bool grid_is_valid,
						const shared_type::coord_xyz_type* intern_volume_verts,
						const float intern_df_distance,
						const unsigned short intern_df_cmprt_size,
						const double intern_grid_spacing,
						const int undo_index	);
		undo_step_type(const undo_step_type& undo_step);
		undo_step_type operator=(const undo_step_type& undo_step);
		~undo_step_type();
	};

	class undo_history_type
	{
		/*	Data Members	*/

		std::vector<undo_step_type> intern_undo_history;
		const undo_step_type* intern_active_undo_step = nullptr;
		const undo_step_type* intern_previous_undo_step = nullptr;

		/*	Member Functions	*/
		
	public:

		/*	Data Members	*/

		//const std::vector<undo_step_type>& undo_history = intern_undo_history;
		const undo_step_type* const& active_undo_step = intern_active_undo_step;
		const undo_step_type* const& previous_undo_step = intern_previous_undo_step;

		/*	Member Functions	*/

		void add_undo_step(	const shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>& dfc_ids,
							const shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>& dfr_ids,
							const std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers,
							const std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers,
							const bool grid_is_valid,
							const shared_type::coord_xyz_type* intern_volume_verts,
							const float intern_df_distance,
							const unsigned short intern_df_cmprt_size,
							const double intern_grid_spacing,
							const int undo_index	);
		int jump_to_closest_undo_step(const int undo_index, const bool delete_further_steps);
		void clean();
	};


	/*Data Members*/

	const unsigned short max_gb = 12u;
	shared_type::write_id_type write_id_buffer;
	undo_history_type undo_history;
	
	/*Member functions*/

	shared_type::coord_xyz_type wrld_space_to_cmprt_indx_space(const shared_type::coord_xyz_type& coordinates);
	double trunc_to_pnt_five_incrmts(const double& value);
	void rasterize_on_axis(shared_type::index_xyz_type* enclosing_cmprts, unsigned short& enclosing_cmprts_next_index, const shared_type::coord_xyz_type& max_vert, const shared_type::coord_xyz_type& mid_vert, const shared_type::coord_xyz_type& min_vert, const char& axis);
	void sort_on_axis(shared_type::coord_xyz_type& max_write_obj, shared_type::coord_xyz_type& mid_write_obj, shared_type::coord_xyz_type& min_write_obj, const shared_type::coord_xyz_type& coord_0, const shared_type::coord_xyz_type& coord_1, const shared_type::coord_xyz_type& coord_2, const char& axis);
	float lerp_btwn_grid_points_values(const unsigned short& min_grid_point_coord, const float& min_grid_point_value, const unsigned short& max_grid_point_coord, const float& max_grid_point_value, double lerp_alpha);
	dfc_id_indx_type* get_dfc_in_dfc_layer(unsigned long& return_dfc_indx, const unsigned long& layer_indx, const unsigned long& dfc_id);
	dfc_id_indx_type* get_dfc_in_dfc_layer(const unsigned long& layer_indx, const unsigned long& dfc_id);
	dfr_id_indx_type* get_dfr_in_dfr_layer(unsigned long& return_dfr_indx, const unsigned long& layer_indx, const unsigned long& dfr_id);
	dfr_id_indx_type* get_dfr_in_dfr_layer(const unsigned long& layer_indx, const unsigned long& dfr_id);

public:

	/*Types*/

	typedef tri_local_type tri_local_type_pub;
	typedef dfc_id_indx_type dfc_id_indx_type;
	typedef dfr_id_indx_type dfr_id_indx_type;

	/*Data members*/

	grid_type*** const& grid = volume_local.grid;
	const float& distance = volume_local.intern_df_distance;
	const update_local_type::dfc_cache_type& dfc_cache = update_local.dfc_cache;
	std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>> dfc_layers;
	std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>> dfr_layers;

	/*Member functions*/

	df_type()
	{

	}

	int add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc);
	int initialize_volume(const shared_type::coord_xyz_type* volume_verts, const float& df_distance, const unsigned short& df_cmprt_size, const double& grid_spacing, const bool skip_sync_check);
	int pre_update(const unsigned long* dfc_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount_total, const unsigned long* ignored_dfcs, const unsigned long ignored_dfcs_nxt_indx);
	int update_grid_points(void* arg_ptr, unsigned short job_index);
	int update_per_tri(const unsigned long& dfc_id, const unsigned long& dfc_index);
	int pre_update_recipients(const unsigned long* dfrs, const unsigned long dfr_amount);
	shared_type::coord_xyz_type wrld_space_to_grid_indx_space(const shared_type::coord_xyz_type coord);
	float get_lerped_point_value(const shared_type::coord_xyz_type& vert_coord, const std::vector<unsigned long>& dfc_ids, const char mode, std::vector<shared_type::ncspline_type>& zaligned_splines, const int local_spline_length);
	bool grid_bounds_check(const shared_type::coord_xyz_type& coord);
	shared_type::index_xyz_type get_enclsing_cmprt(const shared_type::coord_xyz_type& coord);
	shared_type::index_xyz_type get_enclsing_cmprt_from_indx_space(const shared_type::coord_xyz_type& indx_space_coord);
	int update_recipient(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::vert_info_type* verts_buffer, unsigned long& vert_amount, const int interp_mode, const float gamma);
	int update_recipient_df_map(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::coord_xyz_type* verts_buffer, const unsigned long vert_amount, shared_type::tri_info_type* tris_buffer, shared_type::tri_uv_info_type* tris_uv_buffer, const unsigned long tri_amount, const unsigned short height, const unsigned short width, const int interp_mode, const float gamma, const char* dir, const char* name);
	int post_update_recipients();
	int clean();
	int clean_special();
	int check_volume(const shared_type::coord_xyz_type* volume_verts);
	int defrag_dfc_ids(unsigned long* dfc_ids, const unsigned long& dfc_amount, int& greatest_id);
	int defrag_dfr_ids(unsigned long* dfr_ids, const unsigned long& dfr_amount, int& greatest_id);
	unsigned long get_dfc_id_size();
	unsigned long get_dfr_id_size();
	unsigned long get_dfc_layer_size(const unsigned long& layer_indx);
	unsigned long get_dfr_layer_size(const unsigned long& layer_indx);
	int add_dfc_layer();
	int add_dfr_layer();
	int remove_dfc_layer(const unsigned long& layer_indx, unsigned long* expelled_dfcs, unsigned long& expelled_dfcs_nxt_indx);
	int remove_dfr_layer(const unsigned long& layer_indx, unsigned long* expelled_dfrs, unsigned long& expelled_dfrs_nxt_indx);
	int assign_dfcs_to_dfc_layer(const unsigned long& layer_indx, const unsigned long* dfc_ids, const unsigned long& dfc_ids_nxt_indx);
	int assign_dfrs_to_dfr_layer(const unsigned long& layer_indx, const unsigned long* dfr_ids, const unsigned long& dfr_ids_nxt_indx);
	int remove_dfcs_from_dfc_layer(const unsigned long& layer_indx, unsigned long* dfcs, unsigned long& dfcs_nxt_indx, const bool enable_expel);
	int remove_dfrs_from_dfr_layer(const unsigned long& layer_indx, unsigned long* dfrs, unsigned long& dfrs_nxt_indx, const bool enable_expel);
	int get_all_dfcs_in_dfc_layer(const unsigned long& layer_indx, unsigned long* dfc_ids);
	int get_all_dfrs_in_dfr_layer(const unsigned long& layer_indx, unsigned long* dfr_ids);
	int get_all_layers_with_dfc(const unsigned long dfc_id, unsigned long* layers, unsigned long& layers_nxt_indx);
	int get_all_layers_with_dfr(const unsigned long dfr_id, unsigned long* layers, unsigned long& layers_nxt_indx);
	int expel_nonexistant_dfcs_from_layers(const unsigned long* dfc_ids, const unsigned long dfc_amount);
	int expel_nonexistant_dfrs_from_layers(const unsigned long* dfr_ids, const unsigned long dfr_amount);
	int load_dfc_dfr_layers_from_buffer(std::vector<shared_type::invrse_jenga_type<dfc_id_indx_type*, unsigned long>>& dfc_layers, std::vector<shared_type::invrse_jenga_type<dfr_id_indx_type*, unsigned long>>& dfr_layers);
	int copy_to_buffer();
	int get_write_id_index();
	int get_write_id_rand();
	int stash_write_id();
	int new_blend_handler(const char* dir, const char* file_name, const shared_type::write_id_type& write_id, const int** dfc_layers, const int** dfr_layers, const bool df_cache_enabled);
	int  write_cache(const char* dir, const char* file_name);
	int stash_state();
	int unstash_state();
	int weak_stash_volume_local();
	int weak_unstash_volume_local();
	int delete_rlvncy_buffers(void* args_ptr, unsigned short job_index);
	int validate_undo_step(const int python_index, const bool delete_further_steps);
	int incrmt_undo_step(const int python_index);

	template <typename T>
	void remove_deleted_element_from_layer_system(T& layer_system, int (df_type::*removal_function)(const unsigned long&, unsigned long*, unsigned long&, const bool), const unsigned long* elements, const unsigned long element_amount, const unsigned long* ignored_elements, const unsigned long ignored_elements_nxt_indx);

private:

	/*Member Functions*/

	void clean_df_layers();
};


void call_update_grid_points(void* arg_ptr, unsigned short job_index);
void call_delete_rlvncy_buffers(void* args_ptr, unsigned short job_index);


/*Exported function prototypes*/
/*-------------------------------------------------------------------------------------------------------------*/


extern "C"
{
	EXPORT int pass_lib_dir(const char* dir);
	EXPORT int call_df_clean();
	EXPORT int call_df_clean_special();
	EXPORT int call_df_initialize_volume(const shared_type::coord_xyz_type* verts_buffer, const float df_distance, const unsigned short df_cmprt_size, const double grid_spacing, const bool skip_sync_check);
	EXPORT int call_df_pre_update(const unsigned long* dfc_ids, const unsigned long& dfc_amount, const unsigned long& vert_amount_total, const unsigned long* ignored_dfcs, const unsigned long ignored_dfcs_nxt_indx);
	EXPORT int call_df_update(const unsigned long& dfc_id, const unsigned long& dfc_index);
	EXPORT int call_df_add_dfc_to_cache(const shared_type::coord_xyz_type* verts, const unsigned long& vert_amount, const shared_type::tri_info_type* tris, const unsigned long& tri_amount, const unsigned long& dfc_index, const bool& split_dfc);
	EXPORT int call_df_pre_update_recipients(const unsigned long* dfrs, const unsigned long dfr_amount);
	EXPORT int call_df_update_recipient(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::vert_info_type* verts_buffer, unsigned long& vert_amount, const int interp_mode, const float gamma);
	EXPORT int call_df_update_recipient_df_map(const unsigned long* dfc_layers, const unsigned long& dfc_layers_nxt_indx, shared_type::coord_xyz_type* verts_buffer, const unsigned long vert_amount, shared_type::tri_info_type* tris_buffer, shared_type::tri_uv_info_type* tris_uv_buffer, const unsigned long tri_amount, const unsigned short height, const unsigned short width, const int interp_mode, const float gamma, const char* dir, const char* name);
	EXPORT int call_df_post_update_recipients();
	EXPORT int call_df_check_volume(shared_type::coord_xyz_type* verts_buffer);
	EXPORT int call_df_defrag_dfc_ids(unsigned long* dfc_ids, const unsigned long& dfc_amount, int& greatest_id);
	EXPORT int call_df_defrag_dfr_ids(unsigned long* dfr_ids, const unsigned long& dfr_amount, int& greatest_id);
	EXPORT unsigned long call_df_get_dfc_ids_size();
	EXPORT unsigned long call_df_get_dfr_ids_size();
	EXPORT unsigned long call_df_get_dfc_layer_size(const unsigned long& layer_indx);
	EXPORT unsigned long call_df_get_dfr_layer_size(const unsigned long& layer_indx);
	EXPORT int call_df_add_dfc_layer();
	EXPORT int call_df_add_dfr_layer();
	EXPORT int call_df_remove_dfc_layer(const unsigned long& layer_indx, unsigned long* expelled_dfcs, unsigned long& expelled_dfcs_nxt_indx);
	EXPORT int call_df_remove_dfr_layer(const unsigned long& layer_indx, unsigned long* expelled_dfrs, unsigned long& expelled_dfrs_nxt_indx);
	EXPORT int call_df_assign_dfcs_to_dfc_layer(const unsigned long& layer_indx, const unsigned long* dfc_ids, const unsigned long& dfc_ids_nxt_indx);
	EXPORT int call_df_assign_dfrs_to_dfr_layer(const unsigned long& layer_indx, const unsigned long* dfr_ids, const unsigned long& dfr_ids_nxt_indx);
	EXPORT int call_df_remove_dfcs_from_dfc_layer(const unsigned long& layer_indx, unsigned long* dfcs, unsigned long& dfcs_nxt_indx);
	EXPORT int call_df_remove_dfrs_from_dfr_layer(const unsigned long& layer_indx, unsigned long* dfrs, unsigned long& dfrs_nxt_indx);
	EXPORT int call_df_get_all_dfcs_in_dfc_layer(const unsigned long& layer_indx, unsigned long* dfc_ids);
	EXPORT int call_df_get_all_dfrs_in_dfr_layer(const unsigned long& layer_indx, unsigned long* dfr_ids);
	EXPORT int call_df_get_all_layers_with_dfc(const unsigned long dfc_id, unsigned long* layers, unsigned long& layers_nxt_indx);
	EXPORT int call_df_get_all_layers_with_dfr(const unsigned long dfr_id, unsigned long* layers, unsigned long& layers_nxt_indx);
	EXPORT int call_df_expel_nonexistant_dfcs_from_layers(const unsigned long* dfc_ids, const unsigned long dfc_amount);
	EXPORT int call_df_expel_nonexistant_dfrs_from_layers(const unsigned long* dfr_ids, const unsigned long dfr_amount);
	EXPORT int call_df_copy_to_buffer();
	EXPORT int call_df_get_write_id_index();
	EXPORT int call_df_get_write_id_rand();
	EXPORT int call_df_stash_write_id();
	EXPORT int call_df_write_cache(const char* dir, const char* file_name);
	EXPORT int call_df_new_blend_handler(const char* dir, const char* file_name, const shared_type::write_id_type& write_id, const int** dfc_layers, const int** dfr_layers, const bool df_cache_enabled);
	EXPORT int call_df_weak_stash_volume_local();
	EXPORT int call_df_weak_unstash_volume_local();
	EXPORT int call_df_validate_undo_step(const int python_index, const bool delete_further_steps);
	EXPORT int call_df_incrmt_undo_step(const int python_index);
}


/*Template Definitions*/
/*-------------------------------------------------------------------------------------------------------------*/


template<typename Tvector_type, typename Tindex_type>
inline Tvector_type* shared_type::invrse_jenga_type<Tvector_type, Tindex_type>::remove_element(const Tindex_type& index)
{
	Tindex_type last_index = vector.size() - 1;

	if (index == last_index)
	{
		vector.pop_back();

		return nullptr;
	}
	else
	{
		vector[index] = vector[last_index];
		vector.pop_back();

		return &vector[index];
	}
}


template<typename Tvector_type, typename Tindex_type>
inline void shared_type::invrse_jenga_type<Tvector_type, Tindex_type>::calc_size()
{
	size = vector.size();
}


template<typename Tintegral_type>
inline shared_type::overflow_integral_type<Tintegral_type>::overflow_integral_type(const Tintegral_type& initial_value, const Tintegral_type& max_value, const Tintegral_type& min_value)
{
	integer = initial_value;
	this->max_value = max_value;
	this->min_value = min_value;
}


template<typename Tintegral_type>
/*__forceinline*/ void shared_type::overflow_integral_type<Tintegral_type>::incrmt(const Tintegral_type& incrmt_amount)
{
	integer += incrmt_amount;

	if (integer > max_value)
	{
		integer = min_value;
	}
	else if (integer < min_value)
	{
		integer = max_value;
	}
}


template<typename Tintegral_type>
/*__forceinline*/ void shared_type::overflow_integral_type<Tintegral_type>::decrmt(const Tintegral_type& decrmt_amount)
{
	integer -= decrmt_amount;

	if (integer < min_value)
	{
		integer = max_value;
	}
	else if (integer > max_value)
	{
		integer = min_value;
	}
}


/*	Checks that all elements that exist within the layer system exist within blender, and expels any that do not.
This is to catch cases where an element may have been assigned to a layer, and then deleted without being properly removed, (and
in the case of dfc's, without update being called, preventing the element from being included in the dfc_legacy cache, and thus caught
in the clean up performed in "df_type::pre_update")	*/
template <typename T>
inline void df_type::remove_deleted_element_from_layer_system(T& layer_system, int (df_type::*removal_function)(const unsigned long&, unsigned long*, unsigned long&, const bool), const unsigned long* elements, const unsigned long element_amount, const unsigned long* ignored_elements, const unsigned long ignored_elements_nxt_indx)
{
	unsigned long layer_amount = layer_system.size();
	for (unsigned long a = 0u; a < layer_amount; ++a)
	{
		layer_system[a].calc_size();

		unsigned long* expelled_dfcs = new unsigned long[layer_system[a].size];
		unsigned long expelled_dfcs_nxt_indx = 0ul;

		for (unsigned long b = 0u; b < layer_system[a].size; ++b)
		{
			for (unsigned long c = 0u; c < element_amount; ++c)
			{
				if (layer_system[a].vector[b]->id == elements[c])
				{
					goto exists;
				}
			}
			goto doesnt_exist;

		exists:

			continue;

		doesnt_exist:

			for (unsigned long c = 0u; c < ignored_elements_nxt_indx; ++c)
			{
				if (layer_system[a].vector[b]->id == ignored_elements[c])
				{
					goto exists_in_ignored;
				}
			}
			goto doesnt_exist_in_ignored;

		exists_in_ignored:

			continue;

		doesnt_exist_in_ignored:

			for (unsigned long c = 0u; c < expelled_dfcs_nxt_indx; ++c)
			{
				if (layer_system[a].vector[b]->id == expelled_dfcs[c])
				{
					goto exists_in_expelled;
				}
			}
			goto doesnt_exist_in_expelled;

		exists_in_expelled:

			continue;

		doesnt_exist_in_expelled:

			expelled_dfcs[expelled_dfcs_nxt_indx] = layer_system[a].vector[b]->id;
			++expelled_dfcs_nxt_indx;
		}

		if (expelled_dfcs_nxt_indx > 0u)
		{
			(this->*removal_function)(a, expelled_dfcs, expelled_dfcs_nxt_indx, true);
		}
	}
}