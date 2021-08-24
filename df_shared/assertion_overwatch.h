#pragma once
#include "..\df_shared\concepts.h"
#include <limits>
#include <vector>
#include <type_traits>

/*Checks for:
	- overflow
	- deleting non existant objects
	- accessing nullptr

	don't forget to add unregister functions for both statically and dynamically allocated objects (for static, unregister when end of scope)
*/

class assertion_overwatch_type
{
	int assertion_code = 0;

	template <convertible_to_int T>
	T saturate(T value, T min_value, T max_value)
	{
		return (std::min)((std::max)(value, min_value), max_value);
	}

	template<typename Tvector_type, convertible_to_int Tindex_type>
	struct invrse_jenga_type
	{
		/*Data Members*/

		std::vector<Tvector_type> vec;

		/*Member Functions*/

		Tvector_type* remove_element(const Tindex_type& index);
		void clean()
		{
			std::vector<Tvector_type>().swap(vec);
		}
	};

	class base_obj_meta_type
	{
	protected:

		virtual void call_self_ptr_handlers() = 0;
		char class_type = '\0';
		char alloc_type = '\0';
		assertion_overwatch_type* assertion_overwatch_ptr = nullptr;

	public:
		virtual char get_class_type()
		{
			return this->class_type;
		}
		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_data_members() = 0;
		virtual void* get_ptr() = 0;
		virtual char get_size() = 0;
		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_types() = 0;
		virtual ~base_obj_meta_type() = 0;
		virtual int call_test_for_overflow(const long double value) = 0;
		virtual int call_test_for_overflow(const long long value) = 0;
		virtual int call_test_for_overflow(const unsigned long long value) = 0;
		virtual int ptr_deletion_handler(base_obj_meta_type* ptr) = 0;
		char get_alloc_type()
		{
			return this->alloc_type;
		}
	};

	class ptr_meta_type : public base_obj_meta_type
	{
		base_obj_meta_type* ptr = nullptr;
		bool ref_obj_deleted = false;
		invrse_jenga_type<ptr_meta_type*, unsigned long long> self_ptrs;

