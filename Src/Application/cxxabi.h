/*
 *	File   : cxxabi.h
 *  Remark : Minimum ABI description for RTTI
 */

#ifndef _CXXABI_H
#define _CXXABI_H 1

#ifdef __cplusplus

namespace __cxxabiv1
{
	class __class_type_info: public std::type_info
	{
	public:
		explicit
		__class_type_info(const char* name)
			: std::type_info(name)
		{
		}

		virtual ~__class_type_info();	// Must be implemented later
	};

	// For single non-virtual base
	class __si_class_type_info: public __class_type_info
	{
	public:
		const __class_type_info*	m_base_ptr;

		__si_class_type_info(const char* name, const __class_type_info* base_ptr)
			: __class_type_info(name), m_base_ptr(base_ptr)
		{
		}

		virtual ~__si_class_type_info();	// Must be implemented later
	};

	class __base_class_type_info
	{
	public:
		const __class_type_info *	m_base_type;
		long						m_offset_flags;
	};

	// For multiple and/or virtual base 
	class __vmi_class_type_info: public __class_type_info
	{
	public:
		unsigned int	m_flags;
		unsigned int	m_base_count;

		__base_class_type_info	m_base_info[1];

		__vmi_class_type_info(const char* name, int flags)
			: __class_type_info(name), m_flags(flags), m_base_count(0)
		{
		}

		virtual ~__vmi_class_type_info();	// Must be implemented later
	};

};	// End of namespace __cxxabiv1

#endif	// End of __cplusplus

#endif  // End of _CXXABI_H
