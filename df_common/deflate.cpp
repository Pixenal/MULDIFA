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


#include "../df_common/deflate.h"
#include <vector>
#include <cmath>

inline shared_type shared;

/*	Returns Adler32 checksum
	(this is uesd in the zlib specification)	*/
unsigned long deflate_code_type::calc_adler32(const shared_type::byte_vec_type& message)
{
	/*	The algorithm for calculating the Adler32 checksum can be described as:
		sum_0 + (65536 * sum_1),
		Where sum_0 = (1 + byte_0 + byte_1 + .... + byte_n-1) % 65521
		and sum_1 = ((1 + byte_0) + (1 + byte_0 + byte_1) + .... + (1 + byte_0 + .... + byte_n-1)) % 65521	*/

	/*	Calculates the checksum	*/
	unsigned long message_size = message.char_vec.size();
	unsigned long long sum_0 = 1ull;
	unsigned long long sum_1 = 0ull;
	unsigned long long sum_1_intermediate_sum = 1ull;
	for (unsigned long a = 0u; a < message_size; ++a)
	{
		if (sum_0 == (std::numeric_limits<unsigned long long>::max)())
		{
			sum_0 %= 65521ull;
		}
		sum_0 += (unsigned char)message.char_vec[a];

		if (sum_1 == (std::numeric_limits<unsigned long long>::max)())
		{
			sum_1 %= 65521ull;
		}
		sum_1_intermediate_sum += (unsigned char)message.char_vec[a];
		sum_1 += sum_1_intermediate_sum;
	}
	sum_0 %= 65521ull;
	sum_1 %= 65521ull;
	unsigned long checksum = sum_0 + (65536ull * sum_1);

	/*	Reverses the byte order of the checksum	*/
	unsigned long checksum_reversed = 0ul;;
	for (unsigned short a = 0u; a < 4u; ++a)
	{
		unsigned long buffer = 0ul;
		memcpy(&buffer, &checksum, 1);
		checksum_reversed ^= (buffer << ((3u - a) * 8u));
		checksum >>= 8u;
	}

	return checksum_reversed;
}


/*	Performs a quicksort on the specified array. The "indices" parameter stores the new position of each element after sorting	*/
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


/*	Moves the specified huffman codeword up the tree. This is done by taking the longest codeword with a length shorter than the specified maximum, and creating a new branch at that codeword which
	contains both the found and specified codewords. Note that this function does not handle the codewords fellow child (if one exists), so as such, can, and usually will, leave the tree in an incomplete
	state	(in practice this is taken care of by the calling function)	*/
int deflate_code_type::move_up_codeword(std::vector<bool>* codewords, unsigned short* codeword_lengths, const unsigned short index, const unsigned short alphabet_size, const unsigned short max_length)
{
	/*	As mentioned above, this function swaps the codeword with the longest codeword below "max length". To do this, the below while loop iterates through each length between ("max_length" - 1) and 0,
		where "margin" represents the current distance from "max_length". If no codeword is found to be of the current length, then margin is incremented by 1.	*/
	unsigned short margin = 0u;
	while (true)
	{
		++margin;
		for (unsigned short b = 0u; b < alphabet_size; ++b)
		{
			if (codeword_lengths[b] == (max_length - margin))
			{
				codewords[index] = codewords[b];
				codewords[index].push_back(0);
				codewords[b].push_back(1);
				codeword_lengths[index] = codewords[index].size();
				++codeword_lengths[b];
				return 0;
			}
		}
	}
	return 1;
}


/*	This function exists to test wether or not the specified huffman tree is complete (this algorithm is used within the zlib library)	*/
int deflate_code_type::check_if_complete_set(const unsigned short* codeword_lengths, const unsigned short alphabet_size, const unsigned short max_length)
{
	long left = 1ul;;
	long* len_count = new long[alphabet_size]{};
	for (unsigned short a = 0u; a < alphabet_size; ++a)
	{
		++len_count[codeword_lengths[a]];
	}
	for (short a = 1u; a <= max_length; ++a)
	{
		left <<= 1;
		left -= len_count[a];
		if (left < 0)
		{
			return 1;
		}
	}
	delete[] len_count;
	return (left == 0ul) ? 0 : 1;
}


