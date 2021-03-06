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

class deflate_code_type
{
	/*	Types	*/

	class huffman_tree_type
	{
		/*	Types	*/

		struct dual_index_type
		{
			/*	Data Members	*/

			unsigned short index = 0u;
			bool array = false;

			/*	Member Functions	*/

			dual_index_type(const unsigned short index, const bool array);
			dual_index_type();
		};

	public:

		class node_type
		{
		public:

			/*	Data Members	*/

			bool extern_node = false;
			unsigned long weight = 0u;

			/*	Member Functions	*/

			virtual ~node_type();
		};

		class intern_node_type : public node_type
		{
		public:

			/*	Data Members*/

			std::vector<node_type*> child_nodes;

			/*	Member Functions	*/

			intern_node_type();
			virtual ~intern_node_type();
		};

		class extern_node_type : public node_type
		{
		public:

			/*	Data Members	*/

			unsigned short symbol = 0ul;

			/*	Member Functions	*/

			extern_node_type(const unsigned short symbol);
			extern_node_type();
			virtual ~extern_node_type();
		};

	private:

		/*	Data Members	*/

		const unsigned long* freqs = nullptr;
		const unsigned long* indices = nullptr;
		unsigned long freqs_next = 0ul;
		unsigned long freqs_start = 0ul;
		std::vector<node_type*> node_roots;
		/*	Afaik num of intern nodes = num extern - 1 */
		intern_node_type intern_nodes[285];
		unsigned short intern_nodes_size = 0u;
		extern_node_type extern_nodes[286];
		unsigned short extern_nodes_size = 0u;

		/*	Member Functions	*/

		int join();

	public:

		/*	Data Members	*/

		intern_node_type* tree = nullptr;

		/*	Member Functions	*/

		huffman_tree_type(const unsigned long* freqs, const unsigned long* indices, const unsigned long start, const unsigned long size);
	};

	/*	Data Members	*/

	const unsigned long max_lzss_length = 258ul;
	const unsigned long max_lzss_distance = 32768ul;
	/*	Last element in both alphabets is just padding	*/
	const unsigned short len_alphabet_size = 30u;
	const unsigned short dist_alphabet_size = 31u;
	const unsigned short dist_alphabet[31] = {	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49,
												65, 97, 129, 193, 257, 385, 513, 769, 1025,
												1537, 2049, 3073, 4097, 6145, 8193, 12289,
												16385, 24577, 32769 };
	const unsigned short dist_extra_bits[31] = {	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
	const unsigned short len_alphabet[30] = {	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17,
												19, 23, 27, 31, 35, 43, 51, 59, 67, 83,
												99, 115, 131, 163, 195, 227, 258, 259 };
	const unsigned short len_extra_bits[30] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5 };

public:

	/*	Data Members	*/

	shared_type::byte_vec_type code;

	/*	Member Functions	*/

	unsigned long calc_adler32(const shared_type::byte_vec_type& message);
	int quick_sort(unsigned long* array, unsigned long* indices, const unsigned short array_size);
	int move_up_codeword(std::vector<bool>* codewords, unsigned short* codeword_lengths, const unsigned short index, const unsigned short alphabet_size, const unsigned short max_length);
	int check_if_complete_set(const unsigned short* codeword_lengths, const unsigned short alphabet_size, const unsigned short max_length);
	int get_canonical_huffman_codewords(std::vector<bool>* codewords, unsigned short* codeword_lengths, unsigned long* alphabet_freqs, unsigned long* alphabet_indices, const unsigned long alphabet_size, const unsigned short max_length);
	int encode(const shared_type::byte_vec_type& message);
	deflate_code_type(const shared_type::byte_vec_type& message);
};