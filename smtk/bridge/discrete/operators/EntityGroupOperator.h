//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_EntityGroupOperator_h
#define __smtk_session_discrete_EntityGroupOperator_h

#include "smtk/bridge/discrete/Exports.h"
#include "smtk/model/operators/EntityGroupOperator.h"
#include "vtkModelEntityGroupOperator.h"
#include "vtkMaterialOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

/**\brief Create, desctory or modify a model entity group.
  *
  * There are three operations available from this Operator class.
  * 1. Create, which will create a entity group with given "BuildEnityType".
  * 2. Destroy, which will remove a entity group with given entity Id;
  * 3. Modify/Operate, which will add or remove entities from the given group.
  */
class SMTKDISCRETESESSION_EXPORT EntityGroupOperator : public smtk::model::EntityGroupOperator
{
public:
  smtkTypeMacro(smtk::bridge::discrete::EntityGroupOperator);
  smtkCreateMacro(smtk::bridge::discrete::EntityGroupOperator);
  smtkSharedFromThisMacro(smtk::model::EntityGroupOperator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  EntityGroupOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Session* discreteSession() const;
  vtkModelEntity* fetchCMBCell(const std::string& parameterName) const;
  vtkModelEntity* fetchCMBCell(
    const smtk::attribute::ModelEntityItemPtr&, int idx ) const;
  int createBoundaryGroup(vtkDiscreteModelWrapper* modelWrapper);
  int createDomainSet(vtkDiscreteModelWrapper* modelWrapper);

  bool modifyGroup(vtkDiscreteModelWrapper* modelWrapper,
                   vtkModelEntity* cmbgroup, bool newGroup,
                   smtk::model::EntityRefArray& modGrps);

  vtkNew<vtkModelEntityGroupOperator> m_opBoundary;
  vtkNew<vtkMaterialOperator> m_opDomain;
};

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

#endif // __smtk_session_discrete_EntityGroupOperator_h
