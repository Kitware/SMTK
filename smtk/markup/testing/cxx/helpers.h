//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_testing_cxx_helpers_h
#define smtk_markup_testing_cxx_helpers_h
/*!\file */

#include "smtk/common/Managers.h"

namespace smtk
{
namespace markup
{

/// Create an smtk::common::Managers instance populated
/// with a resource and operation manager.
smtk::common::Managers::Ptr createTestManagers();

} // namespace markup
} // namespace smtk

#endif // smtk_markup_testing_cxx_helpers_h
