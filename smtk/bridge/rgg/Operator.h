//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_rgg_Operator_h
#define __smtk_session_rgg_Operator_h

#include "smtk/bridge/rgg/Exports.h"
#include "smtk/model/Operator.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;

/**\brief An operator using the RGG "kernel."
  *
  * This is a base class for actual operators.
  */
class SMTKRGGSESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  SessionPtr activeSession();
};

} // namespace rgg
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_Operator_h
