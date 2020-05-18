//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_common_Managers_h
#define __smtk_common_Managers_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/TypeContainer.h"

namespace smtk
{
namespace common
{
class SMTKCORE_EXPORT Managers
  : public TypeContainer
  , public std::enable_shared_from_this<Managers>
{
public:
  typedef TypeContainer Container;

  smtkTypeMacroBase(smtk::common::Managers);
  smtkCreateMacro(Managers);

  virtual ~Managers();

protected:
  Managers();
};
} // namespace common
} // namespace smtk

#endif
