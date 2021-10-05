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


#include "../df_common/df_lib.h"


/*shared_type*/
/*-------------------------------------------------------------------------------------------------------------*/


shared_type::coord_uvw_type shared_type::cartesian_to_barycentric(const shared_type::coord_xyz_type& tri_vertex_0, const shared_type::coord_xyz_type& tri_vertex_1, const shared_type::coord_xyz_type& tri_vertex_2, const shared_type::coord_xyz_type& point, const shared_type::coord_xyz_type& tri_normal)
{
	shared_type::coord_uvw_type point_bc;
	double dert_a = 0;
	double dert_au = 0;
	double dert_av = 0;

	/*Performes cramers rule on system matrix A*/

	/*Must be done differently depending on the normal of the tri*/

	if ((tri_normal.z == 1.0) || (tri_normal.z == -1.0))
	{
		dert_a = (tri_vertex_0.x * tri_vertex_1.y) - (tri_vertex_0.x * tri_vertex_2.y) -
			(tri_vertex_1.x * tri_vertex_0.y) + (tri_vertex_1.x * tri_vertex_2.y) +
			(tri_vertex_2.x * tri_vertex_0.y) - (tri_vertex_2.x * tri_vertex_1.y);

		/*Get determinate of Au*/
		dert_au = (point.x * tri_vertex_1.y) - (point.x * tri_vertex_2.y) -
			(tri_vertex_1.x * point.y) + (tri_vertex_1.x * tri_vertex_2.y) +
			(tri_vertex_2.x * point.y) - (tri_vertex_2.x * tri_vertex_1.y);

		/*Get determinate of Av*/
		dert_av = (tri_vertex_0.x * point.y) - (tri_vertex_0.x * tri_vertex_2.y) -
			(point.x * tri_vertex_0.y) + (point.x * tri_vertex_2.y) +
			(tri_vertex_2.x * tri_vertex_0.y) - (tri_vertex_2.x * point.y);
	}
	else if ((tri_normal.y == 1.0) || (tri_normal.y == -1.0) || (tri_normal.x == .0f))
	{
		dert_a = (tri_vertex_0.x * tri_vertex_1.z) - (tri_vertex_0.x * tri_vertex_2.z) -
			(tri_vertex_1.x * tri_vertex_0.z) + (tri_vertex_1.x * tri_vertex_2.z) +
			(tri_vertex_2.x * tri_vertex_0.z) - (tri_vertex_2.x * tri_vertex_1.z);

		/*Get determinate of Au*/
		dert_au = (point.x * tri_vertex_1.z) - (point.x * tri_vertex_2.z) -
			(tri_vertex_1.x * point.z) + (tri_vertex_1.x * tri_vertex_2.z) +
			(tri_vertex_2.x * point.z) - (tri_vertex_2.x * tri_vertex_1.z);

		/*Get determinate of Av*/
		dert_av = (tri_vertex_0.x * point.z) - (tri_vertex_0.x * tri_vertex_2.z) -
			(point.x * tri_vertex_0.z) + (point.x * tri_vertex_2.z) +
			(tri_vertex_2.x * tri_vertex_0.z) - (tri_vertex_2.x * point.z);
	}
	else
	{
		dert_a = (tri_vertex_0.y * tri_vertex_1.z) - (tri_vertex_0.y * tri_vertex_2.z) -
			(tri_vertex_1.y * tri_vertex_0.z) + (tri_vertex_1.y * tri_vertex_2.z) +
			(tri_vertex_2.y * tri_vertex_0.z) - (tri_vertex_2.y * tri_vertex_1.z);

		/*Get determinate of Au*/
		dert_au = (point.y * tri_vertex_1.z) - (point.y * tri_vertex_2.z) -
			(tri_vertex_1.y * point.z) + (tri_vertex_1.y * tri_vertex_2.z) +
			(tri_vertex_2.y * point.z) - (tri_vertex_2.y * tri_vertex_1.z);

		/*Get determinate of Av*/
		dert_av = (tri_vertex_0.y * point.z) - (tri_vertex_0.y * tri_vertex_2.z) -
			(point.y * tri_vertex_0.z) + (point.y * tri_vertex_2.z) +
			(tri_vertex_2.y * tri_vertex_0.z) - (tri_vertex_2.y * point.z);
	}

		//dert_a = 1;
		//dert_au = 1;
		//dert_av = 1;
	

	/*u = dert(Au) / dert(A)*/
	point_bc.u = dert_au / dert_a;

	/*u = dert(Av) / dert(A)*/
	point_bc.v = dert_av / dert_a;

	/*w can be derived from u and v*/
	point_bc.w = 1 - point_bc.u - point_bc.v;

	return point_bc;
}


