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


#pragma once

#include "../df_common/df_lib.h"

class png_code_type
{
	/*	Types	*/

	class chunk_type
	{
		/*	Data Members	*/

		shared_type::byte_vec_type length_byte_vec;
		shared_type::byte_vec_type crc_byte_vec;

		unsigned long long intern_length = 0ull;
		unsigned long long intern_crc = 0ull;

		//const char generator[5] = { 183, 29, 193, 4, 1 };
		const unsigned long long generator = 4374732215ull;

	public:

		/*	Data Members	*/

		const unsigned long long& length = intern_length;
		const unsigned long long& crc = intern_crc;
		shared_type::byte_vec_type type_and_data;

		/*	Length of data segment, not length of whole chunk	*/

		/*	Member Functions	*/

		chunk_type(const char* type);
		int calc_crc();
		int update_meta();
		int write_to_file(std::ofstream& file);
	};

	/*	Data Members	*/

	std::vector<chunk_type> chunks;
	unsigned char** filtered_image = nullptr;

	/*	They're unsigned long because the serielizer works with unsigned long longs	
		bit_depth = 8 bits per sample
		color_type = grayscale
		compression_method = deflate
		filter_method = adaptive filtering with 5 filter types (0 in this case does not mean no filtering is used)
		interlace_method = no interlacing
	*/
	unsigned long long bit_depth = 8ull;
	unsigned long long color_type = 0ull;
	unsigned long long compression_method = 0ull;
	unsigned long long filter_method = 0ull;
	unsigned long long interlace_method = 0ull;

public:

	std::string name;

	/*	Member Functions	*/

	png_code_type(const std::string& name, unsigned char** image, const unsigned short width, const unsigned short height);
	int write_to_file(const std::string& dir);

};