/*	Generates cannonical huffman codewords	*/
int deflate_code_type::get_canonical_huffman_codewords(std::vector<bool>* codewords, unsigned short* codeword_lengths, unsigned long* alphabet_freqs, unsigned long* alphabet_indices, const unsigned long alphabet_size, const unsigned short max_length)
{
	{
		/*	The list of frequencies is first sorted fro low to high	*/
		this->quick_sort(alphabet_freqs, alphabet_indices, alphabet_size);
		
		/*	Gets the index of the first non zero character in the (now sorted) list of frequencies (there are often quite a few characters with frequencies of zero,
			meaning the first however many characters in the list are often 0).
			This is used by the next block of code	*/
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

		/*	At a high level, the below block of code first creates a huffman tree structure by creating an object of type "huffman_tree_type"
			(whose constructor generates a tree in memory from the specified frequencies), then walks this tree in order to get the actual
			codewords themselves.	
			Unfortunately, (because I'm a big dumbus) the codewords are stored as objects of type std::vector<bool> (where each element
			represents a bit). I wasn't properly aware of the concept of manipluating binary data using bitwise operations when I wrote this.	*/
		/*	The overflowing vector stores a list of any codewords whose length exceeds the max (the deflate specification defines max lengths for codewords)	*/
		std::vector<unsigned short> overflowing;
		{
			/*	Creates huffman tree object	*/
			huffman_tree_type* huffman_tree = new huffman_tree_type(alphabet_freqs, alphabet_indices, nonzero_start, alphabet_size);
			/*	Ensures tree generation was successful, returns with error code if not	*/
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
			/*	Gets the huffman codewords by walking the tree	*/
			std::vector<huffman_tree_type::node_type*> node_stack;
			std::vector<int> bin_stack;
			/*	The node stack keeps track of the actual nodes themselves	*/
			node_stack.push_back(huffman_tree->tree);
			/*	The binary stack keeps track of the value of parent nodes (wether a node represents 0 or 1)	*/
			bin_stack.push_back(0);
			while (node_stack.size() > 0)
			{
				/*	Check wether current node is external or internal	*/
				if (node_stack.back()->extern_node)
				{
					/*	External node	*/
					bin_stack.pop_back();
					unsigned short index = ((huffman_tree_type::extern_node_type*)node_stack.back())->symbol;
					codeword_lengths[index] = bin_stack.size();

					/*	Gets codeword by looping through binary stack	*/
					for (short a = 0; a < codeword_lengths[index]; ++a)
					{
						codewords[index].push_back(bin_stack[a]);
					}

					/*	If codeword exceeds max length, add it to "overflowing"	*/
					if (codeword_lengths[index] > max_length)
					{
						overflowing.push_back(index);
					}
				}
				else
				{
					/*	Internal Node	*/
					/*	Checks if both child nodes have already been processed (see comment at bottom of while loop for why 2 denotes this)	*/
					if (bin_stack.back() < 2)
					{
						/*	If not, adds the first child of the current node to the node and binary stacks	*/
						node_stack.push_back(((huffman_tree_type::intern_node_type*)node_stack.back())->child_nodes[bin_stack.back()]);
						bin_stack.push_back(0);
						continue;
					}
					else
					{
						/*	Otherwise, pops the current node off of the binary stack	*/
						bin_stack.pop_back();
					}
				}

				/*	Removes node from the node stack, and switches to next child of the current parent node
				(this will be the the node processed in the next loop iteration). 
				If the current node is the second child node, then the last element in the binary stack will be incremented
				to 2, signifying to the above code in the next iteration that both childen have been processed.	*/
				node_stack.pop_back();
				if (bin_stack.size() > 0)
				{
					++bin_stack.back();
				}
			}
			delete huffman_tree;
		}

		/*	Rearranges tree (in an abstract sense atleast, the actual tree doesn't exist in memory anymore) to prevent overflowing (if any overflowing codewords exist)	*/
		unsigned short overflowing_size = overflowing.size();
		for (short a = (overflowing_size - 1); a >= 0; --a)
		{
			this->move_up_codeword(codewords, codeword_lengths, overflowing[a], alphabet_size, max_length);
			overflowing.pop_back();
		}

		/*	Ensures complete set.
			The above anti-overflow rearrangement can result in an incompomplete set of lengths, eg:
			  O
			 / \
			0   O
				 \
				  0
			The below code checks for any branches like this and moves the codeword up the tree if it encounters one,
			resulting in this:
			  O
			 / \
			0   0
			(Where 0 denotes an external node, and O denotes an internal node)
			*/
		{
			/*	First check if tree is incomplete (can skip if not)	*/
			if (this->check_if_complete_set(codeword_lengths, alphabet_size, max_length))
			{
				/*	If so, fixes tree	*/
				/*	This is done by iterating through every character, and for each, iterating through each parent node of the
					character'ss respective external node. Each parent node is then check agaist every other character's codeword
					in order to determine if said node has one or two children. If it is found that the parent node has only one
					child, then the child node of said parent node is removed, and the children of the removed node are made children of
					the parent node (this is the equivalent to just removing the child node's respective bit from the codewords of all
					characters whose codewwords contain it)	*/
				/*	The below for loop iterates through each character	*/
				for (unsigned short a = 0u; a < alphabet_size; ++a)
				{
					if (codeword_lengths[a] > 1u)
					{
						/*	The below for loop iterates through each of the character's parent nodes	*/
						for (short node_len = codeword_lengths[a] - 1; node_len > 0; --node_len)
						{
							/*	The below for loop iterates through each character in order to determine how many children the current
								parent node has	*/
							unsigned short matches[286] = {};
							matches[0] = a;
							unsigned short matches_size = 1u;
							bool last_match_child = codewords[a][node_len];
							for (unsigned short b = 0u; b < alphabet_size; ++b)
							{
								if ((b != a) && (codeword_lengths[b] > node_len))
								{
									/*	As mentioned above, the codewords are stored as vectors. The below for loop iterates through
										each bit and compares it to the current serchee codeword	*/
									for (unsigned short c = 0u; c < node_len; ++c)
									{
										if (codewords[a][c] != codewords[b][c])
										{
											goto adoesnt_match;
										}
									}
									matches[matches_size] = b;
									++matches_size;
									if (last_match_child != codewords[b][node_len])
									{
										goto is_full_node;
									}

								adoesnt_match:

									continue;
								}
							}
							if (matches_size == 1u)
							{
								/*	If the parent node only has one child, and that child happens to be the current character's respective node,
									then just remove the last bit from the character's codeword	*/
								codewords[a].pop_back();
								--codeword_lengths[a];
							}
							else
							{
								/*	If other codewords are grand children of the current parent node, but the parent node has only one child,
									then remove the parent nodes child node's respective bit from each character's codeword that contains it
									/ remove it from all the grand children	*/
								for (unsigned short b = 0u; b < matches_size; ++b)
								{
									codewords[matches[b]].erase(codewords[matches[b]].begin() + node_len);
									--codeword_lengths[matches[b]];
								}
							}

						is_full_node:

							continue;
						}
					}
				}
			}
		}
	}

	/*	Convert to canonical huffman codewords */
	{
		/*	Creates an array of vectores, each of which represent a codeword length,
			the below for loop adds the indices of each codeword to the vector
			of it's respective length, such that by the end, each vector will
			contain the indices of all codewords which are of it's specific length.
			This is used further down to help generate the canonical huffman codewords	*/
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

		/*	This chunk of code generates the canonical huffman codewords	*/
		unsigned long counter = (std::numeric_limits<unsigned long>::max)();
		/*	The below for loop iterates through each codeword length length	*/
		for (unsigned short a = 1u; a < 16u; ++a)
		{
			/*	The below for loop iterates through each codeword of the same length as the current length
				(if there are no codewords that are of of the current length, this loop will iterate zero times)	*/
			for (unsigned short b = 0u; b < bit_sorted_codewords[a].size(); ++b)
			{
				++counter;
				/*	Check wether the current codeword is the first or not	*/
				if (b == 0u)
				{
					/*	If the current codeword is the first in the vector, then "counter" is first bit shifted
					to the left by the difference betwee the current bit length and the bit length
					of the last codeword	*/

					/*	Finds the length of the last codeword (keep in mind that the last codewords length
					is not neccessarily equal to the current length - 1. One codeword's length may be, for instance,
					6, and the next codeword's length may be 8)	*/
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

				/*	Sets the current codeword to equal the current value of "counter"	*/
				std::bitset<16> bitset(counter);
				for (unsigned short c = 0u; c < codeword_lengths[bit_sorted_codewords[a][b]]; ++c)
				{
					/*	Inverts index (so that the bit order is reversed)	*/
					unsigned short bitset_index = (codeword_lengths[bit_sorted_codewords[a][b]] - 1u) - c;
					codewords[bit_sorted_codewords[a][b]][c] = bitset[bitset_index];
				}
			}
		}
	}

	/*	Uncomment print canonical Huffman codewords to console	*/
	/*
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
	*/
	
	/*	Checks if any prefixes exist within the canonical codewords	*/
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


/*	The primary member function of the "deflate_code_type" class. All other member functions
	are called directly or indirectly from this function	*/
int deflate_code_type::encode(const shared_type::byte_vec_type& message)
{
	/*	Calculate Adler32 checksum (this can be done here as the checksum represents the source data/ messag, not the code)	*/
	unsigned long adler32_checksum = this->calc_adler32(message);

	/*	Defines arrays to keep track of the frequency of symbols within the literal-length and distance alphabets	*/
	unsigned long lit_len_freqs[286] = {};
	unsigned long dist_freqs[30] = {};

	/*	"intermediate_code" stores the result of the LZSS compression before it is huffman encoded	*/
	std::vector<unsigned short> intermediate_code;

	/*	The below block of code performs LZSS encoding on the source data/ message	*/
	{
		/*	This implementation of LZSS does not maintain an actual buffer in memory (in order havign to
			repeatedly shift arrays). Instead the implementation just maintains indices that mark the begining
			and end of the buffers	*/
		unsigned long s_buffer_start = 0ul;
		unsigned long la_buffer_start = 1ul;
		unsigned long la_buffer_end = 0ul;
		const unsigned long message_size = message.char_vec.size();
		const unsigned long min_string_size = 3ul;
		if (message.char_vec.size() < this->max_lzss_length)
		{
			la_buffer_end = message.char_vec.size();
		}
		else
		{
			la_buffer_end = max_lzss_length;
		}

		/*	Push first byte to intermediate code	*/
		{
			/* ensures char is non negative to avoid overflow issues */
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

		/*	The below while loop iterates through the source data/ message	*/
		while (la_buffer_start < message_size)
		{
			unsigned long length = 1ul;
			unsigned long distance = 0ul;

			/*	The below for loop searches through the search buffer to find any characters equal to
				the current character	*/
			unsigned long match_index = la_buffer_start;
			for (long b = (la_buffer_start - 1); b >= (long)s_buffer_start; --b)
			{
				if (message.char_vec[b] == message.char_vec[la_buffer_start])
				{
					/*	Match found	*/
					match_index = b;
					goto search_for_additional_matches;
				}
			}
			/*	No match was found	*/
			goto write_pair_or_literal;

		search_for_additional_matches:

			/*	If this statement is reached, then a match was found. The below for loop searches up from the index of the found match
				in order to see if the match spans multiple characters (eg, if the characters "aCh" are in the look ahead buffer and
				"a" was found, the below code checks to see if the match is also succeeded by "Ch")	*/
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

			/*	As this is LZSS compresion, not LZ77, a length-distance pair is only written if the length of the found string
				exceeds the preset minimum (defined at the top of the enclosing block). This is done in order to prevent uneccisarily
				increasing the size of the output code, as there would be not point replacing a one byte literal 2 bytes (the size of
				a length-distance pair)	*/
			if (length < min_string_size)
			{
				/*	If the length was lower than the preset minimum, then simply write the character/ literal
					(Flow will also reach this point if no match was found, as the length remains 1 in such case)	*/
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
				/*	If the lenght exceeds the preset minimum, write a length-distance pair	*/
				/*	In DEFLATE, the length and distance are represented using a constant alphabet,
					(the literals are also represented by an alphabet, but their symbols line
					up with their ascii codes so there's no need to change them). Each symbol
					in both the distance alphabet, as well as the length portion of the literal
					length alphabet, represents a specific distanc or length respectively. Certain
					symbols are also succeeded by a predefined number of bits, which specifies
					the amount by which the value exceeds the value of the symbol, for instance
					if one symbol was 513, and the next was 769, but the value was 523, then the
					bits after the symbol would represent 10 (the number or bits for each
					symbol is predefined). See DEFLATE specification for more info.	*/

				unsigned short length_symbol = 0u;
				unsigned short length_extra = 0u;
				unsigned short distance_symbol = 0u;
				unsigned short distance_extra = 0u;

				/*	Finds which symbol represents the length, as well as the value of the extra bits (if needed)	*/
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

				/*	Finds which symbol represents the distance, as well as the value of the extra bits (if needed)	*/
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

				/*	Writes to intermediate code	*/
				{
					intermediate_code.push_back(length_symbol);
					intermediate_code.push_back(length_extra);
				}
				{
					intermediate_code.push_back(distance_symbol);
					intermediate_code.push_back(distance_extra);
				}
			}

			/*	Maintains search and look ahead buffer markers (slides buffers along message
				and clamps if buffer begins hitting end of message)	*/
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

	/*	Add stop codeword to end of code (the symbol 256 in the literal-len alphabet is the stop code)	*/
	intermediate_code.push_back(256);
	++lit_len_freqs[256];


	/*	Generates Huffman Trees	*/

	/*	Generates tree for literals and lengths	*/
	/*Keep in mind that only the alphabet symbols themselves are compressed with the huffman tree,
		not the extra bits (ie, the extra bits are not replaced with a huffman codeword). The same
		goes for the distance symbols	*/
	unsigned short lit_len_lengths[286] = {};
	std::vector<bool>* lit_len_codewords = new std::vector<bool>[286];
	{
		unsigned long lit_len_indices[286];
		for (unsigned short a = 0u; a < 286; ++a)
		{
			lit_len_indices[a] = a;
		}
		this->get_canonical_huffman_codewords(lit_len_codewords, lit_len_lengths, lit_len_freqs, lit_len_indices, 286ul, 15u);
	}

	/*	Generates tree for distances	*/
	unsigned short distance_lengths[30] = {};
	std::vector<bool>* distance_codewords = new std::vector<bool>[30];
	{
		unsigned long distance_indices[30];
		for (unsigned short a = 0u; a < 30; ++a)
		{
			distance_indices[a] = a;
		}
		this->get_canonical_huffman_codewords(distance_codewords, distance_lengths, dist_freqs, distance_indices, 30ul, 15u);
	}

	/*	Generates tree for the codeword lengths	(keep in mind that, as DEFLATE uses canonical huffman coding,
		only the lengths need to be stored (the codewords are derived from the lengths when decompressing).
		These lengths are themselves huffman encoded, which is what the below tree is for)	*/
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
		this->get_canonical_huffman_codewords(secondary_len_codewords, secondary_len_lengths, secondary_len_freqs, secondary_len_indices, 19ul, 7u);
	}


	/*	Writes final code	*/
	{
		/*	Writes Zlib header	*/

		{
			/*	CMF	*/
			{
				/*	Compresion method (deflate)	*/
				shared.feed_by_bit(this->code, 0u, 8ul, 4u);

				/*	Compression info (specifies a window size of max 32k)	*/
				shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 7ul, 4u);
			}

			/*	Flags	*/
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 30ul, 5u);
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ul, 1u);
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 1ul, 2u);
		}

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

		/*	Writes the actual compressed contents	*/
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

		/*	Writes Adler32 Checksum	*/
		{
			/*	Ensures that the current next bit index is byte aligned (the Adler32 checksum is byte aligned)	*/
			if (this->code.next_bit_index != 7u)
			{
				shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), 0ull, (7u - this->code.next_bit_index));
			}

			/*	Writes checksum	*/
			shared.feed_by_bit(this->code, (this->code.char_vec.size() - 1), adler32_checksum, 32u);
		}

		this->code.buffer_to_char_vec();
	}

	return 0;
}

