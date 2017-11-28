//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_GrowOperator_h
#define __smtk_session_discrete_GrowOperator_h

#include "smtk/bridge/discrete/Operator.h"
#include "smtk/bridge/discrete/Resource.h"
#include "vtkNew.h"
#include "vtkSeedGrowSelectionFilter.h"
#include "vtkSelection.h"
#include "vtkSelectionSplitOperator.h"
#include <map>
#include <set>

class vtkDiscreteModelWrapper;

namespace smtk
{
namespace bridge
{
namespace discrete
{

class Session;

class SMTKDISCRETESESSION_EXPORT GrowOperator : public Operator
{
public:
  smtkTypeMacro(GrowOperator);
  smtkCreateMacro(GrowOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(Operator);

  bool ableToOperate() override;

protected:
  GrowOperator();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  void findVisibleModelFaces(
    const smtk::model::CellEntity& cellent, std::set<vtkIdType>& ModelFaceIds, Session* opsession);

  bool writeSelectionResult(const std::map<smtk::common::UUID, std::set<int> >& cachedSelection,
    smtk::model::OperatorResult& result);
  void writeSplitResult(vtkSelectionSplitOperator* splitOp, vtkDiscreteModelWrapper* modelWrapper,
    smtk::bridge::discrete::Resource::Ptr& resource, Session* opsession, Result& result);
  // This grow_selection is a list of cell ids from master polydata,
  // so we need to convert that to the format of
  // <FaceUUID, 'set' of cellIds on that face>
  bool convertAndResetOutSelection(
    vtkSelection* inSelection, vtkDiscreteModelWrapper* modelWrapper, Session* opsession);
  bool copyToOutSelection(const smtk::attribute::MeshSelectionItemPtr& inSelectionItem);
  void convertToGrowSelection(const smtk::attribute::MeshSelectionItemPtr& inSelectionItem,
    vtkSelection* outSelection, Session* opsession);

  vtkNew<vtkSelectionSplitOperator> m_splitOp;
  vtkNew<vtkSeedGrowSelectionFilter> m_growOp;
  vtkNew<vtkSelection> m_growSelection;
  std::map<smtk::common::UUID, std::set<int> > m_outSelection;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_GrowOperator_h