		virtual void call_self_ptr_handlers()
		{
			unsigned long long size = this->self_ptrs.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->self_ptrs.vec[a] != nullptr)
				{
					this->self_ptrs.vec[a]->ref_obj_deletion_handler();
				}
			}
		}

	public:

		virtual int ptr_deletion_handler(base_obj_meta_type* ptr)
		{
			if (ptr != nullptr)
			{
				unsigned long long size = this->self_ptrs.vec.size();
				for (unsigned long long a = 0u; a < size; ++a)
				{
					if (this->self_ptrs.vec[a] == ptr)
					{
						this->self_ptrs.remove_element(a);
						return 0;
					}
				}

				this->assertion_overwatch_ptr->assertion_code = 207;
				return 1;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 202;
				return -1;
			}
		}

		void ref_obj_deletion_handler()
		{
			this->ptr = nullptr;
			this->ref_obj_deleted = true;
		}

		bool get_ref_obj_deleted()
		{
			return this->ref_obj_deleted;
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_data_members()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}

		virtual void* get_ptr()
		{
			return this->ptr;
		}

		void change_ref_obj(base_obj_meta_type* ptr)
		{
			this->ptr = ptr;
			this->ref_obj_deleted = false;
		}

		virtual char get_size()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return '\0';
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_types()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}

		virtual int call_test_for_overflow(const long double value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const unsigned long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		ptr_meta_type(base_obj_meta_type* ptr, const char alloc_type)
		{
			this->class_type = '\1';
			this->alloc_type = alloc_type;

			if (ptr != nullptr)
			{
				this->ptr = ptr;
			}
		}

		virtual ~ptr_meta_type()
		{
			call_self_ptr_handlers();

			if (this->ptr != nullptr)
			{
				this->ptr->ptr_deletion_handler(this);
			}
		}
	};

	template<convertible_to_int T>
	class primitive_meta_type : public base_obj_meta_type
	{
		T* ptr = nullptr;
		char size = 0;
		invrse_jenga_type<ptr_meta_type*, unsigned long long> self_ptrs;

		virtual void call_self_ptr_handlers()
		{
			unsigned long long size = this->self_ptrs.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->self_ptrs.vec[a] != nullptr)
				{
					this->self_ptrs.vec[a]->ref_obj_deletion_handler();
				}
			}
		}

	public:

		virtual int ptr_deletion_handler(base_obj_meta_type* ptr)
		{
			if (ptr != nullptr)
			{
				unsigned long long size = this->self_ptrs.vec.size();
				for (unsigned long long a = 0u; a < size; ++a)
				{
					if (this->self_ptrs.vec[a] == ptr)
					{
						this->self_ptrs.remove_element(a);
						return 0;
					}
				}

				this->assertion_overwatch_ptr->assertion_code = 207;
				return 1;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 202;
				return -1;
			}
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_data_members()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}
		
		virtual void* get_ptr()
		{
			return this->ptr;
		}

		virtual char get_size()
		{
			return this->size;
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_types()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}

		template <convertible_to_int U>
		int test_for_overflow_min(const U value)
		{
			const U min_value = (U)(std::numeric_limits<T>::min)();
			if (value <= min_value)
			{
				return 0;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 100;
				return 1;
			}
		}

		template <convertible_to_int U>
		int test_for_overflow_max(const U value)
		{
			const U max_value = (U)(std::numeric_limits<T>::max)();
			if (value <= max_value)
			{
				return 0;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 100;
				return 1;
			}
		}

		int test_for_overflow_float_to_int(const long double value)
		{
			const long double value_casted = value;
			const long double min_value = (long double)(std::numeric_limits<T>::min)();
			const long double max_value = (long double)(std::numeric_limits<T>::max)();
			const long double margin = .00001l;
			if ((value_casted < (max_value - .00001l)) && (value_casted > (min_value + .00001l)))
			{
				return 0;
			}
			else if ((value_casted > max_value) || (value_casted < min_value))
			{
				this->assertion_overwatch_ptr->assertion_code = 100;
				return 1;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 101;
				return 2;
				/*ASSERT DUE TO POTENTIAL FLOATING POINT PRECISION ERROR*/
			}
		}

		template <convertible_to_int U>
		int test_for_overflow(const U value, const char type)
		{
			if (std::is_signed<T>() == true)
			{
				if (std::is_floating_point<T>() == true)
				{
					
					return test_for_overflow_min<long double>((const long double)value) && test_for_overflow_max<long double>((const long double)value);
				}
				else
				{
					if (type != '\1')
					{
						if (type == '\2')
						{
							unsigned long long max_value = (unsigned long long)(std::numeric_limits<T>::max)();
							if (value > max_value)
							{
								this->assertion_overwatch_ptr->assertion_code = 100;
								return 1;
							}
						}
						else
						{
							return test_for_overflow_float_to_int(value);
						}

						return check_for_overflow_min<long long>((long long)value);
					}
					else
					{
						return check_for_overflow_min<long long>(value) && check_for_overflow_max<long long>(value);
					}
				}
			}
			else
			{
				if (type != '\2')
				{
					if (type == '\1')
					{
						if (value < 0ll)
						{
							this->assertion_overwatch_ptr->assertion_code = 100;
							return 1;
						}
						
						return check_for_overflow_max<unsigned long long>((unsigned long long)value);
					}
					else
					{
						return test_for_overflow_float_to_int(value);
					}
				}
				else
				{
					return test_for_overflow_min<unsigned long long>(value) && test_for_overflow_max<unsigned long long>(value);
				}
			}
		}

		virtual int call_test_for_overflow(const long double value)
		{
			return test_for_overflow<long double>(value, '\0');
		}

		virtual int call_test_for_overflow(const long long value)
		{
			return test_for_overflow<long long>(value, '\1');
		}

		virtual int call_test_for_overflow(const unsigned long long value)
		{
			return test_for_overflow<unsigned long long>(value, '\2');
		}

		primitive_meta_type(T* ptr, const char alloc_type)
		{
			this->class_type = '\2';
			this->alloc_type = alloc_type;

			if (ptr != nullptr)
			{
				this->ptr = ptr;
				this->size = sizeof(*ptr);
			}
		}

		virtual ~primitive_meta_type()
		{
			call_self_ptr_handlers();
		}
	};

	template<not_convertible_to_int T>
	class composite_meta_type : public base_obj_meta_type
	{
	
		void* ptr = nullptr;
		invrse_jenga_type<base_obj_meta_type*, unsigned long long> data_members;
		invrse_jenga_type<ptr_meta_type*, unsigned long long> self_ptrs;

		void call_self_ptr_handlers()
		{
			unsigned long long size = this->self_ptrs.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->self_ptrs.vec[a] != nullptr)
				{
					this->self_ptrs.vec[a]->ref_obj_deletion_handler();
				}
			}
		}

	public:

		virtual int ptr_deletion_handler(base_obj_meta_type* ptr)
		{
			if (ptr != nullptr)
			{
				unsigned long long size = this->self_ptrs.vec.size();
				for (unsigned long long a = 0u; a < size; ++a)
				{
					if (this->self_ptrs.vec[a] == ptr)
					{
						this->self_ptrs.remove_element(a);
						return 0;
					}
				}

				this->assertion_overwatch_ptr->assertion_code = 207;
				return 1;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 202;
				return -1;
			}
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_data_members()
		{
			return &this->data_members;
		}

		virtual void* get_ptr()
		{
			return this->ptr;
		}

		virtual char get_size()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return '\0';
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_types()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}

		virtual int call_test_for_overflow(const long double value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const unsigned long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		composite_meta_type(T* ptr, const char alloc_type)
		{
			this->class_type = '\3';
			this->alloc_type = alloc_type;

			if (ptr != nullptr)
			{
				this->ptr = (void*)ptr;
			}
		}

		composite_meta_type()
		{
			this->class_type = '\3';
		}

		virtual ~composite_meta_type()
		{
			const unsigned long long size = this->data_members.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->data_members.vec[a] != nullptr)
				{
					delete this->data_members.vec[a];
				}
			}

			call_self_ptr_handlers();
		}
	};

	class array_meta_type : public base_obj_meta_type
	{
		unsigned long long length = 0ull;
		void* ptr = nullptr;
		invrse_jenga_type<base_obj_meta_type*, unsigned long long> types;
		invrse_jenga_type<ptr_meta_type*, unsigned long long> self_ptrs;
		invrse_jenga_type<base_obj_meta_type*, unsigned long long> contents;

		void call_self_ptr_handlers()
		{
			unsigned long long size = this->self_ptrs.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->self_ptrs.vec[a] != nullptr)
				{
					this->self_ptrs.vec[a]->ref_obj_deletion_handler();
				}
			}
		}

	public:

		virtual int ptr_deletion_handler(base_obj_meta_type* ptr)
		{
			if (ptr != nullptr)
			{
				unsigned long long size = this->self_ptrs.vec.size();
				for (unsigned long long a = 0u; a < size; ++a)
				{
					if (this->self_ptrs.vec[a] == ptr)
					{
						this->self_ptrs.remove_element(a);
						return 0;
					}
				}

				this->assertion_overwatch_ptr->assertion_code = 207;
				return 1;
			}
			else
			{
				this->assertion_overwatch_ptr->assertion_code = 202;
				return -1;
			}
		}

		unsigned long long get_length()
		{
			return this->length;
		}
		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_data_members()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return nullptr;
		}

		invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_contents()
		{
			return &this->contents;
		}

		virtual void* get_ptr()
		{
			return this->ptr;
		}

		virtual char get_size()
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return '\0';
		}

		virtual invrse_jenga_type<base_obj_meta_type*, unsigned long long>* get_types()
		{
			return &this->types;
		}

		virtual int call_test_for_overflow(const long double value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		virtual int call_test_for_overflow(const unsigned long long value)
		{
			this->assertion_overwatch_ptr->assertion_code = 200;
			return 3;
		}

		array_meta_type(void* ptr, const char alloc_type, const unsigned long long length)
		{
			this->class_type = '\4';
			this->alloc_type = alloc_type;

			if (ptr != nullptr)
			{
				this->ptr = ptr;
			}
			this->length = length;
		}

		virtual ~array_meta_type()
		{
			const unsigned long long size = this->types.vec.size();
			for (unsigned long long a = 0u; a < size; ++a)
			{
				if (this->types.vec[a] != nullptr)
				{
					delete this->types.vec[a];
				}
			}

			call_self_ptr_handlers();
		}
	};

	composite_meta_type<void> base_object = composite_meta_type<void>();
	invrse_jenga_type<base_obj_meta_type*, unsigned long long>* base_data_members = nullptr;

