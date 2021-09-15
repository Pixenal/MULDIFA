#include "../df_common/deflate.h"
#include <vector>
#include <cmath>

inline shared_type shared;

int deflate_code_type::quick_sort(unsigned long* array, unsigned long* indices, const unsigned short array_size)
{
	/*	Padded as first and last are out of bounds (first = -1 and last = array_size)	*/
	const unsigned short array_size_padded = array_size + 2u;

	/*	Is for visualization	*/
	/*unsigned long highest_freq = 0u;
	for (short a = 0; a < array_size; ++a)
	{
		if (array[a] > highest_freq)
		{
			highest_freq = array[a];
		}
	}*/
	std::vector<short> pivots;
	pivots.push_back(-1);
	pivots.push_back(array_size);
	bool another_pass = true;

	/*	Visualization	*/
	/*for (short b = 20; b >= 0; --b)
	{
		unsigned long level = (highest_freq / 20ul) * (unsigned long)b;
		for (short c = 0; c < array_size; c += 1)
		{
			if ((array[c] >= level) || (level == 0))
			{
				std::cout << "#";
			}
			else
			{
				std::cout << " ";
			}
		}
		std::cout << std::endl;
	}*/
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

				/*	Visualization	*/
				
				/*for (short b = 20; b >= 0; --b)
				{
					unsigned long level = (highest_freq / 20ul) * (unsigned long)b;
					for (short c = 0; c < array_size; c += 1)
					{
						if ((array[c] >= level) || (level == 0))
						{
							std::cout << "#";
						}
						else
						{
							std::cout << " ";
						}
					}
					std::cout << std::endl;
				}*/
			}
		}
	}

	return 0;
}