shared_type::coord_xyz_type shared_type::barycentric_to_cartesian(const shared_type::coord_xyz_type& tri_vertex_0, const shared_type::coord_xyz_type& tri_vertex_1, const shared_type::coord_xyz_type& tri_vertex_2, const shared_type::coord_uvw_type& point)
{
	shared_type::coord_xyz_type point_cart;
	point_cart.x = (point.u * tri_vertex_0.x) + (point.v * tri_vertex_1.x) + (point.w * tri_vertex_2.x);
	point_cart.y = (point.u * tri_vertex_0.y) + (point.v * tri_vertex_1.y) + (point.w * tri_vertex_2.y);
	point_cart.z = (point.u * tri_vertex_0.z) + (point.v * tri_vertex_1.z) + (point.w * tri_vertex_2.z);
	return point_cart;
}


double shared_type::lerp(const double& value_0, const double& value_1, const double& lerp_alpha)
{
	double lerp_alpha_cpy;

	if (value_1 > value_0)
	{
		double value_difference = value_1 - value_0;

		return value_0 + (value_difference * lerp_alpha);
	}
	else
	{
		lerp_alpha_cpy = 1.0 - lerp_alpha;

		double value_difference = value_0 - value_1;

		return value_1 + (value_difference * lerp_alpha_cpy);
	}
}


shared_type::coord_xyz_type shared_type::get_dist_vector(const coord_xyz_type& point_0, const coord_xyz_type& point_1)
{
	coord_xyz_type dist;

	dist.x = point_1.x - point_0.x;
	dist.x = point_1.y - point_0.y;
	dist.x = point_1.z - point_0.z;

	return dist;
}


shared_type::coord_xy_type shared_type::get_dist_vector(const coord_xy_type& point_0, const coord_xy_type& point_1)
{
	coord_xy_type dist;

	dist.x = point_1.x - point_0.x;
	dist.x = point_1.y - point_0.y;

	return dist;
}


double shared_type::pythag(const coord_xyz_type& set)
{
	double distance = (set.x * set.x) + (set.y * set.y) + (set.z * set.z);
	return sqrt(distance);
}


double shared_type::pythag(const coord_xy_type& set)
{
	double distance = (set.x * set.x) + (set.y * set.y);
	return sqrt(distance);
}


/*shared_type::coord_xyz_type*/
/*-------------------------------------------------------------------------------------------------------------*/


shared_type::coord_xyz_type::coord_xyz_type()
{
}


