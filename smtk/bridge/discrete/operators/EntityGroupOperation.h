//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_EntityGroupOperation_h
#define __smtk_session_discrete_EntityGroupOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "smtk/bridge/discrete/Resource.h"
#include "smtk/model/operators/EntityGroupOperation.h"
#include "vtkMaterialOperation.h"
#include "vtkModelEntityGroupOperation.h"
#include "vtkNew.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/**\brief Create, desctory or modify a model entity group.
  *
  * There are three operations available from this Operation class.
  * 1. Create, which will create a entity group with given "BuildEnityType".
  * 2. Destroy, which will remove a entity group with given entity Id;
  * 3. Modify/Operate, which will add or remove entities from the given group.
  */
class SMTKDISCRETESESSION_EXPORT EntityGroupOperation : public smtk::model::EntityGroupOperation
{
public:
  smtkTypeMacro(smtk::bridge::discrete::EntityGroupOperation);
  smtkCreateMacro(smtk::bridge::discrete::EntityGroupOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::model::EntityGroupOperation);

  bool ableToOperate() override;

protected:
  EntityGroupOperation();
  Result operateInternal() override;

  const char* xmlDescription() const override;
  vtkModelEntity* fetchCMBCell(
    smtk::bridge::discrete::Resource::Ptr& resource, const std::string& parameterName) const;
  vtkModelEntity* fetchCMBCell(smtk::bridge::discrete::Resource::Ptr& resource,
    const smtk::attribute::ComponentItemPtr&, int idx) const;
  int createBoundaryGroup(vtkDiscreteModelWrapper* modelWrapper);
  int createDomainSet(vtkDiscreteModelWrapper* modelWrapper);

  bool modifyGroup(smtk::bridge::discrete::Resource::Ptr& resource,
    vtkDiscreteModelWrapper* modelWrapper, vtkModelEntity* cmbgroup, bool newGroup,
    smtk::model::EntityRefArray& modGrps);

  vtkNew<vtkModelEntityGroupOperation> m_opBoundary;
  vtkNew<vtkMaterialOperation> m_opDomain;
};

} // namespace discrete
} // namespace bridge

} // namespace smtk

#endif // __smtk_session_discrete_EntityGroupOperation_h
