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


#include "../df_common/deflate.h"
#include <vector>
#include <cmath>

inline shared_type shared;


unsigned long deflate_code_type::calc_adler32(const shared_type::byte_vec_type& message)
{
	unsigned long message_size = message.char_vec.size();
	unsigned long long low = 1ull;
	unsigned long long high = 0ull;
	unsigned long long high_intermediate_sum = 1ull;

	for (unsigned long a = 0u; a < message_size; ++a)
	{
		if (low == (std::numeric_limits<unsigned long long>::max)())
		{
			low %= 65521ull;
		}
		low += (unsigned char)message.char_vec[a];

		if (high == (std::numeric_limits<unsigned long long>::max)())
		{
			high %= 65521ull;
		}
		high_intermediate_sum += (unsigned char)message.char_vec[a];
		high += high_intermediate_sum;
	}
	low %= 65521ull;
	high %= 65521ull;
	unsigned long checksum = low + (65536ull * high);
	unsigned long checksum_reversed = 0ul;;
	for (unsigned short a = 0u; a < 4u; ++a)
	{
		unsigned long buffer  =0ul;
		memcpy(&buffer, &checksum, 1);
		checksum_reversed ^= (buffer << ((3u - a) * 8u));
		checksum >>= 8u;
	}

	return checksum_reversed;
}


int deflate_code_type::quick_sort(unsigned long* array, unsigned long* indices, const unsigned short array_size)
{
	/*	Padded as first and last are out of bounds (first = -1 and last = array_size)	*/
	const unsigned short array_size_padded = array_size + 2u;

	std::vector<short> pivots;
	pivots.push_back(-1);
	pivots.push_back(array_size);
	bool another_pass = true;

	while (another_pass)
	{
		another_pass = false;
		for (short a = 1; a < pivots.size(); ++a)
		{
			short dist = pivots[a] - (pivots[a - 1] + 1);
			if (dist > 1)
			{
				short pivot_index = (dist / 2) + pivots[a - 1];
				unsigned long pivot = array[pivot_index];
				unsigned long pivot_true_index = indices[pivot_index];
				/*	When swapping the last element with the pivot, the pivot isn't actually placed at the end,
					it's just kept in a seperate object, the the effect of swapping is still the same	*/
				array[pivot_index] = array[pivots[a] - 1];
				indices[pivot_index] = indices[pivots[a] - 1];
				bool stop_swapping = false;

				short b = pivots[a - 1] + 1;
				short c = (pivots[a] - 2);

			swapping:

				/* Largest from left	*/
				for (b = b; b < (pivots[a] - 1); ++b)
				{
					if (array[b] >= pivot)
					{
						/*	Smallest from right	*/
						for (c = c; c > pivots[a - 1]; --c)
						{
							if (array[c] <= pivot)
							{
								if (c > b)
								{
									/*	Swap	*/
									unsigned long left_entry = array[b];
									unsigned long left_true_index = indices[b];
									array[b] = array[c];
									indices[b] = indices[c];
									array[c] = left_entry;
									indices[c] = left_true_index;
									++b;
									--c;
									goto swapping;
								}
								else
								{
									/*	If not, swap pivot with left entry and stop swapping	*/
									goto swap_with_left_entry;
								}
							}
						}
					swap_with_left_entry:

						array[pivots[a] - 1] = array[b];
						indices[pivots[a] - 1] = indices[b];
						array[b] = pivot;
						indices[b] = pivot_true_index;
						pivot_index = b;
						goto end_of_swapping;
					}
				}
				pivot_index = pivots[a] - 1;
				array[pivot_index] = pivot;
				indices[pivot_index] = pivot_true_index;

			end_of_swapping:

				pivots.insert((pivots.begin() + a), pivot_index);
				++a;
				if (!another_pass)
				{
					another_pass = true;
				}
			}
		}
	}

	return 0;
}