int deflate_code_type::get_canonical_huffman_codewords(std::vector<bool>* codewords, unsigned short* codeword_lengths, unsigned long* alphabet_freqs, unsigned long* alphabet_indices, const unsigned long alphabet_size)
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
					if (codeword_lengths[index] > 15u)
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

		unsigned short overflowing_size = overflowing.size();
		for (short a = (overflowing_size - 1); a >= 0; --a)
		{
			unsigned short margin = 0u;
			while (true)
			{
				++margin;
				for (unsigned short b = 0u; b < alphabet_size; ++b)
				{
					if (codeword_lengths[b] == (15u - margin))
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
	}

	/*std::cout << std::endl;
	std::cout << std::endl;

	for (short a = 0; a < alphabet_size; ++a)
	{
		std::cout << a << " : " << codeword_lengths[a] << " : ";
		for (short b = 0; b < codewords[a].size(); ++b)
		{
			std::cout << codewords[a][b];
		}
		std::cout << std::endl;
	}*/

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
	/*	Defines arrays to keep track of the frequency of symbols within the literal-length and distance alphabets	*/
	unsigned long lit_len_freqs[288] = {};
	unsigned long dist_freqs[32] = {};

	std::vector<unsigned short> intermediate_code;
	//std::vector<char> intermediate_code_sizes;

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
		intermediate_code.push_back(message.char_vec[0]);
		//intermediate_code_sizes.push_back(8);
		++lit_len_freqs[message.char_vec[0]];

		while (la_buffer_start < message_size)
		{
			unsigned long length = 1ul;
			unsigned long distance = 0ul;
			unsigned long match_index = la_buffer_start;
			for (long b = (la_buffer_start - 1); b >= (long)s_buffer_start; --b)
			{
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
					unsigned short non_negative = (128 + message.char_vec[b]) + 128;
					intermediate_code.push_back(non_negative);
					//intermediate_code_sizes.push_back(8);
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
					/*shared_type::byte_vec_type length_byte_vec;
					shared.feed_by_bit(length_byte_vec, 0, length_symbol, 9);
					intermediate_code.push_back((short)length_byte_vec.char_vec[0]);
					unsigned short length_extra_bits = 0u;
					if (length_extra > 0u)
					{
						length_extra_bits = std::log2(length_extra) + 1u;
						shared.feed_by_bit(length_byte_vec, (length_byte_vec.char_vec.size() - 1), length_symbol, length_extra_bits);
					}*/
					//length_byte_vec.buffer_to_char_vec();
					intermediate_code.push_back(length_symbol);
					intermediate_code.push_back(length_extra);
					//intermediate_code_sizes.push_back(9u + length_extra_bits);
				}

				{
					/*shared_type::byte_vec_type distance_byte_vec;
					shared.feed_by_bit(distance_byte_vec, 0, distance_symbol, 5);
					unsigned short distance_extra_bits = 0u;
					if (distance_extra > 0u)
					{
						distance_extra_bits = std::log2(distance_extra) + 1u;
						shared.feed_by_bit(distance_byte_vec, (distance_byte_vec.char_vec.size() - 1), distance_symbol, distance_extra_bits);
					}*/
					intermediate_code.push_back(distance_symbol);
					intermediate_code.push_back(distance_extra);
					/*distance_byte_vec.buffer_to_char_vec();
					for (unsigned short b = 0u; b < 3; ++b)
					{
						if (b < distance_byte_vec.char_vec.size())
						{
							intermediate_code.push_back((short)distance_byte_vec.char_vec[b]);
						}
						else
						{
							intermediate_code.push_back(0u);
						}
					}*/
					//intermediate_code_sizes.push_back(9u + distance_extra_bits);
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
	//intermediate_code_sizes.push_back(9);
	++lit_len_freqs[256];

	/*	Generate Huffman Trees	*/

	unsigned short lit_len_lengths[288] = {};
	std::vector<bool>* lit_len_codewords = new std::vector<bool>[288];
	unsigned long lit_len_indices[288];
	for (unsigned short a = 0u; a < 288; ++a)
	{
		lit_len_indices[a] = a;
	}
	std::cout << "Lit Len" << std::endl;
	this->get_canonical_huffman_codewords(lit_len_codewords, lit_len_lengths, lit_len_freqs, lit_len_indices, 288ul);

	unsigned short distance_lengths[32] = {};
	std::vector<bool>* distance_codewords = new std::vector<bool>[32];
	unsigned long distance_indices[32];
	for (unsigned short a = 0u; a < 32; ++a)
	{
		distance_indices[a] = a;
	}
	std::cout << "Distance" << std::endl;
	this->get_canonical_huffman_codewords(distance_codewords, distance_lengths, dist_freqs, distance_indices, 32ul);

	unsigned short secondary_len_lengths[19] = {};
	std::vector<bool>* secondary_len_codewords = new std::vector<bool>[19];
	unsigned long secondary_len_freqs[19] = {};
	for (unsigned short a = 0u; a < 288; ++a)
	{
		++secondary_len_freqs[lit_len_lengths[a]];
	}
	for (unsigned short a = 0u; a < 32; ++a)
	{
		++secondary_len_freqs[distance_lengths[a]];
	}
	unsigned long secondary_len_indices[19];
	for (unsigned short a = 0u; a < 19u; ++a)
	{
		secondary_len_indices[a] = a;
	}
	std::cout << "Secondary" << std::endl;
	this->get_canonical_huffman_codewords(secondary_len_codewords, secondary_len_lengths, secondary_len_freqs, secondary_len_indices, 19ul);

	shared_type::byte_vec_type contents_code;
	int	context_flag = 0;
	unsigned long intermediate_code_size = intermediate_code.size();
	for (unsigned long a = 0u; a < intermediate_code_size; ++a)
	{
		if (context_flag == 0u)
		{

			std::bitset<16> bitset;
			for (unsigned short b = 0u; b < lit_len_lengths[intermediate_code[a]]; ++a)
			{
				bitset[b] = lit_len_codewords[intermediate_code[a]][b];
			}
			unsigned long byte_index = contents_code.char_vec.size();
			if (byte_index > 0u)
			{
				--byte_index;
			}
			shared.feed_by_bit(contents_code, byte_index, bitset.to_ulong(), 9);
			if (intermediate_code[a] > 256)
			{
				++a;
				shared.feed_by_bit(contents_code, (contents_code.char_vec.size() - 1), intermediate_code[a], this->len_extra_bits[intermediate_code[a - 1u] - 257u]);
				context_flag = 1u;
			}
		}
		else if (context_flag == 1u)
		{
			std::bitset<16> bitset;
			for (unsigned short b = 0u; b < distance_lengths[intermediate_code[a]]; ++a)
			{
				bitset[b] = distance_codewords[intermediate_code[a]][b];
			}
			shared.feed_by_bit(contents_code, (contents_code.char_vec.size() - 1), bitset.to_ulong(), 5);
			++a;
			shared.feed_by_bit(contents_code, (contents_code.char_vec.size() - 1), intermediate_code[a], this->dist_extra_bits[intermediate_code[a - 1u]]);
			context_flag = 0u;
		}
	}

	return 1;
	/*Only the alphabet symbols themselves are compressed with the huffman tree, not the extra bits (ie the extra bits are not replaced with a huffman codeword)	*/
	/*	Remember to read section about lexicographic rules	*/
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
	this->tree = (intern_node_type*)this->node_roots[0];
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