shared_type::coord_xyz_type::coord_xyz_type(const double x, const double y, const double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


bool shared_type::coord_xyz_type::operator==(const coord_xyz_type& operand)
{
	return ((this->x == operand.x) && (this->y == operand.y) && (this->z == operand.z));
}


bool shared_type::coord_xyz_type::operator==(coord_xyz_type& operand)
{
	return ((this->x == operand.x) && (this->y == operand.y) && (this->z == operand.z));
}


void shared_type::coord_xyz_type::clean()
{
	x = .0;
	y = .0;
	z = .0;
}


/*shared_type::coord_xy_type*/
/*-------------------------------------------------------------------------------------------------------------*/


shared_type::coord_xy_type::coord_xy_type()
{
}


shared_type::coord_xy_type::coord_xy_type(const double x, const double y)
{
	this->x = x;
	this->y = y;
}


void shared_type::coord_xy_type::clean()
{
	x = .0;
	y = .0;
}


/*shared_type::coord_uvw_type*/
/*-------------------------------------------------------------------------------------------------------------*/


shared_type::coord_uvw_type::coord_uvw_type()
{
}


shared_type::coord_uvw_type::coord_uvw_type(const double u, const double v, const double w)
{
	this->u = u;
	this->v = v;
	this->w = w;
}


void shared_type::coord_uvw_type::clean()
{
	u = .0;
	v = .0;
	w = .0;
}


bool shared_type::coord_uvw_type::operator==(const coord_uvw_type& operand)
{
	return (this->u == operand.u) && (this->v == operand.v) && (this->w == operand.w);
}


bool shared_type::coord_uvw_type::operator!=(const coord_uvw_type& operand)
{
	return !(*this == operand);
}


/*shared_type::vert_info_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::vert_info_type::clean()
{
	coord.clean();
	value = .0f;
}


/*shared_type::tri_info_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::tri_info_type::clean()
{
	vert_0 = 0ul;
	vert_1 = 0ul;
	vert_2 = 0ul;
	normal.clean();
	d = .0;
}


/*shared_type::tri_uv_info_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::tri_uv_info_type::clean()
{
	uv_vert_0.clean();
	uv_vert_1.clean();
	uv_vert_2.clean();
}


bool shared_type::tri_uv_info_type::operator==(const tri_uv_info_type& operand)
{
	return ((this->uv_vert_0 == operand.uv_vert_0) && (this->uv_vert_1 == operand.uv_vert_1) && (this->uv_vert_2 == operand.uv_vert_2));
}


bool shared_type::tri_uv_info_type::operator!=(const tri_uv_info_type& operand)
{
	return !(*this == operand);
}


/*shared_type::loop_info_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::loop_info_type::clean()
{
	vert_index = 0;
}


/*shared_type::vert_tri_ref_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::vert_tri_ref_type::clean()
{
	dfc_index = 0;
	coord.clean();
	tris = nullptr;
}


/*shared_type::scale_xyz_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::scale_xyz_type::clean()
{
	x = .0;
	y = .0;
	z = .0;
}


/*shared_type::index_xyw_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::index_xyzw_type::clean()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}


/*shared_type::index_xyz_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::index_xyz_type::clean()
{
	x = 0;
	y = 0;
	z = 0;
}


/*shared_type::index_xy_type*/
/*-------------------------------------------------------------------------------------------------------------*/


shared_type::index_xy_type::index_xy_type()
{
}


shared_type::index_xy_type::index_xy_type(const unsigned short x, const unsigned short y)
{
	this->x = x;
	this->y = y;
}


bool shared_type::index_xy_type::operator==(const index_xy_type& operand)
{
	return ((this->x == operand.x) && (this->y == operand.y));
}


bool shared_type::index_xy_type::operator!=(const index_xy_type& operand)
{
	return !(*this == operand);
}


void shared_type::index_xy_type::clean()
{
	x = 0;
	y = 0;
}


/*shared_type::index_xyz_signed_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::index_xyz_signed_type::clean()
{
	x = 0;
	y = 0;
	z = 0;
}



/*shared_type::index_xy_signed_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::index_xy_signed_type::clean()
{
	x = 0;
	y = 0;
}


/*shared_type::amount_xyz_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::amount_xyz_type::clean()
{
	x = 0;
	y = 0;
	z = 0;
}


/*shared_type::amount_xy_type*/
/*-------------------------------------------------------------------------------------------------------------*/


void shared_type::amount_xy_type::clean()
{
	x = 0;
	y = 0;
}


/*shared_type::invrse_jenga_type*/
/*-------------------------------------------------------------------------------------------------------------*/

/*Member function definitions are in seperate implementation file as invrse_jenga_type is a class template*/


void shared_type::invert_bit_order(unsigned long long& number)
{
	std::bitset<64> buffer(number);
	std::bitset<64> inverted_buffer;

	for (unsigned short a = 0u; a < 64u; ++a)
	{
		unsigned short inverted_index = 63u - a;
		inverted_buffer[inverted_index] = (int)buffer[a];
	}

	number = inverted_buffer.to_ullong();
}


void shared_type::invert_bit_order(unsigned long& number)
{
	std::bitset<32> buffer(number);
	std::bitset<32> inverted_buffer;

	for (unsigned short a = 0u; a < 32u; ++a)
	{
		unsigned short inverted_index = 31u - a;
		inverted_buffer[inverted_index] = (int)buffer[a];
	}

	number = inverted_buffer.to_ulong();
}


/*	Serializes the specified unsigned long long, writing to the specified byte vector.
	If param "bit_size" == 0, the number is serialized into the sv format, which consists
	of 2 components, size and value, the size component is always 6 bits long, and represents
	the length of the value component in bits. The value component, as the name suggests,
	stores the actual value (the value's bit size is reduced down to the minimum number of
	bits needed to represent it, hence the need for the size component).
	If param "bit_size > 0, then the function will simply serialize the above mentioned value
	component, with the component's bit length equal to "bit_size" (no size component is made)"
	Note that this function is ultimately called for all datatypes (all other "feed_by_bit" overloads'
	just convert from the  specified values original type (including floating point types) to an
	unsigned long long before calling this function)	*/
void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, unsigned long long byte_index, const unsigned long long number, unsigned short bit_size)
{
	if (!((byte_vec.next_bit_index == 0u) && (byte_index == 0u)))
	{
		++byte_vec.next_bit_index;
	}
	if (byte_vec.next_bit_index >= 8u)
	{
		byte_vec.incrmt();
		++byte_index;
	}
	byte_vec.next_bit_index %= 8u;

	unsigned short return_bit_index = 0u;
	unsigned short buffer_bit_counter = 0u;
	unsigned short bit_size_meta = bit_size;
	std::bitset<70> number_byte_arr_buffer(number);
	/*	If bit_size == 0, gets minimum number of bits needed to represent the spencified value	*/
	if (bit_size == 0u)
	{
		unsigned short optimal_bit_size = std::log2(number) + 1u;
		/*	"sv_meta_size" is a const data member of "shared_type" which defines the set length
			the size component (is currently set to 6)	*/
		/*	"bit_size_meta" stores the total bits to be written (including both the size and value components)	*/
		bit_size_meta = sv_meta_size + optimal_bit_size;
		std::bitset<8> meta_byte_arr_buffer(optimal_bit_size);
		number_byte_arr_buffer <<= sv_meta_size;
		for (unsigned short a = 0u; a < sv_meta_size; ++a)
		{
			number_byte_arr_buffer[a] = meta_byte_arr_buffer[a];
		}
	}

	/*	"byte_size_planted" stores the total number of bytes that the sv will occupy in the specified byte vec
		(note that this does not simply represent the bit length of the data converted to bytes, but rather
		how many bytes within the byte vector the data will span. Keep in mind that this function is often
		called multiple times for the same byte vector (multiple numbers packed into the same byte string), 
		and so the bytes of the data doesn not neccessarily align with the bytes of the byte vec)	*/
	unsigned short byte_size_planted = ((((byte_vec.next_bit_index) + bit_size_meta) - 1u) / 8u) + 1u;

	/*	Iterates through each spanned byte	(see comment above "byte_size_planted" for more info)	*/
	for (unsigned short a = 0u; a < byte_size_planted; ++a)
	{	
		const unsigned long long byte_offset_index = byte_index + a;

		if ((byte_vec.char_vec.size()) <= byte_offset_index)
		{
			byte_vec.incrmt();
		}

		unsigned short b;
		if (a == 0u)
		{
			b = byte_vec.next_bit_index;
		}
		else
		{
			b = 0u;
		}
		
		for (; (b < 8u) && (buffer_bit_counter < bit_size_meta); ++b)
		{
			/*	Writes bit	*/
			byte_vec.byte_buffer[b] = (int)number_byte_arr_buffer[buffer_bit_counter];
			++buffer_bit_counter;

			/*	This is set every bit that once all the data is written, the last value of b can be known
				(iirc using an external object for b instead of one local to the loop caused issues)	*/
			return_bit_index = b;
		}
	}

	byte_vec.next_bit_index = return_bit_index;
}


