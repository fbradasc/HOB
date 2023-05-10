#pragma once

#ifndef TYPE_TRAITS_LITE_HPP
#define TYPE_TRAITS_LITE_HPP

// Control presence of exception handling (try and auto discover):

// C++ language version detection (C++20 is speculative):
// Note: VC14.0/1900 (VS2015) lacks too much from C++14.

#ifndef   tt_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define tt_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define tt_CPLUSPLUS  __cplusplus
# endif
#endif

#define tt_COMPILER_VERSION( major, minor, patch )  ( 10 * ( 10 * (major) + (minor) ) + (patch) )

#if defined(__GNUC__) && !defined(__clang__)
# define tt_COMPILER_GNUC_VERSION  tt_COMPILER_VERSION(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#else
# define tt_COMPILER_GNUC_VERSION  0
#endif

// Compiler versions:
//
// MSVC++ 6.0  _MSC_VER == 1200 (Visual Studio 6.0)
// MSVC++ 7.0  _MSC_VER == 1300 (Visual Studio .NET 2002)
// MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio .NET 2003)
// MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
// MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
// MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
// MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
// MSVC++ 14.1 _MSC_VER >= 1910 (Visual Studio 2017)

#if defined(_MSC_VER ) && !defined(__clang__)
# define tt_COMPILER_MSVC_VER      (_MSC_VER )
# define tt_COMPILER_MSVC_VERSION  (_MSC_VER / 10 - 10 * ( 5 + (_MSC_VER < 1900 ) ) )
#else
# define tt_COMPILER_MSVC_VER      0
# define tt_COMPILER_MSVC_VERSION  0
#endif

#define tt_HAVE_TYPE_TRAITS          (( tt_CPLUSPLUS >= 201103L ) || tt_COMPILER_MSVC_VER >= 1500)
#define tt_HAVE_TR1_TYPE_TRAITS      (!! tt_COMPILER_GNUC_VERSION )

#if tt_HAVE_TYPE_TRAITS
# include <type_traits>
using namespace std;
#elif tt_HAVE_TR1_TYPE_TRAITS
# include <tr1/type_traits>
using namespace std::tr1;
#else
template<typename B, typename D>
struct is_base_of
{
    typedef char yes[1];
    typedef char no [2];

    static yes& test(const B*);
    static no&  test(...);

    static const bool value = sizeof(test(static_cast<D*>(0))) == sizeof(yes);
};
#endif

#endif // TYPE_TRAITS_LITE_HPP