/*	deflate_code_type constructor	*/
deflate_code_type::deflate_code_type(const shared_type::byte_vec_type& message)
{
	this->encode(message);
}


/*	huffman_tree_type constructor*/
deflate_code_type::huffman_tree_type::huffman_tree_type(const unsigned long* freqs, const unsigned long* indices, const unsigned long start, const unsigned long size)
{
	this->freqs = freqs;
	this->indices = indices;
	this->freqs_start = start;
	this->freqs_next = size;

	/*	The below while loop generates the huffman tree by repeatedly calling "join" until the tree is completed
		("join" joins 2 elements in the queue)	*/
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


/*	Joins 2 elements in the queue	*/
int deflate_code_type::huffman_tree_type::join()
{
	/*	This function creates an actual tree structure in memory (where each node
		is an object that contnains pointers to it's children). Each node is stored
		in an array for ease of book keeping (as opposed to just having them
		floating in memory). Also note the structure of the queue: the queue,
		as an abstract concept, represents the list of both the raw frequencies,
		as well as the weights of the nodes that have been formed as a result
		of merging. The frequencies and nodes are kept in seperated arrays,
		but figuratively represent a single queue	*/

	/*	First checks if the tree is already complete (returns 1 if so)	*/
	unsigned short node_roots_size = this->node_roots.size();
	if (((this->freqs_next - this->freqs_start) + node_roots_size) >= 2)
	{
		/*	If not yet complete:	*/

		/*	Stores the indices of the queue elements to be joined	*/
		dual_index_type entries_to_join[2];

		/*	Stores the pointers to the queue elements to be joined	(once a raw frequency is joined,
			it is represented by an external node object (this is pointed to be it's parent node,
			which will be created during the joining process))	*/
		node_type* child_nodes[2];

		/*	The below for loop iterates twice, in order to find which 2 elements in the queue to join	*/
		unsigned short freq_index_offset = 0u;
		for (unsigned short a = 0u; a < 2u; ++a)
		{
			/*	Initialy set the current element to join to the next raw frequency*/
			entries_to_join[a] = dual_index_type((freqs_start + freq_index_offset), false);

			/*	The below for loop iterates through the array of node roots (this array represents all root nodes
				currently sitting in the figurative queue) in order to determine if the current element to join
				should be replaced by a node (if so, then the current value of "entris_to_join[a]" is overwritten
				with the value of a node)	*/
			for (unsigned short b = 0u; b < node_roots_size; ++b)
			{
				/*	Ensures the frequency index is actually valid (if the raw frequency index is >= the total
					number of raw frequencies, then the frequency doesn't actually exist, the end of the list
					of raw frequencies has already been reached (there are not more raw frequencies in the
					figurative queue)). If the frequency is not valid, then figurative queue contains only
					root nodes, so set the value of "entries_to_join[a]" to the index of the current root node
					(ie whichever root node the for loop iterates through first)	*/
				if (entries_to_join[a].index >= this->freqs_next)
				{
					goto use_root_node_if_not_repeat;
				}

				/*	If the current raw frequency is valid, then check if the current root node'ss weight is less
					than the current raw frequency	*/
				if (node_roots[b]->weight < freqs[entries_to_join[a].index])
				{
				use_root_node_if_not_repeat:

					/*	Check if current root node has already been listed for merge (if a == 1)	*/
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

					/*	If not, then mark the current node as the element to join	*/
					entries_to_join[a] = dual_index_type(b, true);
					break;
				}
			}

			/*	Checks if the current entry to join is a root node	*/
			if (entries_to_join[a].array)
			{
				/*	If so, then set the current child node entry to point to equal the root node	*/
				child_nodes[a] = this->node_roots[entries_to_join[a].index];
			}
			else
			{
				/*	Otherwise create an external node that represents the raw frequency, and set the child node
					entry to point to that	*/
				this->extern_nodes[this->extern_nodes_size].symbol = this->indices[entries_to_join[a].index];
				this->extern_nodes[this->extern_nodes_size].weight = this->freqs[entries_to_join[a].index];
				child_nodes[a] = &this->extern_nodes[this->extern_nodes_size];
				++this->extern_nodes_size;
				++freq_index_offset;
			}
		}

		/*	Creates a parent node and attaches the 2 child nodes to it, and maintains the figurative queue
			(maintains the 2 arrays that represent it)	*/
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