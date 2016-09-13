//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_commmon_CompilerInformation_h
#define __smtk_commmon_CompilerInformation_h

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

#if __cplusplus >= 201103L || ( defined(SMTK_MSVC) && _MSC_VER >= 1800  )
#define SMTK_HAVE_CXX_11
#endif

// Define a pair of macros, SMTK_THIRDPARTY_PRE_INCLUDE and
// SMTK_THIRDPARTY_POST_INCLUDE, that should be wrapped around any #include
// for a third party header file. Mostly this is used to set pragmas that
// disable warnings that smtk checks for but third parties do not.
#if (defined(SMTK_GCC) || defined(SMTK_CLANG))

#define SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS \
  _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
  _Pragma("GCC diagnostic ignored \"-Wshadow\"") \
  _Pragma("GCC diagnostic ignored \"-Wcast-align\"") \
  _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#define SMTK_THIRDPARTY_CLANG_WARNING_PRAGMAS \
  _Pragma("clang diagnostic ignored \"-Wabsolute-value\"") \
  _Pragma("clang diagnostic ignored \"-Wdeprecated-register\"")

#define SMTK_THIRDPARTY_WARNINGS_PUSH _Pragma("GCC diagnostic push")
#define SMTK_THRIDPARTY_WARNINGS_POP  _Pragma("GCC diagnostic pop")

#if defined (SMTK_GCC)
#define SMTK_THIRDPARTY_PRE_INCLUDE \
  SMTK_THIRDPARTY_WARNINGS_PUSH \
  SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS
#else /* clang takes GCC pragmas + additional pragmas */
#define SMTK_THIRDPARTY_PRE_INCLUDE \
  SMTK_THIRDPARTY_WARNINGS_PUSH \
  SMTK_THIRDPARTY_GCC_WARNING_PRAGMAS \
  SMTK_THIRDPARTY_CLANG_WARNING_PRAGMAS
#endif
#define SMTK_THIRDPARTY_POST_INCLUDE \
  SMTK_THRIDPARTY_WARNINGS_POP

#elif defined(SMTK_MSVC)
#define SMTK_THIRDPARTY_PRE_INCLUDE \
 __pragma(warning(push)) \
 __pragma(warning(disable:4996))  /*using non checked iterators*/ \
 __pragma(warning(disable:4267))  /*from size_t to type*/ \
 __pragma(warning(disable:4251))  /*missing DLL-interface*/ \
 __pragma(warning(disable:4267))  /*unreferenced inline function*/
#define SMTK_THIRDPARTY_POST_INCLUDE \
 __pragma(warning(pop))

#else
#define SMTK_THIRDPARTY_PRE_INCLUDE
#define SMTK_THIRDPARTY_POST_INCLUDE
#endif

#endif
