//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME SystemConfig.h - An .h included by every one for build configuration/macros.
// .SECTION Description
// .SECTION See Also

#ifndef smtk_SystemConfig_h
#define smtk_SystemConfig_h

//Windows specific stuff
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#if !defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#pragma warning(disable : 4251) /* missing DLL-interface */
#pragma warning(disable : 4275) /* non DLL-interface base class */
#pragma warning(disable : 4503) /* truncated decorated name */
#endif                          //!defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#endif                          //Windows specific stuff

#define SMTK_BASE_TYPE(thisclass)                                                                  \
  virtual const char* classname() const                                                            \
  {                                                                                                \
    return #thisclass;                                                                             \
  }

#define SMTK_DERIVED_TYPE(thisclass, superclass)                                                   \
  typedef superclass Superclass;                                                                   \
  const char* classname() const override                                                           \
  {                                                                                                \
    return #thisclass;                                                                             \
  }

#endif //__smtk_SystemConfig_h
