#include "../df_common/deflate.h"
#include <vector>
#include <cmath>


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


int deflate_code_type::create_canonical_huffman(std::vector<char>& tree, const unsigned long* alphabet_freqs, const unsigned long alphabet_size)
{
	
	return 0;
}


int deflate_code_type::encode(const shared_type::byte_vec_type& message)
{
	shared_type shared_obj;

	/*	Defines arrays to keep track of the frequency of symbols within the literal-length and distance alphabets	*/
	unsigned long lit_len_freqs[286] = {};
	unsigned long dist_freqs[30] = {};

	std::vector<char> intermediate_code;
	std::vector<char> intermediate_code_sizes;

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
	intermediate_code_sizes.push_back(8);
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
				intermediate_code.push_back(message.char_vec[b]);
				intermediate_code_sizes.push_back(8);
				++lit_len_freqs[message.char_vec[b]];
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
				shared_type::byte_vec_type length_byte_vec;
				shared_obj.feed_by_bit(length_byte_vec, 0, length_symbol, 9);
				intermediate_code.push_back((short)length_byte_vec.char_vec[0]);
				unsigned short length_extra_bits = 0u;
				if (length_extra > 0u)
				{
					length_extra_bits = std::log2(length_extra) + 1u;
					shared_obj.feed_by_bit(length_byte_vec, (length_byte_vec.char_vec.size() - 1), length_symbol, length_extra_bits);
				}
				length_byte_vec.buffer_to_char_vec();
				intermediate_code.push_back((short)length_byte_vec.char_vec[1]);
				intermediate_code_sizes.push_back(9u + length_extra_bits);
			}

			{
				shared_type::byte_vec_type distance_byte_vec;
				shared_obj.feed_by_bit(distance_byte_vec, 0, distance_symbol, 5);
				unsigned short distance_extra_bits = 0u;
				if (distance_extra > 0u)
				{
					distance_extra_bits = std::log2(distance_extra) + 1u;
					shared_obj.feed_by_bit(distance_byte_vec, (distance_byte_vec.char_vec.size() - 1), distance_symbol, distance_extra_bits);
				}
				distance_byte_vec.buffer_to_char_vec();
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
				}
				intermediate_code_sizes.push_back(9u + distance_extra_bits);
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

	/*	Add stop codeword to end of code	*/
	intermediate_code.push_back((char)0);
	intermediate_code.push_back((char)1);
	intermediate_code_sizes.push_back(9);
	++lit_len_freqs[256];

	shared_type::byte_vec_type contents_code;

	/*	Generate Huffman Trees	*/
	{
		/*unsigned long test_freqs[10];
		unsigned long test_indices[10];
		for (unsigned short a = 0u; a < 10; ++a)
		{
			test_indices[a] = a;
			test_freqs[a] = rand() % 200;
		}
		this->quick_sort(test_freqs, test_indices, 10);
		for (unsigned short a = 0u; a < 10; ++a)
		{
			std::cout << "PreProcess: " << a << " : " << test_freqs[a] << std::endl;
		}
		huffman_tree_type huffman_tree(test_freqs, test_indices, 0, 10);*/

		unsigned long lit_len_indices[286];
		for (unsigned short a = 0u; a < 286; ++a)
		{
			lit_len_indices[a] = a;
			//lit_len_freqs[a] = rand();
		}
		unsigned short nonzero_start = 0u;
		this->quick_sort(lit_len_freqs, lit_len_indices, 286);
		for (unsigned short a = 0u; a < 286; ++a)
		{
			if (lit_len_freqs[a] > 0ul)
			{
				break;
			}
			else
			{
				++nonzero_start;
			}
		}
		huffman_tree_type huffman_tree(lit_len_freqs, lit_len_indices, nonzero_start, 286);

		{
			std::cout << "start" << std::endl;
			unsigned short codeword_lengths[286];
			std::vector<bool> codewords[286];
			std::vector<unsigned short> overflowing;
			std::vector<huffman_tree_type::node_type*> node_stack;
			std::vector<int> bin_stack;
			node_stack.push_back(huffman_tree.tree);
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
						std::cout << bin_stack[a];
						codewords[index].push_back(bin_stack[a]);
					}
					if (codeword_lengths[index] > 15u)
					{
						overflowing.push_back(index);
					}
					std::cout << std::endl;
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

			unsigned short overflowing_size = overflowing.size();
			for (short a = (overflowing_size - 1); a >= 0; --a)
			{
				unsigned short margin = 0u;
				while (true)
				{
					++margin;
					for (unsigned short b = 0u; b < 286; ++b)
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
					/*	Retry with larger margin	*/
				}
			moved_up_tree:

				overflowing.pop_back();
			}

			std::cout << std::endl;
			std::cout << std::endl;

			for (short a = 0; a < 286; ++a)
			{
				std::cout << a << " : " << codeword_lengths[a] << " : ";
				for (short b = 0; b < codewords[a].size(); ++b)
				{
					std::cout << codewords[a][b];
				}
				std::cout << std::endl;
			}
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