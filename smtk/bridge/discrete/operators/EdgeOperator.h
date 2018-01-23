//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_EdgeOperator_h
#define __smtk_session_discrete_EdgeOperator_h

#include "smtk/bridge/discrete/Operator.h"
#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/operation/vtkEdgeSplitOperator.h"
#include "smtk/bridge/discrete/operation/vtkMergeOperator.h"
#include "vtkNew.h"
#include <map>
#include <set>

class vtkDiscreteModelWrapper;
class vtkDiscreteModelVertex;
class vtkDiscreteModelEdge;

namespace smtk
{
namespace mesh
{
class MeshSet;
typedef std::set<smtk::mesh::MeshSet> MeshSets;
}
}

namespace smtk
{
namespace bridge
{
namespace discrete
{

class Session;

class SMTKDISCRETESESSION_EXPORT EdgeOperator : public Operator
{
public:
  smtkTypeMacro(EdgeOperator);
  smtkCreateMacro(EdgeOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(Operator);

  bool ableToOperate() override;

protected:
  EdgeOperator();
  Result operateInternal() override;
  const char* xmlDescription() const override;

  // some internal methods
  void getSelectedVertsAndEdges(smtk::bridge::discrete::Resource::Ptr& resource,
    std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
    std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
    const smtk::attribute::MeshSelectionItemPtr& inSelectionItem,
    smtk::bridge::discrete::SessionPtr opsession);
  bool convertSelectedEndNodes(const std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
    vtkDiscreteModelWrapper* modelWrapper, smtk::bridge::discrete::SessionPtr opsession,
    smtk::model::EntityRefArray& srcsRemoved, smtk::model::EntityRefArray& srcsModified,
    smtk::mesh::MeshSets& modifiedMeshes, vtkMergeOperator* mergOp);
  bool splitSelectedEdgeNodes(
    const std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
    vtkDiscreteModelWrapper* modelWrapper, smtk::bridge::discrete::SessionPtr opsession,
    smtk::model::EntityRefArray& srcsCreated, smtk::model::EntityRefArray& srcsModified,
    smtk::mesh::MeshSets& modifiedMeshes, vtkEdgeSplitOperator* splitOp);
  int convertToGlobalPointId(smtk::bridge::discrete::Resource::Ptr& resource, int localPid,
    vtkDiscreteModelEdge* cmbModelEdge);

  vtkNew<vtkEdgeSplitOperator> m_splitOp;
  vtkNew<vtkMergeOperator> m_mergeOp;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_EdgeOperator_h