/*	Serializes specified value (using sv format if "bit_size" == 0, or with a fixed size if > 0)	*/
void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const unsigned long number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	unsigned long long number_buffer = number;
	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}


void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const unsigned short number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	unsigned long long number_buffer = number;
	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}


void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const double number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	unsigned long long number_buffer = 0u;
	std::memcpy(&number_buffer, &number, 8);
	invert_bit_order(number_buffer);

	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}


void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const float number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	unsigned long number_buffer = 0u;
	std::memcpy(&number_buffer, &number, 4);
	invert_bit_order(number_buffer);

	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}


void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const bool number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	unsigned long long number_buffer = number;
	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}

void shared_type::feed_by_bit(shared_type::byte_vec_type& byte_vec, const unsigned long long byte_index, const int number, unsigned short bit_size)
{
	/*	Converts value to an unsigned long long and calls ull overload	*/
	int test = 0u;
	unsigned long long number_buffer = 0u;
	std::memcpy(&number_buffer, &number, 4);

	std::memcpy(&test, &number_buffer, 4);

	feed_by_bit(byte_vec, byte_index, number_buffer, bit_size);
}


/*	Writes a single byte to the specified file	*/
void shared_type::write_byte_vec(shared_type::byte_vec_type& byte_vec, std::ofstream& file, const bool& byte_endianness)
{
	/*	Loops through and writes each byte in "byte_vec"	*/

	unsigned long long size = byte_vec.char_vec.size();
	for (unsigned long long a = 0u; a < size; ++a)
	{
		unsigned long long byte_index;
		if (byte_endianness == true)
		{
			byte_index = (size - a) - 1u;
		}
		else
		{
			byte_index = a;
		}

		file.put(byte_vec.char_vec[byte_index]);
	}
}


