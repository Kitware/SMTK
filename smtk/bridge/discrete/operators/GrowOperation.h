//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_GrowOperation_h
#define __smtk_session_discrete_GrowOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "smtk/bridge/discrete/Resource.h"
#include "vtkNew.h"
#include "vtkSeedGrowSelectionFilter.h"
#include "vtkSelection.h"
#include "vtkSelectionSplitOperation.h"
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

class SMTKDISCRETESESSION_EXPORT GrowOperation : public Operation
{
public:
  smtkTypeMacro(smtk::bridge::discrete::GrowOperation);
  smtkCreateMacro(GrowOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  GrowOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  void findVisibleModelFaces(
    const smtk::model::CellEntity& cellent, std::set<vtkIdType>& ModelFaceIds, Session* opsession);

  bool writeSelectionResult(
    const std::map<smtk::common::UUID, std::set<int> >& cachedSelection, Result& result);
  void writeSplitResult(vtkSelectionSplitOperation* splitOp, vtkDiscreteModelWrapper* modelWrapper,
    smtk::bridge::discrete::Resource::Ptr& resource, Session* opsession, Result& result);
  // This grow_selection is a list of cell ids from master polydata,
  // so we need to convert that to the format of
  // <FaceUUID, 'set' of cellIds on that face>
  bool convertAndResetOutSelection(
    vtkSelection* inSelection, vtkDiscreteModelWrapper* modelWrapper, Session* opsession);
  bool copyToOutSelection(const smtk::attribute::MeshSelectionItemPtr& inSelectionItem);
  void convertToGrowSelection(const smtk::attribute::MeshSelectionItemPtr& inSelectionItem,
    vtkSelection* outSelection, Session* opsession);

  vtkNew<vtkSelectionSplitOperation> m_splitOp;
  vtkNew<vtkSeedGrowSelectionFilter> m_growOp;
  vtkNew<vtkSelection> m_growSelection;
  std::map<smtk::common::UUID, std::set<int> > m_outSelection;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_GrowOperation_h
