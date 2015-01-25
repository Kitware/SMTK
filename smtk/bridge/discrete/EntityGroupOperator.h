//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_bridge_discrete_EntityGroupOperator_h
#define __smtk_bridge_discrete_EntityGroupOperator_h

#include "smtk/bridge/discrete/discreteBridgeExports.h"
#include "smtk/model/Operator.h"
#include "vtkModelEntityGroupOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Bridge;

/**\brief Create, desctory or modify a model entity group.
  *
  * There are three operations available from this Operator class.
  * 1. Create, which will create a entity group with given "BuildEnityType".
  * 2. Destroy, which will remove a entity group with given entity Id;
  * 3. Modify/Operate, which will add or remove entities from the given group.
  */
class SMTKDISCRETEBRIDGE_EXPORT EntityGroupOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(EntityGroupOperator);
  smtkCreateMacro(EntityGroupOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  EntityGroupOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* discreteBridge() const;
  int fetchCMBCellId(const std::string& parameterName) const;
  int fetchCMBCellId(
    smtk::attribute::ModelEntityItemPtr entItem, int idx ) const;

  vtkNew<vtkModelEntityGroupOperator> m_op;
};

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

#endif // __smtk_bridge_discrete_EntityGroupOperator_h