bool shared_type::write_id_type::operator==(const write_id_type& write_id)
{
	return (write_id.index == this->index) && (write_id.rand == this->rand);
}


/*	shared_type::ncspline_type	*/
/*-------------------------------------------------------------------------------------------------------------*/


/*	Creates spline, ie, computes and/ or stores unknowns neccessary to sample spline	*/
void shared_type::ncspline_type::init_spline(const double* knott_values, const unsigned short knott_values_nxt_indx, const index_xy_type& spline_prpndclr_coord)
{
	/*	The main computation done in this function is calculating the double derivative at each knott. At a high level, this is done
		through solving a system of linear equations, where each equation represents an internal knott, and its respective unknown is
		the double derivative at said knott (for the 2 polynomials that pass through it). As mentioned above, only the internal knotts
		have unknowns, as the double derivatives at the 2 end knotts are already known (0 for both, as this is a natural spline). This 
		results in (n - 2) unknowns/ equations for n knotts. The equation used is:
		
		hi-1 * zi-1 + 2 * (hi-1 + h1) * zi + hi * zi+1 = 6 * (bi - bi-1)    Where the h expressions are the coefficients,
		                                                                    the the z's the unknowns.

		                                                                    And where:  hi = (xi+1 - xi),
		                                                                    and:        bi = (yi+1 - yi) / hi  (where y = the passed values)

		This is represented in tridiagonal matrix form as:
		 __                                                            __   _  _     __           __
		| 2*(h0 + h1)    h1                                              | | z1 |   |   6*(b1-b0)   |
		|     h1     2*(h1+h2)    h2                                     | | z2 |   |   6*(b2-b1)   |
		|               h2    2*(h2+h3)    h3                            | | z3 |   |   6*(b3-b2)   |
		|                .         .         .                           | |  . | = |       .       |
		|                   .          .         .                       | |  . |   |       .       |
		|                    hn-3    2(hn-3+hn-2)    hn-2                | |zn-1|   | 6*(bn-2-bn-3) |
		|__                              hn-2     2(hn-2 + hn-1)    hn-1_| |zn-2|   |_6*(bn-1-bn-2)_|
		
		The system is solved using TDMA/ Thomas Algorithm. It should be noted that the x coordinates (again, where y = the passed values)
		are aligned to the indices of the parameter array "knott_values", and as such are equally spaced. Because of this, all h's
		are equal to 1, and thus all diagonals in the coeffient matrix are constants (with hi = 1, and 2*(hi-1+hi) = 4), simplifying the
		computation.	*/

	/*	First the values of b are computed for each knott and stored in an array	*/
	const unsigned short knott_b_values_nxt_indx = knott_values_nxt_indx - 1u;
	double* knott_b_values = new double[knott_b_values_nxt_indx];
	for (unsigned short a = 0u; a < knott_b_values_nxt_indx; ++a)
	{
		knott_b_values[a] = knott_values[a + 1u] - knott_values[a];
	}

	/*	TDMA/ thomas algorithm	*/


	/*	 Forward Sweep	*/

	/*	Constant representing the values in the principle diagonal (remember that all elements in the coefficient matrix are constants)	*/
	const double princple_diag = 4.0f;


	/*	Calculates the new values for the upper diagonal elements of the coefficient matrix	*/
	const unsigned short coeff_matrix_upper_nxt_indx = knott_values_nxt_indx - 3u;
	double* coeff_matrix_upper = new double[coeff_matrix_upper_nxt_indx];

	/*	Sets first upper diagonal element:	upper_diag_0_new = upper_diag_0 / princple_diag_0
		after subbing constants:			upper_diag_0_new = 1 / 4	*/
	coeff_matrix_upper[0] = .25f;

	/*	Computes all other upper diagonal elements:	upper_diag_i_new = upper_diag_i / (princple_diag_i - (lower_diag_i * upper_diag_i-1_new))
		after subbing constants and simplifying:	upper_diag_i_new = 1 / (4 - upper_diag_i-1_new)*/
	for (unsigned short a = 1u; a < coeff_matrix_upper_nxt_indx; ++a)
	{
		coeff_matrix_upper[a] = 1.0f / (princple_diag - coeff_matrix_upper[a - 1u]);
	}

	/*	Calculates the new values for the elements within the constant matrix	*/
	const unsigned short const_matrix_nxt_indx = knott_values_nxt_indx - 2u;
	double* const_matrix = new double[knott_values_nxt_indx - 2u];

	/*  Calculates first element ("constant_0 refering to the
	    element in the constant matrix,not the coeff matrix constants"):    constant_0_new = constant_0 / princple_diag_0
	    After subbing constants:                                            constant_0_new = constant_0 / 4
	    Note that the equation for the elements of the constant matrix are inlined below (6 * (bi - bi-1))	*/
	const_matrix[0] = (6 * (knott_b_values[1] - knott_b_values[0])) / princple_diag;

	/*  Calculates all other elements within the constnat matrix:    constant_i_new = (constant_i - (lower_diag_i * constant_i-1_new)) /
	                                                                                  (princple_diag_i - (lower_diag_i * upper_diag_i-1_new))
	    After subbing constants and simplifying:                     constant_i_new = (constant_i - constant_i-1_new) / 
	                                                                                  (4 - upper_diag_i-1_new)	*/
	for (unsigned short a = 1u; a < const_matrix_nxt_indx; ++a)
	{
		unsigned short original_indx = a + 1u;
		const_matrix[a] = ((6 * (knott_b_values[original_indx] - knott_b_values[original_indx - 1u])) - const_matrix[a - 1u]) / (princple_diag - coeff_matrix_upper[a - 1u]);
	}

	
	/*	Back Substitution	*/

	knott_z_values = new double[knott_values_nxt_indx];

	/*  Sets the double derivative of knotts 0 and (n - 1) to = 0    (natural spline)    (also am using (n - 1) to represent the last knott
	    instead of n as 0 is being used to represent the first knott)	*/
	knott_z_values[0] = .0f;
	knott_z_values[knott_values_nxt_indx - 1u] = .0f;

	/*  Calculates last element in variable matrix (finds double derivative of knott (n - 2)):  var_n-1 (where n = size of var matrix) = constant_n-1_new	*/
	knott_z_values[knott_values_nxt_indx - 2u] = const_matrix[const_matrix_nxt_indx - 1u];

	/*  Calculates all remaining elements in the variable matrix: var_i = constant_i_new - (upper_diag_i_new * var_i+1)	*/
	const unsigned short start_indx = knott_values_nxt_indx - 3u;
	for (unsigned short a = start_indx; a > 0; --a)
	{
		const unsigned short matrix_indx = a - 1u;
		knott_z_values[a] = const_matrix[matrix_indx] - (coeff_matrix_upper[matrix_indx] * knott_z_values[a + 1u]);
	}

	/*	Deletes dynamically allocated memory	*/
	this->knott_values = new double[knott_values_nxt_indx];
	for (unsigned short a = 0u; a < knott_values_nxt_indx; ++a)
	{
		this->knott_values[a] = knott_values[a];
	}
	delete[] const_matrix;
	delete[] coeff_matrix_upper;
	delete[] knott_b_values;

	/*	Sets misc objects	*/

	knott_amount = knott_values_nxt_indx;

	/*	Marks that the current object contains a computed spline (thus it can be sampled). Is more so mean't to be used externally	*/	
	intern_is_valid = true;

	/*	Stores the coord of the first knott in the spline internally (only storeing the x and y coordinates, where the z coord
		is the axis in which the spline is aligned, note that these x and y coords are not the same as the indices and values passed
		in "knott_values" (in practice this coord represents the splines location within the distance field)).	*/
	this->spline_prpndclr_coord = spline_prpndclr_coord;
}


