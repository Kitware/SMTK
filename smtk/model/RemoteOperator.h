//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_RemoteOperator_h
#define __smtk_model_RemoteOperator_h

#include "smtk/SharedFromThis.h"
#include "smtk/model/Operator.h"

#include <string>

namespace smtk {
  namespace model {

class RemoteOperator;
typedef smtk::shared_ptr<RemoteOperator> RemoteOperatorPtr;

/**\brief A class for describing remote solid modeling operations.
  *
  * Applications that present model information from a separate process
  * may subclass DefaultSession and add RemoteOperator instances to it
  * which mirror the other process' list of operators.
  * The ImportJSON class aids in this respect by creating
  * RemoteOperator instances from JSON descriptions of operators
  * when a session session inherits DefaultSession.
  *
  * The ableToOperate() and operateInternal() methods of this class
  * will call delegate methods on their session (provided it inherits
  * DefaultSession) so that the results may be obtained from the
  * separate process.
  * The session is responsible for managing communication with the
  * separate process.
  */
class SMTKCORE_EXPORT RemoteOperator : public Operator
{
public:
  smtkTypeMacro(RemoteOperator);
  smtkCreateMacro(RemoteOperator);
  smtkSharedFromThisMacro(Operator);
  // NB. We do not call smtkDeclareModelOperator() because we override name():
  static std::string operatorName;
  static smtk::model::OperatorPtr baseCreate();

  virtual std::string name() const;
  virtual std::string className() const;
  virtual bool ableToOperate();

protected:
  friend class Session;
  friend class DefaultSession;

  Ptr setName(const std::string& opName);

  virtual OperatorResult operateInternal();

  std::string m_name;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Operator_h
