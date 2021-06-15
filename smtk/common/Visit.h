//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Visit_h
#define smtk_common_Visit_h

namespace smtk
{
namespace common
{

/// Return values common to most visitor methods.
enum class Visit
{
  Continue, //!< Continue to visit items.
  Halt      //!< Stop visiting items immediately.
};

} // namespace common
} // namespace smtk

#endif // smtk_common_Visit_h