/*	This constructor overload initializes the spline	*/
shared_type::ncspline_type::ncspline_type(const double* knott_values, const unsigned short knott_values_nxt_indx, const index_xy_type& spline_prpndclr_coord)
{
	init_spline(knott_values, knott_values_nxt_indx, spline_prpndclr_coord);
}


/*	This constructor does not initialize the spline (in practice this is used if one wishes to delay spline initialization)	*/
shared_type::ncspline_type::ncspline_type(const index_xy_type& spline_prpndclr_coord)
{
	this->spline_prpndclr_coord = spline_prpndclr_coord;
}


/*	Copy Constructor	*/
shared_type::ncspline_type::ncspline_type(const ncspline_type& ncspline)
{
	knott_amount = ncspline.knott_amount;
	if ((ncspline.knott_z_values != nullptr) && (ncspline.knott_values != nullptr))
	{
		knott_z_values = new double[ncspline.knott_amount];
		knott_values = new double[ncspline.knott_amount];
		for (unsigned short a = 0u; a < ncspline.knott_amount; ++a)
		{
			knott_z_values[a] = ncspline.knott_z_values[a];
			knott_values[a] = ncspline.knott_values[a];
		}
		intern_is_valid = true;
	}
	else
	{
		intern_is_valid = false;
	}
	spline_prpndclr_coord = ncspline.spline_prpndclr_coord;
}


