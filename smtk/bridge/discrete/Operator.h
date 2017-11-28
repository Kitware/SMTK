//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_Operator_h
#define __smtk_session_discrete_Operator_h

#include "smtk/bridge/discrete/Exports.h"

#include "smtk/operation/XMLOperator.h"

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
class SMTKDISCRETESESSION_EXPORT Operator : public smtk::operation::XMLOperator
{
public:
  virtual ~Operator();
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_Operator_h
