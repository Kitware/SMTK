//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME SharedPtr.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_SharedPtr_h
#define __smtk_SharedPtr_h

#include <memory>

namespace smtk
{
  //bring the correct shared_ptr implementation into our project namespace
  using std::shared_ptr;

  //bring the correct make_shared implementation into our project namespace
  using std::make_shared;

  //bring the correct weak_ptr implementation into our project namespace
  using std::weak_ptr;

  //bring in the correct shared_ptr dynamic cast
  using std::dynamic_pointer_cast;

  //bring in the correct shared_ptr const_pointer_cast
  using std::const_pointer_cast;

  //bring in the correct shared_ptr static_pointer_cast
  using std::static_pointer_cast;

  //bring in the correct enable_shared_from_this
  using std::enable_shared_from_this;

  //bring in the correct owner_less so that
  //we store weak ptr in sets/maps even more safely
  using std::owner_less;
}
#endif /* __smtk_SharedPtr_h */
