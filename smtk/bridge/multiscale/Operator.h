//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_Operator_h
#define __smtk_session_multiscale_Operator_h

#include "smtk/bridge/multiscale/Exports.h"
#include "smtk/model/Operator.h"

class vtkDataObject;

namespace smtk {
  namespace bridge {
    namespace multiscale {

class Session;

/**\brief An operator using the Multiscale "kernel."
  *
  * This is a base class for actual operators.
  * It provides convenience methods for accessing Multiscale-specific data
  * for its subclasses to use internally.
  */
class SMTKMULTISCALESESSION_EXPORT Operator : public smtk::model::Operator
{
protected:
  Session* activeSession();
};

    } // namespace multiscale
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_Operator_h