/*	Copy Assignment Operator Overload	*/
shared_type::ncspline_type shared_type::ncspline_type::operator=(const ncspline_type& ncspline)
{
	if (this != &ncspline)
	{
		if (knott_z_values != nullptr)
		{
			delete[] knott_z_values;
			knott_z_values = nullptr;
		}
		if (knott_values != nullptr)
		{
			delete[] knott_values;
			knott_values = nullptr;
		}
		ncspline_type copy(ncspline);
		knott_z_values = copy.knott_z_values;
		copy.knott_z_values = nullptr;
		knott_values = copy.knott_values;
		copy.knott_values = nullptr;
		knott_amount = copy.knott_amount;
		spline_prpndclr_coord = copy.spline_prpndclr_coord;
		intern_is_valid = copy.intern_is_valid;
	}

	return *this;
}


/*	Destructor	*/
shared_type::ncspline_type::~ncspline_type()
{
	if (knott_z_values != nullptr)
	{
		delete[] knott_z_values;
		knott_z_values = nullptr;
	}

	if (knott_values != nullptr)
	{
		delete[] knott_values;
		knott_values = nullptr;
	}
}


/*	Default Constructor	*/
shared_type::ncspline_type::ncspline_type()
{

}


/*	Samples spline at specified coord, returns .0 if spline has not yet been initialized	*/
double shared_type::ncspline_type::sample(const double indx_coord)
{
	/*	Checks if spline has been computed/ initialized	*/
	if (knott_z_values != nullptr)
	{
		/*	As the spline is a piecewise function, the below objects are set to represent the values of the
			2 knotts enclosing the specified x coord	*/
		const unsigned short knott_indx = (unsigned short)indx_coord;
		const unsigned short knott_indx_p1 = (unsigned short)indx_coord + 1u;
		const double zi = knott_z_values[knott_indx];
		const double zip1 = knott_z_values[knott_indx + 1u];

		/*	Calculates value at specified coord using a simplified form of the function:
		* 
		    f(x) =  ((zi+1 / (6 * hi)) * (x - xi)^3) -
		            ((zi / (6 * hi)) * (x - xi+1)^3) + 
		            (((yi+1 / hi) - ((hi / 6) * zi+1)) * (x - xi)) -
		            (((yi / hi) - ((hi / 6) * zi)) * (x - xi+1)),

		            Where {xi < 0 < xi+1}

		    (simplified as all h's = 1)	*/

		return	((zip1 / 6.0f) * ((indx_coord - knott_indx) * (indx_coord - knott_indx) * (indx_coord - knott_indx))) -
				((zi / 6) * ((indx_coord - knott_indx_p1) * (indx_coord - knott_indx_p1) * (indx_coord - knott_indx_p1))) +
				((knott_values[knott_indx_p1] - (.166666f * zip1)) * (indx_coord - knott_indx)) -
				((knott_values[knott_indx] - (.166666f * zi)) * (indx_coord - knott_indx_p1));
	}
	else
	{
		return .0;
	}
}