int deflate_code_type::get_canonical_huffman_codewords(std::vector<bool>* codewords, unsigned short* codeword_lengths, unsigned long* alphabet_freqs, unsigned long* alphabet_indices, const unsigned long alphabet_size, const unsigned short max_length)
{
	{
		this->quick_sort(alphabet_freqs, alphabet_indices, alphabet_size);
		unsigned short nonzero_start = 0u;
		for (unsigned short a = 0u; a < alphabet_size; ++a)
		{
			if (alphabet_freqs[a] > 0ul)
			{
				break;
			}
			else
			{
				++nonzero_start;
			}
		}
		std::vector<unsigned short> overflowing;
		{
			huffman_tree_type* huffman_tree = new huffman_tree_type(alphabet_freqs, alphabet_indices, nonzero_start, alphabet_size);
			if (huffman_tree->tree == nullptr)
			{
				if ((alphabet_size - nonzero_start) == 1u)
				{
					codeword_lengths[alphabet_indices[nonzero_start]] = 1u;
					codewords[alphabet_indices[nonzero_start]].push_back(0);
					return 1;
				}
				else
				{
					return 2;
				}
			}
			std::vector<huffman_tree_type::node_type*> node_stack;
			std::vector<int> bin_stack;
			node_stack.push_back(huffman_tree->tree);
			bin_stack.push_back(0);
			while (node_stack.size() > 0)
			{
				if (node_stack.back()->extern_node)
				{
					std::cout << ((huffman_tree_type::extern_node_type*)node_stack.back())->symbol << " : ";
					bin_stack.pop_back();
					unsigned short index = ((huffman_tree_type::extern_node_type*)node_stack.back())->symbol;
					codeword_lengths[index] = bin_stack.size();
					for (short a = 0; a < codeword_lengths[index]; ++a)
					{
						codewords[index].push_back(bin_stack[a]);
					}
					if (codeword_lengths[index] > max_length)
					{
						overflowing.push_back(index);
					}
				}
				else
				{
					if (bin_stack.back() < 2)
					{
						node_stack.push_back(((huffman_tree_type::intern_node_type*)node_stack.back())->child_nodes[bin_stack.back()]);
						bin_stack.push_back(0);
						continue;
					}
					else
					{
						bin_stack.pop_back();
					}
				}
				node_stack.pop_back();
				if (bin_stack.size() > 0)
				{
					++bin_stack.back();
				}
			}
			delete huffman_tree;
		}

		std::cout << "PreOverflow" << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;

		for (short a = 0; a < alphabet_size; ++a)
		{
			std::cout << a << " : " << codeword_lengths[a] << " : ";
			for (short b = 0; b < codewords[a].size(); ++b)
			{
				std::cout << codewords[a][b];
			}
			std::cout << std::endl;
		}

		/*	Rearranges to prevent overflowing (if any overflowing codewords exist)	*/
		unsigned short overflowing_size = overflowing.size();
		for (short a = (overflowing_size - 1); a >= 0; --a)
		{
			unsigned short margin = 0u;
			while (true)
			{
				++margin;
				for (unsigned short b = 0u; b < alphabet_size; ++b)
				{
					if (codeword_lengths[b] == (max_length - margin))
					{
						codewords[overflowing[a]] = codewords[b];
						codewords[overflowing[a]].push_back(0);
						codewords[b].push_back(1);
						codeword_lengths[overflowing[a]] = codewords[overflowing[a]].size();
						++codeword_lengths[b];
						goto moved_up_tree;
					}
				}
			}
		moved_up_tree:

			overflowing.pop_back();
		}

		/*	Ensures complete set. 
			The above anti-overflow rearrangement can result in an compomplete set of lengths, eg:
			  O
			 / \
			O   O
			     \
			      O
			The below code checks for any branches like this and moves the codeword up the tree if it encounters one,
			resulting in this:
			  O
			 / \
			O   O
			*/
		{
			for (unsigned short a = 0u; a < alphabet_size; ++a)
			{
				if (codeword_lengths[a] < 2)
				{
					continue;
				}
				unsigned short parent_node_len = codeword_lengths[a] - 1u;
				for (unsigned short b = 0u; b < alphabet_size; ++b)
				{
					if ((b != a) && (codeword_lengths[b] > parent_node_len))
					{
						for (unsigned short c = 0u; c < parent_node_len; ++c)
						{
							if (codewords[a][c] != codewords[b][c])
							{
								goto doesnt_match;
							}
						}
						goto parent_node_is_valid;

					doesnt_match:

						continue;
					}
				}
				/*	Parent node is invalid
					(codeword[a] is the only child node of parent node)	*/
				codewords[a].pop_back();
				codeword_lengths[a] -= 1u;

			parent_node_is_valid:

				continue;
			}
		}
	}


	std::cout << "AfterOverflow" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;

	for (short a = 0; a < alphabet_size; ++a)
	{
		std::cout << a << " : " << codeword_lengths[a] << " : ";
		for (short b = 0; b < codewords[a].size(); ++b)
		{
			std::cout << codewords[a][b];
		}
		std::cout << std::endl;
	}

	/*	Convert to canonical huffman codewords	*/
	std::vector<unsigned long> bit_sorted_codewords[16];
	for (unsigned short a = 1u; a < 16u; ++a)
	{
		for (unsigned short b = 0u; b < alphabet_size; ++b)
		{
			if (codeword_lengths[b] == a)
			{
				bit_sorted_codewords[a].push_back(b);
			}
		}
	}
	unsigned long counter = (std::numeric_limits<unsigned long>::max)();
	for (unsigned short a = 1u; a < 16u; ++a)
	{
		for (unsigned short b = 0u; b < bit_sorted_codewords[a].size(); ++b)
		{
			++counter;
			if (b == 0u)
			{
				unsigned short last_bit_len;
				for (short c = a - 1; c >= 0; --c)
				{
					if (bit_sorted_codewords[c].size() > 0)
					{
						last_bit_len = c;
						goto last_found;
					}
				}
				/*	No last found, set last_bit_len to a so that a - last_bit_len == 0	*/
				last_bit_len = a;

			last_found:

				counter = counter << (a - last_bit_len);
			}
			std::bitset<16> bitset(counter);
			for (unsigned short c = 0u; c < codeword_lengths[bit_sorted_codewords[a][b]]; ++c)
			{
				unsigned short bitset_index = (codeword_lengths[bit_sorted_codewords[a][b]] - 1u) - c;
				codewords[bit_sorted_codewords[a][b]][c] = bitset[bitset_index];
			}
		}
	}

	
	std::cout << "CANONICAL" << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;

	for (short a = 0; a < alphabet_size; ++a)
	{
		std::cout << a << " : " << codeword_lengths[a] << " : ";
		for (short b = 0; b < codewords[a].size(); ++b)
		{
			std::cout << codewords[a][b];
		}
		std::cout << std::endl;
	}

	for (unsigned short a = 0u; a < alphabet_size; ++a)
	{
		for (unsigned short b = 0u; b < alphabet_size; ++b)
		{
			if ((codeword_lengths[b] > codeword_lengths[a]) && (codeword_lengths[a] != 0u))
			{
				for (unsigned short c = 0u; c < codewords[a].size(); ++c)
				{
					if (codewords[a][c] != codewords[b][c])
					{
						goto not_prefix;
					}
				}
				goto is_prefix;

			not_prefix:

				continue;

			is_prefix:

				std::cout << "IS_PREFIX" << std::endl;
			}
		}
	}
	
	return 0;
}


