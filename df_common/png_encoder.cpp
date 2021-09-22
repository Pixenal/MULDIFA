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
#include "../df_common/png_encoder.h"

inline shared_type shared;


/*	TODO:
	- Implement crc in "gen_crc"
	- Filter image, then deflate, then move to chunk data (use std::move)
*/

png_code_type::chunk_type::chunk_type(const char* type)
{
	for (unsigned short a = 0; a < 4; ++a)
	{
		this->type_byte_vec.char_vec.push_back(type[a]);
	}
}


int png_code_type::chunk_type::gen_crc()
{
	return 0;
}


int png_code_type::chunk_type::update_meta()
{
	/*	Updates length of data segmnet	*/
	this->intern_length = data.char_vec.size();
	this->length_byte_vec = shared_type::byte_vec_type();
	shared.feed_by_bit(this->length_byte_vec, 0u, this->intern_length, 64u);
	
	/*	Updates CRC	*/
	this->gen_crc();

	return 0;
}


int png_code_type::chunk_type::write_to_file(std::ofstream& file)
{
	shared.write_byte_vec(this->length_byte_vec, file, false);
	shared.write_byte_vec(this->type_byte_vec, file, false);
	shared.write_byte_vec(this->data, file, false);
	shared.write_byte_vec(this->crc_byte_vec, file, false);

	return 0;
}


png_code_type::png_code_type(const std::string name, const char** image, const unsigned short width, const unsigned short height)
{
	this->name = name;

	/*	IDHR Chunk (Header Chunk)	*/
	{
		this->chunks.push_back(chunk_type("IDHR"));

		/*	Alias	*/
		chunk_type& chunk = this->chunks[0];

		shared.feed_by_bit(chunk.data, 0u, width, 64u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), height, 64u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), this->bit_depth, 8u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), this->color_type, 8u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), this->compression_method, 8u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), this->filter_method, 8u);
		shared.feed_by_bit(chunk.data, (chunk.data.char_vec.size() - 1), this->interlace_method, 8u);
		chunk.update_meta();
	}

	/*	IDAT Chunk	(Image Data)*/
	{
		this->chunks.push_back(chunk_type("IDAT"));
	}

	/*	IEND Chunk	(End Marker)*/
	{
		this->chunks.push_back(chunk_type("IEND"));
		this->chunks.back().update_meta();
	}
}


int png_code_type::write_to_file(const std::string dir)
{
	std::ofstream file(dir + this->name, std::ios::binary);

	/*	Writes signiture	*/

	file.put((char)137);
	file.put((char)80);
	file.put((char)78);
	file.put((char)71);
	file.put((char)13);
	file.put((char)10);
	file.put((char)26);
	file.put((char)10);

	/*	Writes chunks	*/

	unsigned short chunks_size = this->chunks.size();
	for (unsigned short a = 0u; a < chunks_size; ++a)
	{
		this->chunks[a].write_to_file(file);
	}

	return 0;
}