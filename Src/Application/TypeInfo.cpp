/*
 * File   : TypeInfo.cpp
 * Remark : Minimum implementation of RTTI (Run Time Type Information).
 *          This implementation is just 'satisfying the compiler', in this case GCC, rather
 *          than complying with standard ABI and may not work for untested cases.
 */

#include <typeinfo>
#include "cxxabi.h"
#include "NanoOSApi.h"

void operator delete(void *p)
{
}

std::type_info::~type_info()
{
}

// May inaccurate
const char* std::type_info::name() const
{
	char* p = const_cast<char*>(m_name);
	while (*p != '\0') {
		if ((*p >= 'A') && (*p <= 'Z'))
			break;
		if ((*p >= 'a') && (*p >= 'A'))
			break;
		if (*p == '_') break;

		p++;
	}
	return p;
}

bool std::type_info::operator ==(const type_info& ti) const
{
	return ((StrCmp(m_name, ti.m_name) == 0) ? true : false);
}

bool std::type_info::operator !=(const type_info& ti) const
{
	return (! operator ==(ti));
}

bool std::type_info::before(const type_info &ti) const
{
	return (m_name < ti.m_name) ? true : false;
}

/*
 * Declare builtin type type info variables.
 * This is incomplete list. If required you can add another 
 * builtin variable type info here when you encounter a 
 * linker error like:
 *
 * "undefined reference to `typeinfo for float'"
 *
 * The error message don't tell variable symbol name, to make 
 * the linker show variable symbol name unmark the linker option 
 * of --no-demangle in this project makefile and then recompile 
 * the project. Now it should show linker error:
 *
 * "undefined reference to `_ZTIf'"
 *
 * In this case variable symbol name of _ZTIf is a typeinfo for float.
 * Then declare a variable name of _ZTIf with type of std::type_info
 * and initialize with specified variable type name in this case float.
 * The declaration should look like this:
 *
 * std::type_info _ZTIf("float");
 *
 */
std::type_info	_ZTIi("int");
std::type_info	_ZTIf("float");
std::type_info	_ZTIPc("char*");
std::type_info	_ZTIPKc("const char*");
std::type_info	_ZTId("double");
std::type_info	_ZTIc("char");

/*
 * Implementations of virtual functions of __cxxabiv1 namespace.
 */
__cxxabiv1::__class_type_info::~__class_type_info()
{
}

__cxxabiv1::__si_class_type_info::~__si_class_type_info()
{
}

__cxxabiv1::__vmi_class_type_info::~__vmi_class_type_info()
{
}

extern "C"
{

// Just to satisfy the compiler.
void __cxa_bad_typeid()
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
	ExitThread();
}

typedef long ptrdiff_t;

/*
 * The compiler can generate almost all dynamic casting operations codes, but in some cases 
 * it requires external function like __dynamic_cast() at run time to decide how dynamic cast 
 * operation being performed. A case like this one, in pseudo code, need __dynamic_cast():
 *
 * ...
 *
 * class Base {
 * public:
 *     virtual void Func() {...}
 * };
 *
 * class Derived: public Base {
 * public:
 *     void Func() {...)
 * };
 *
 * ...
 *
 * Base *pBase;
 * Derived *pDer, DerivedObj;
 *
 * pBase = &DerivedObj;
 * pDer = dynamic_cast<Derived *>(pBase);	--> This need __dynamic_cast()
 *
 * ...
 *
 * This implementation handle case as such above.
 *
 * Yes, i've made a lot of assumtions.
 */
void* __dynamic_cast(
					 const void* __src_ptr, 
					 const __cxxabiv1::__class_type_info* __src_type, 
					 const __cxxabiv1::__class_type_info* __dst_type, 
					 ptrdiff_t __src2dst
					 )
{
	UINT32_PTR_T	pTmp;

	DBG_PRINTF("%s: __src_type=0x%X (%s), __dst_type=0x%X (%s)\r\n", 
		__FUNCTION__, __src_type, __src_type->name(), __dst_type, __dst_type->name());

	if (!__src_ptr) return 0;

	/*
	 * Assume that first 4 bytes content of __src_ptr is address to 
	 * first virtual function field of __cxxabiv1::xxx vtable structure, 
	 * this maybe compiler specific implementation, GCC for ARM ver. 4.8.x 
	 * in this case.
	 * The vtable structure in C may look like this:
	 * struct _VTABLE
	 * {
	 *     UINT32_T Null; --> content always 0
	 *     UINT32_T TInfoPtr; --> pointer to type_info or __cxxabi1::xxx type 
	 *                            info structure such as __class_type_info
	 *     UINT32_T VFuncAddr; --> first virtual function address
	 *     ...
	 * }
	 */
	pTmp = (UINT32_PTR_T) (*(UINT32_PTR_T) __src_ptr);
	if (!pTmp) return 0;

	/*
	 * Also assume that 4 bytes above first virtual function of 
	 * vtable is the address of one of __cxxabiv1::xxx type info class
	 * such as __class_type_info class (see _VTABLE structure above). 
	 * If this address is the same as __dst_type then __src_ptr should 
	 * point to the original object plus __src2dst.
	 * In this case __src2dat is interpreted as an offset from __src_ptr
	 * to the original object pointer. __src2dst maybe bigger than 0 
	 * whenever user use multiple class inheritance.
	 */
	pTmp--;
	if (*pTmp == (UINT32_T) __dst_type) {
		return (void*) (((UINT32_T) __src_ptr) - __src2dst);
	}

	return 0;
}

};	// End of extern "C"