int deflate_code_type::encode(const shared_type::byte_vec_type& message)
{
	unsigned long adler32_checksum = this->calc_adler32(message);

	/*	Defines arrays to keep track of the frequency of symbols within the literal-length and distance alphabets	*/
	unsigned long lit_len_freqs[286] = {};
	unsigned long dist_freqs[30] = {};

	std::vector<unsigned short> intermediate_code;

	{
		unsigned long s_buffer_start = 0ul;
		unsigned long la_buffer_start = 1ul;
		unsigned long la_buffer_end = 0ul;
		const unsigned long message_size = message.char_vec.size();
		const unsigned long min_string_size = 3ul;
		/*	The values arn't decremented by one to get the index as the first element is pushed here
			equivalent to minusing by one, then pushing first element, then incrementing by one	*/
		if (message.char_vec.size() < this->max_lzss_length)
		{
			la_buffer_end = message.char_vec.size();
		}
		else
		{
			la_buffer_end = max_lzss_length;
		}
		{
			unsigned short non_negative;
			if (message.char_vec[0] < 0)
			{
				non_negative = (128 + message.char_vec[0]) + 128;
			}
			else
			{
				non_negative = message.char_vec[0];
			}
			intermediate_code.push_back(non_negative);
		}
		++lit_len_freqs[message.char_vec[0]];

		while (la_buffer_start < message_size)
		{
			unsigned long length = 1ul;
			unsigned long distance = 0ul;
			unsigned long match_index = la_buffer_start;
			for (long b = (la_buffer_start - 1); b >= (long)s_buffer_start; --b)
			{
				if (b == ((513 * 399) + 195))
				{
					int e = 1;
				}

				if (message.char_vec[b] == message.char_vec[la_buffer_start])
				{
					match_index = b;
					goto search_for_additional_matches;
				}
			}
			goto write_pair_or_literal;

		search_for_additional_matches:

			distance = la_buffer_start - match_index;
			for (unsigned long b = (match_index + 1ul); ((b + distance) <= la_buffer_end) && (length <= max_lzss_length); ++b)
			{
				if (message.char_vec[b + distance] != message.char_vec[b])
				{
					break;
				}
				++length;
			}

		write_pair_or_literal:

			if (length < min_string_size)
			{
				/*	Add literals	*/
				unsigned long match_next_index = match_index + length;
				for (unsigned long b = match_index; b < match_next_index; ++b)
				{
					unsigned short non_negative;
					if (message.char_vec[b] < 0)
					{
						non_negative = (128 + message.char_vec[b]) + 128;
					}
					else
					{
						non_negative = message.char_vec[b];
					}
					intermediate_code.push_back(non_negative);
					++lit_len_freqs[non_negative];
				}
			}
			else
			{
				/*	Add Pair	*/

				unsigned short length_symbol = 0u;
				unsigned short length_extra = 0u;
				unsigned short distance_symbol = 0u;
				unsigned short distance_extra = 0u;
				for (unsigned short b = 0u; b < this->len_alphabet_size; ++b)
				{
					if (length < this->len_alphabet[b])
					{
						length_symbol = 257 + (b - 1u);
						length_extra = length - this->len_alphabet[b - 1u];
						++lit_len_freqs[length_symbol];
						break;
					}
				}

				for (unsigned short b = 0u; b < this->dist_alphabet_size; ++b)
				{
					if (distance < this->dist_alphabet[b])
					{
						distance_symbol = b - 1u;
						distance_extra = distance - this->dist_alphabet[b - 1u];
						++dist_freqs[distance_symbol];
						break;
					}
				}

				{
					intermediate_code.push_back(length_symbol);
					intermediate_code.push_back(length_extra);
				}

				{
					intermediate_code.push_back(distance_symbol);
					intermediate_code.push_back(distance_extra);
				}
			}

			la_buffer_end += length;
			if (la_buffer_end >= message_size)
			{
				la_buffer_end = message_size - 1ul;
			}
			la_buffer_start += length;
			const unsigned long s_buffer_size = (la_buffer_start - 1ul) - s_buffer_start;
			if (s_buffer_size >= this->max_lzss_distance)
			{
				s_buffer_start = la_buffer_start - this->max_lzss_distance;
			}
		}
	}

	/*	Add stop codeword to end of code	*/
	intermediate_code.push_back(256);
	++lit_len_freqs[256];

	/*	Generate Huffman Trees	*/

	unsigned short lit_len_lengths[286] = {};
	std::vector<bool>* lit_len_codewords = new std::vector<bool>[286];
	{
		unsigned long lit_len_indices[286];
		for (unsigned short a = 0u; a < 286; ++a)
		{
			lit_len_indices[a] = a;
		}
		std::cout << "Lit Len" << std::endl;
		this->get_canonical_huffman_codewords(lit_len_codewords, lit_len_lengths, lit_len_freqs, lit_len_indices, 286ul, 15u);
	}

	unsigned short distance_lengths[30] = {};
	std::vector<bool>* distance_codewords = new std::vector<bool>[30];
	{
		unsigned long distance_indices[30];
		for (unsigned short a = 0u; a < 30; ++a)
		{
			distance_indices[a] = a;
		}
		std::cout << "Distance" << std::endl;
		this->get_canonical_huffman_codewords(distance_codewords, distance_lengths, dist_freqs, distance_indices, 30ul, 15u);
	}

	unsigned short secondary_len_lengths[19] = {};
	std::vector<bool>* secondary_len_codewords = new std::vector<bool>[19];
	{
		unsigned long secondary_len_freqs[19] = {};
		for (unsigned short a = 0u; a < 286; ++a)
		{
			++secondary_len_freqs[lit_len_lengths[a]];
		}
		for (unsigned short a = 0u; a < 30; ++a)
		{
			++secondary_len_freqs[distance_lengths[a]];
		}
		unsigned long secondary_len_indices[19];
		for (unsigned short a = 0u; a < 19u; ++a)
		{
			secondary_len_indices[a] = a;
		}
		std::cout << "Secondary" << std::endl;
		this->get_canonical_huffman_codewords(secondary_len_codewords, secondary_len_lengths, secondary_len_freqs, secondary_len_indices, 19ul, 7u);
		int e = 1;
	}

	/*	Writes final code	*/

	/*	Writes Zlib header	*/

		/*	CMF	*/

		/*	Compresion method (deflate)	*/
		shared.feed_by_bit(this->code, 0u, 8ul, 4u);
		/*	Compression info (specifies a window size of max 32k)	*/
		shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 7ul, 4u);

		/*	Flags	*/

		shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 30ul, 5u);
		shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ul, 1u);
		shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 1ul, 2u);

	/*	Writes 3bit Header	*/

	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 5ul, 3u);

	/*	Writes 14bit Header	*/

	shared.feed_by_bit(this->code, 0u, 29, 5u);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 29, 5u);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 15, 4u);

	/*	Writes Secondary tree	*/
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[16], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[17], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[18], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[0], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[8], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[7], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[9], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[6], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[10], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[5], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[11], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[4], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[12], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[3], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[13], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[2], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[14], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[1], 3);
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), secondary_len_lengths[15], 3);

	/*	Writes Literal Length tree*/
	for (unsigned short a = 0u; a < 286u; ++a)
	{
		std::bitset<8> bitset;
		for (unsigned short b = 0u; b < secondary_len_lengths[lit_len_lengths[a]]; ++b)
		{
			bitset[b] = secondary_len_codewords[lit_len_lengths[a]][b];
		}
		if (secondary_len_lengths[lit_len_lengths[a]] > 0u)
		{
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), bitset.to_ulong(), secondary_len_lengths[lit_len_lengths[a]]);
		}
		else
		{
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ul, 1u);
		}
	}

	/*	Writes Distance tree	*/
	for (unsigned short a = 0u; a < 30u; ++a)
	{
		std::bitset<8> bitset;
		for (unsigned short b = 0u; b < secondary_len_lengths[distance_lengths[a]]; ++b)
		{

			bitset[b] = secondary_len_codewords[distance_lengths[a]][b];
		}
		if (secondary_len_lengths[distance_lengths[a]] > 0u)
		{
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), bitset.to_ulong(), secondary_len_lengths[distance_lengths[a]]);
		}
		else
		{
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ul, 1u);
		}
	}

	/*	Writes contents	*/
	{
		int	context_flag = 0;
		unsigned long intermediate_code_size = intermediate_code.size();
		for (unsigned long a = 0u; a < intermediate_code_size; ++a)
		{
			if (context_flag == 0u)
			{

				std::bitset<16> bitset;
				for (unsigned short b = 0u; b < lit_len_lengths[intermediate_code[a]]; ++b)
				{
					bitset[b] = lit_len_codewords[intermediate_code[a]][b];
				}
				shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), bitset.to_ulong(), lit_len_lengths[intermediate_code[a]]);
				if (intermediate_code[a] > 256)
				{
					++a;
					unsigned short extra_bits = this->len_extra_bits[intermediate_code[a - 1u] - 257u];
					if (extra_bits > 0u)
					{
						shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), intermediate_code[a], extra_bits);
					}
					context_flag = 1u;
				}
			}
			else if (context_flag == 1u)
			{
				std::bitset<16> bitset;
				for (unsigned short b = 0u; b < distance_lengths[intermediate_code[a]]; ++b)
				{
					bitset[b] = distance_codewords[intermediate_code[a]][b];
				}
				shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), bitset.to_ulong(), distance_lengths[intermediate_code[a]]);
				++a;
				unsigned short extra_bits = this->dist_extra_bits[intermediate_code[a - 1u]];
				if (extra_bits > 0u)
				{
					shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), intermediate_code[a], extra_bits);
				}
				context_flag = 0u;
			}
		}
	}

	
	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ull, (7u - this->code.next_bit_index));

	/*	Writes Adler32 Checksum	*/

	shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), adler32_checksum, 32u);

	this->code.buffer_to_char_vec();
	/*
	unsigned long code_size = this->code.char_vec.size();
	unsigned char* code_uchar = new unsigned char[code_size];
	for (unsigned long a = 0u; a < code_size; ++a)
	{
		code_uchar[a] = this->code.char_vec[a];
	}
	unsigned long message_size = message.char_vec.size();
	unsigned char* message_decompressed = new unsigned char[message_size] {};
	//Insert decompress test here
	std::cout << "RESULT!!!!" << std::endl;
	for (unsigned short a = 0u; a < message_size; ++a)
	{
		std::cout << message_decompressed[a];
	}
	std::cout << std::endl;
	*/

	return 0;
	/*Only the alphabet symbols themselves are compressed with the huffman tree, not the extra bits (ie the extra bits are not replaced with a huffman codeword)	*/
}