public:

	/*	Data Members	*/

	typedef base_obj_meta_type* registry_address_type;

	/*	Member Functions	*/

	template <floating_point T>
	registry_address_type register_primitive(T* ptr, const char alloc_type)
	{
		base_obj_meta_type* new_meta_ptr = new primitive_meta_type<T>(ptr, alloc_type);
		base_data_members->vec.push_back(new_meta_ptr);
		return new_meta_ptr;
	}

	template <floating_point T>
	registry_address_type register_primitive(T* ptr, const char alloc_type, registry_address_type enclsing_obj_address)
	{
		if (enclsing_obj_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* data_members = enclsing_obj_address->get_data_members();
			if (data_members != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new primitive_meta_type <T>(ptr, alloc_type);
				enclsing_obj_address->get_data_members()->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 203;
				return nullptr;
			}

		}
		else
		{
			this->assertion_code = 204;
			return nullptr;
		}
	}


	template <not_convertible_to_int T>
	registry_address_type register_composite(T* ptr, const char alloc_type)
	{
		base_obj_meta_type* new_meta_ptr = new composite_meta_type<T>(ptr, alloc_type);
		base_data_members->vec.push_back(new_meta_ptr);
		return new_meta_ptr;
	}


	template <not_convertible_to_int T>
	registry_address_type register_composite(T* ptr, const char alloc_type, registry_address_type enclsing_obj_address)
	{
		if (enclsing_obj_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* data_members = enclsing_obj_address->get_data_members();
			if (data_members != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new composite_meta_type <T>(ptr, alloc_type);
				data_members->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 203;
				return nullptr;
			}
		}
		else
		{
			this->assertion_code = 204;
			return nullptr;
		}
	}


	registry_address_type register_array(void* ptr, const char alloc_type, const unsigned long long length)
	{
		base_obj_meta_type* new_meta_ptr = new array_meta_type(ptr, alloc_type, length);
		base_data_members->vec.push_back(new_meta_ptr);
		return new_meta_ptr;
	}


	registry_address_type register_array(void* ptr, const char alloc_type, const unsigned long long length, registry_address_type enclsing_obj_address)
	{
		if (enclsing_obj_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* data_members = enclsing_obj_address->get_data_members();
			if (data_members != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new array_meta_type(ptr, alloc_type, length);
				data_members->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 203;
				return nullptr;
			}
		}
		else
		{
			this->assertion_code = 204;
			return nullptr;
		}
	}


	registry_address_type register_ptr(registry_address_type registered_obj_address, const char alloc_type)
	{
		base_obj_meta_type* new_meta_ptr = new ptr_meta_type(registered_obj_address, alloc_type);
		base_data_members->vec.push_back(new_meta_ptr);
		return new_meta_ptr;
	}


	registry_address_type register_ptr(registry_address_type registered_obj_address, const char alloc_type, registry_address_type enclsing_obj_address)
	{
		if (enclsing_obj_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* data_members = enclsing_obj_address->get_data_members();
			if (data_members != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new ptr_meta_type(registered_obj_address, alloc_type);
				data_members->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 203;
				return nullptr;
			}
		}
		else
		{
			this->assertion_code = 204;
			return nullptr;
		}
	}

	int register_ptr_change(registry_address_type ptr, registry_address_type registered_obj_address)
	{
		if (ptr != nullptr)
		{
			ptr_meta_type* ptr_casted = (ptr_meta_type*)ptr;
			ptr_casted->change_ref_obj(registered_obj_address);
		}
		else
		{
			this->assertion_code = 204;
		}
	}


	/*	Registers primitive type (only types implicitly convertible to int are able to be registered as primitives)	*/
	template <floating_point T>
	registry_address_type register_primitive_type(T* ptr, const char alloc_type, registry_address_type enclsing_array_address)
	{
		if (enclsing_array_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* types = enclsing_array_address->get_types();
			if (types != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new primitive_meta_type<T, long double>(ptr, alloc_type);
				types->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 205;
			}
		}
		else
		{
			this->assertion_code = 204;
		}
	}

	template <not_convertible_to_int T>
	registry_address_type register_composite_type(T* ptr, const char alloc_type, registry_address_type enclsing_array_address)
	{
		if (enclsing_array_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* types = enclsing_array_address->get_types();
			if (types != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new composite_meta_type<T>(ptr, alloc_type);
				types->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 205;
			}
		}
		else
		{
			this->assertion_code = 204;
		}
	}

	registry_address_type register_array_type(void* ptr, const char alloc_type, const unsigned long long length, registry_address_type enclsing_array_address)
	{
		if (enclsing_array_address != nullptr)
		{
			invrse_jenga_type<base_obj_meta_type*, unsigned long long>* types = enclsing_array_address->get_types();
			if (types != nullptr)
			{
				base_obj_meta_type* new_meta_ptr = new array_meta_type(ptr, alloc_type, length);
				types->vec.push_back(new_meta_ptr);
				return new_meta_ptr;
			}
			else
			{
				this->assertion_code = 205;
			}
		}
		else
		{
			this->assertion_code = 204;
		}
	}

	template<convertible_to_int T>
	int verify_primitive_assignment(registry_address_type obj_address, const T value)
	{
		if (obj_address != nullptr)
		{
			if (obj_address->get_class_type() == '\2')
			{
				if (std::is_signed<T>() == true)
				{
					if (std::is_floating_point<T>() == true)
					{
						return obj_address->call_test_for_overflow((long double)value);
					}
					else
					{
						return obj_address->call_test_for_overflow((long long)value);
					}
				}
				else
				{
					return obj_address->call_test_for_overflow((unsigned long long)value);
				}
			}
			else
			{
				this->assertion_code = 206;
			}
		}
		else
		{
			this->assertion_code = 204;
		}
	}

	/*	Return codes:
			0	:	Index is within range
			1	:	Index is outside range
			-1	:	Invalid arguments (array_address was of incorrect derived class type)
			-2	:	Invalid arguments (array_address was nullptr)*/
	int verify_array_indexing(registry_address_type array_address, const unsigned long long index)
	{
		if (array_address != nullptr)
		{
			if (array_address->get_class_type() == '\4')
			{
				array_meta_type* array_address_casted = (array_meta_type*)array_address;
				if (index < array_address_casted->get_length())
				{
					return 0;
				}
				else
				{
					this->assertion_code = 103;
					return 1;
				}
			}
			else
			{
				this->assertion_code = 206;
				return -1;
			}
		}
		else
		{
			this->assertion_code = 204;
			return -2;
		}
	}


	int verify_ptr_access(); //Test if poiinted to object is deleted

	/*	Different functions for unregistering objects vs unregistering pointers to objects (so that you can unregister an object
		by passing it's pointer, without making the overwatch think your trying to unregister to pointer)	*/

	int unregister();

	/*	Return codes:
			0	:	No direct errors
			-1	:	Invalid arguments (obj_address was dynamically allocated, must be static)
			-2	:	Invalid arguments (obj_address was nullptr)*/
	int unregister_static_obj(registry_address_type obj_address)
	{
		if (obj_address != nullptr)
		{
			if (obj_address->get_alloc_type() == '\0')
			{
				delete obj_address;
				return 0;
			}
			else
			{
				this->assertion_code = 206;
				return -1;
			}
		}
		else
		{
			this->assertion_code = 204;
			return -2;
		}
	}

	/*	Return codes:
			0	:	No direct errors
			1	:	Object has been deleted
			2	:	Registered ptr does not point to an object
			3	:	Object has been deleted and argument ptr was nullptr
			4	:	Registered ptr does not point to an object and argument ptr was nullptr
			-1	:	Invalid arguments (ptr_address was not a pointer (dynamically allocated memory must be deregistered through a registered pointer))
			-2	:	Invalid arguments (ptr_address was nullptr)*/
	int verify_ptr_access(registry_address_type ptr_address, const void* ptr)
	{
		if (ptr_address != nullptr)
		{
			if (ptr_address->get_class_type() == '\1')
			{
				ptr_meta_type* ptr_address_casted = (ptr_meta_type*)ptr_address;
				if (ptr_address_casted->get_ref_obj_deleted() == false)
				{
					if (ptr_address_casted->get_ptr() != nullptr)
					{
						return 0;
					}
					else
					{
						if (ptr == nullptr)
						{
							this->assertion_code = 104;
							return 4;
						}
						else
						{
							this->assertion_code = 208;
							return 2;
						}
					}
				}
				else
				{
					if (ptr == nullptr)
					{
						this->assertion_code = 106;
						return 3;
					}
					else
					{
						this->assertion_code = 107;
						return 1;
					}
				}
			}
			else
			{
				this->assertion_code = 206;
				return -1;
			}
		}
		else
		{
			this->assertion_code = 204;
			return -2;
		}
	}

	/*	Return codes:
			0	:	No direct errors
			1	:	Object has already been deleted
			2	:	Registered ptr does not point to an object
			3	:	Object has been deleted and argument ptr was nullptr
			4	:	Registered ptr does not point to an object and argument ptr was nullptr
			-1	:	Invalid arguments (ptr_address was not a pointer (dynamically allocated memory must be deregistered through a registered pointer))
			-2	:	Invalid arguments (ptr_address was nullptr)*/
	int unregister_dynamic_obj(registry_address_type ptr_address, const void* ptr)
	{
		int return_code = verify_ptr_access(ptr_address, ptr);
		if (return_code == 0)
		{
			ptr_meta_type* ptr_address_casted = (ptr_meta_type*)ptr_address;
			delete ptr_address_casted->get_ptr();
			return 0;
		}
		else
		{

			return return_code;
		}
	}

	/*	Assertion Codes:
			*/
	int check_assertion_code()
	{
		return this->assertion_code;
	}

	assertion_overwatch_type()
	{
		base_data_members = base_object.get_data_members();
	}
};

template<typename Tvector_type, convertible_to_int Tindex_type>
inline Tvector_type* assertion_overwatch_type::invrse_jenga_type<Tvector_type, Tindex_type>::remove_element(const Tindex_type& index)
{
	Tindex_type last_index = vec.size() - 1;

	if (index == last_index)
	{
		vec.pop_back();

		return nullptr;
	}
	else
	{
		vec[index] = vec[last_index];
		vec.pop_back();

		return &vec[index];
	}
}