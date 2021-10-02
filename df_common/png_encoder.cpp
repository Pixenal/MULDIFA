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
#include "../df_common/deflate.h"
#include "../df_common/png_encoder.h"

inline shared_type shared;


/*	TODO:
	- Implement crc in "calc_crc"
	- Filter image, then deflate, then move to chunk data (use std::move)
*/

png_code_type::chunk_type::chunk_type(const char* type)
{
	for (unsigned short a = 0; a < 4; ++a)
	{
		shared.feed_by_bit(this->type_and_data, 0u, type[a], 8u);
	}
}


int png_code_type::chunk_type::calc_crc()
{
	//this->type_and_data.char_vec.swap(std::vector<char>)

	unsigned long crc = 0xFFFFFFFF;
	{
		unsigned long long message_size = this->type_and_data.char_vec.size();
		for (unsigned long long a = 0ull; a < message_size; ++a)
		{
			crc ^= (unsigned char)this->type_and_data.char_vec[a];
			for (unsigned short b = 0u; b < 8u; ++b)
			{
				if (crc & 1)
				{
					crc = (crc >> 1) ^ 0xEDB88320;
				}
				else
				{
					crc >>= 1;
				}
			}
		}
		crc ^= 0xFFFFFFFF;
	}
	shared.feed_by_bit(this->crc_byte_vec, 0u, crc, 32u);
	this->crc_byte_vec.buffer_to_char_vec();

	return 0;
}


int png_code_type::chunk_type::update_meta()
{
	/*	Updates length of data segmnet	*/
	this->intern_length = (type_and_data.char_vec.size() - 4);
	this->length_byte_vec = shared_type::byte_vec_type();
	shared.feed_by_bit(this->length_byte_vec, 0u, this->intern_length, 32u);
	this->length_byte_vec.buffer_to_char_vec();
	
	/*	Updates CRC	*/
	this->calc_crc();

	return 0;
}


int png_code_type::chunk_type::write_to_file(std::ofstream& file)
{
	shared.write_byte_vec(this->length_byte_vec, file, true);
	shared.write_byte_vec(this->type_and_data, file, false);
	shared.write_byte_vec(this->crc_byte_vec, file, true);

	return 0;
}

/*	Doesn't support less than 8 bits per sample	*/
png_code_type::png_code_type(const std::string& name, unsigned char** image, const unsigned short width, const unsigned short height)
{
	this->name = name;

	/*	IDHR Chunk (Header Chunk)	*/
	{
		this->chunks.push_back(chunk_type("IHDR"));

		/*	Alias	*/
		chunk_type& chunk = this->chunks[0];

		shared.feed_by_bit(chunk.type_and_data, (chunk.type_and_data.char_vec.size() - 1), height, 32u);
		shared.feed_by_bit(chunk.type_and_data, (chunk.type_and_data.char_vec.size() - 1), width, 32u);
		chunk.type_and_data.buffer_to_char_vec();
		std::reverse((chunk.type_and_data.char_vec.begin() + 4), chunk.type_and_data.char_vec.end());
		chunk.type_and_data.char_vec.push_back(this->bit_depth);
		chunk.type_and_data.char_vec.push_back(this->color_type);
		chunk.type_and_data.char_vec.push_back(this->compression_method);
		chunk.type_and_data.char_vec.push_back(this->filter_method);
		chunk.type_and_data.char_vec.push_back(this->interlace_method);
		chunk.update_meta();
	}

	/*	IDAT Chunk	(Image Data)*/
	{
		this->chunks.push_back(chunk_type("IDAT"));

		/*	Alias	*/
		chunk_type& chunk = this->chunks.back();

		/*	the filtering and conversion to byte_vec for loops are seperated for organization purposes (this is why they are not done in the same for loop)	*/
		chunk.type_and_data.buffer_to_char_vec();
		/*	Filtering	*/
		/*	Using filtering method "sub" (specified as method 1)	*/
		{
			this->filtered_image = new unsigned char* [height];
			unsigned short offset = this->bit_depth / 8;
			for (unsigned short a = 0u; a < height; ++a)
			{
				if (a == 399)
				{
					int e = 1;
				}
				this->filtered_image[a] = new unsigned char[width];
				this->filtered_image[a][0] = image[a][0];
				for (unsigned short b = 1u; b < width; ++b)
				{
					this->filtered_image[a][b] = image[a][b] - image[a][b - offset];
				}
			}
			/*	Offset of corresponding byte (eg, would be 1 for 8 bits per pixel)*/
		}
		shared_type::byte_vec_type data_byte_vec;
		for (unsigned short a = 0u; a < height; ++a)
		{
			data_byte_vec.char_vec.push_back(1);
			for (unsigned short b = 0u; b < width; ++b)
			{
				data_byte_vec.char_vec.push_back(this->filtered_image[a][b]);
			}
		}
		deflate_code_type deflated_data(data_byte_vec);
		const unsigned long deflated_data_size = deflated_data.code.char_vec.size();
		for (unsigned long a = 0u; a < deflated_data_size; ++a)
		{
			chunk.type_and_data.char_vec.push_back(deflated_data.code.char_vec[a]);
		}
		chunk.update_meta();
	}

	/*	IEND Chunk	(End Marker)*/
	{
		this->chunks.push_back(chunk_type("IEND"));
		this->chunks.back().type_and_data.buffer_to_char_vec();
		this->chunks.back().update_meta();
	}
}


int png_code_type::write_to_file(const std::string& dir)
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

	file.close();

	return 0;
}