deflate_code_type::deflate_code_type(const shared_type::byte_vec_type& message)
{
	this->encode(message);
}


deflate_code_type::huffman_tree_type::huffman_tree_type(const unsigned long* freqs, const unsigned long* indices, const unsigned long start, const unsigned long size)
{
	this->freqs = freqs;
	this->indices = indices;
	this->freqs_start = start;
	this->freqs_next = size;

	bool tree_completed = false;
	while (!tree_completed)
	{
		tree_completed = this->join();
	}
	if (this->node_roots.size() > 0)
	{
		this->tree = (intern_node_type*)this->node_roots[0];
	}
	else
	{
		this->tree = nullptr;
	}
}


int deflate_code_type::huffman_tree_type::join()
{
	unsigned short node_roots_size = this->node_roots.size();
	if (((this->freqs_next - this->freqs_start) + node_roots_size) >= 2)
	{
		dual_index_type entries_to_join[2];
		node_type* child_nodes[2];
		unsigned short freq_index_offset = 0u;
		for (unsigned short a = 0u; a < 2u; ++a)
		{
			entries_to_join[a] = dual_index_type((freqs_start + freq_index_offset), false);
			for (unsigned short b = 0u; b < node_roots_size; ++b)
			{
				if (node_roots[b]->weight < freqs[entries_to_join[a].index])
				{
					if (a == 1u)
					{
						if (entries_to_join[0].array)
						{
							if (entries_to_join[0].index == b)
							{
								continue;
							}
						}
					}
					entries_to_join[a] = dual_index_type(b, true);
					break;
				}
			}

			if (entries_to_join[a].array)
			{
				child_nodes[a] = this->node_roots[entries_to_join[a].index];
			}
			else
			{
				this->extern_nodes[this->extern_nodes_size].symbol = this->indices[entries_to_join[a].index];
				this->extern_nodes[this->extern_nodes_size].weight = this->freqs[entries_to_join[a].index];
				child_nodes[a] = &this->extern_nodes[this->extern_nodes_size];
				++this->extern_nodes_size;
				++freq_index_offset;
			}
		}

		this->intern_nodes[this->intern_nodes_size].weight = child_nodes[0]->weight + child_nodes[1]->weight;
		this->intern_nodes[this->intern_nodes_size].child_nodes.push_back(child_nodes[0]);
		this->intern_nodes[this->intern_nodes_size].child_nodes.push_back(child_nodes[1]);
		if (entries_to_join[0].array)
		{
			this->node_roots.push_back(&this->intern_nodes[this->intern_nodes_size]);
			this->node_roots.erase(this->node_roots.begin() + entries_to_join[0].index);
			if (entries_to_join[1].array)
			{
				--entries_to_join[1].index;
				this->node_roots.erase(this->node_roots.begin() + entries_to_join[1].index);
			}
			else
			{
				++this->freqs_start;
			}
		}
		else if (entries_to_join[1].array)
		{
			this->node_roots.push_back(&this->intern_nodes[this->intern_nodes_size]);
			this->node_roots.erase(this->node_roots.begin() + entries_to_join[1].index);
			++this->freqs_start;
		}
		else
		{
			this->node_roots.push_back(&this->intern_nodes[this->intern_nodes_size]);
			this->freqs_start += 2u;
		}
		++this->intern_nodes_size;

		return 0;
	}
	else
	{
		return 1;
	}
}