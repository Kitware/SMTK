//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_commmon_CompilerInformation_h
#define smtk_commmon_CompilerInformation_h

#if defined(_MSC_VER)
#define SMTK_MSVC

#elif defined(__INTEL_COMPILER)
#define SMTK_ICC

#elif defined(__PGI)
#define SMTK_PGI

#elif defined(__clang__)
//On OSX the intel compiler uses clang as the front end
//On Windows you can specify the clang compiler as front end to msvc
#define SMTK_CLANG

#elif defined(__GNUC__)
//Now that we have gone through all other compilers that report as gcc/clang
//we can safely say we have the 'real' gcc compiler
#define SMTK_GCC
#endif

#if __cplusplus >= 201103L || (defined(SMTK_MSVC) && _MSC_VER >= 1800)
#define SMTK_HAVE_CXX_11
#endif

#if __cplusplus >= 201402L || (defined(SMTK_MSVC) && _MSC_VER >= 1910)
#define SMTK_HAVE_CXX_14
#endif

// Issue:
// Dynamic cast is not just based on the name of the class, but also the
// combined visibility of the class on OSX. When building the hash_code of
// an object the symbol visibility controls of the type are taken into
// consideration (including symbol vis of template parameters). Therefore, if a
// class has a component with private/hidden vis then it cannot be passed across
// library boundaries.
//
// Solution:
// The solution is fairly simple, but annoying. You need to mark template
// classes intended for use in dynamic_cast with appropropriate visibility
// settings.
//
// TL;DR:
// This markup is used when we want to make sure:
//  - The class can be compiled into multiple libraries and at runtime will
//    resolve to a single type instance
//  - Be a type ( or component of a types signature ) that can be passed between
//    dynamic libraries and requires RTTI support ( dynamic_cast ).
#if defined(SMTK_MSVC)
#define SMTK_ALWAYS_EXPORT
#else
#define SMTK_ALWAYS_EXPORT __attribute__((visibility("default")))
#endif

// Define a pair of macros, SMTK_THIRDPARTY_PRE_INCLUDE and
// SMTK_THIRDPARTY_POST_INCLUDE, that should be wrapped around any #include
// for a third party header file. Mostly this is used to set pragmas that
// disable warnings that smtk checks for but third parties do not.
#if (defined(SMTK_GCC) || defined(SMTK_CLANG))

#define SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS                                                        \
  _Pragma("GCC diagnostic ignored \"-Wconversion\"")                                               \
    _Pragma("GCC diagnostic ignored \"-Wshadow\"")                                                 \
      _Pragma("GCC diagnostic ignored \"-Wcast-align\"")                                           \
        _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")                                   \
          _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#define SMTK_THIRDPARTY_CLANG_WARNING_PRAGMAS                                                      \
  _Pragma("clang diagnostic ignored \"-Wabsolute-value\"")                                         \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-register\"")

#define SMTK_THIRDPARTY_WARNINGS_PUSH _Pragma("GCC diagnostic push")
#define SMTK_THRIDPARTY_WARNINGS_POP _Pragma("GCC diagnostic pop")

#if defined(SMTK_GCC)
#define SMTK_THIRDPARTY_PRE_INCLUDE                                                                \
  SMTK_THIRDPARTY_WARNINGS_PUSH                                                                    \
  SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS
#else /* clang takes GCC pragmas + additional pragmas */
#define SMTK_THIRDPARTY_PRE_INCLUDE                                                                \
  SMTK_THIRDPARTY_WARNINGS_PUSH                                                                    \
  SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS                                                              \
  SMTK_THIRDPARTY_CLANG_WARNING_PRAGMAS
#endif
#define SMTK_THIRDPARTY_POST_INCLUDE SMTK_THRIDPARTY_WARNINGS_POP

#elif defined(SMTK_MSVC)
#define SMTK_THIRDPARTY_PRE_INCLUDE                                                                \
  __pragma(warning(push)) __pragma(                                                                \
    warning(disable : 4180)) /*qualifier applied to function type has no meaning; ignored*/        \
    __pragma(warning(disable : 4244)) /*conversion from 'double' to 'unsigned int'*/               \
    __pragma(warning(disable : 4251)) /*missing DLL-interface*/                                    \
    __pragma(warning(disable : 4267)) /*from size_t to type*/                                      \
    __pragma(warning(disable : 4273)) /*inconsistent dll linkage*/                                 \
    __pragma(warning(disable : 4275)) /*non dll-interface class used as base*/                     \
    __pragma(warning(disable : 4305)) /*truncation from 'double' to 'float'*/                      \
    __pragma(                                                                                      \
      warning(disable : 4373)) /*override when parameters differ by const/volatile qualifiers*/    \
    __pragma(warning(disable : 4522)) /*multiple assignment operators specified*/                  \
    __pragma(warning(disable : 4800)) /*'int': forcing value to bool 'true' or 'false'*/           \
    __pragma(warning(disable : 4996)) /*using non checked iterators*/
#define SMTK_THIRDPARTY_POST_INCLUDE __pragma(warning(pop))

#else
#define SMTK_THIRDPARTY_PRE_INCLUDE
#define SMTK_THIRDPARTY_POST_INCLUDE
#endif

#endif
