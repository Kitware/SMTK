//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_Operation_h
#define __smtk_session_discrete_Operation_h

#include "smtk/bridge/discrete/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/**\brief An operator using the discrete kernel.
  *
  * This is a base class for actual discrete operators.
  * It provides convenience methods for accessing discrete-specific data
  * for its subclasses to use internally.
  */
class SMTKDISCRETESESSION_EXPORT Operation : public smtk::operation::XMLOperation
{
public:
  virtual ~Operation();
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_Operation_h
