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

#include "smtk/bridge/discrete/Exports.h"
#include "smtk/model/Operator.h"
#include "vtkEdgeSplitOperator.h"
#include "vtkMergeOperator.h"
#include "vtkNew.h"
#include <set>
#include <map>

class vtkDiscreteModelWrapper;
class vtkDiscreteModelVertex;
class vtkDiscreteModelEdge;

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

class SMTKDISCRETESESSION_EXPORT EdgeOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(EdgeOperator);
  smtkCreateMacro(EdgeOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  EdgeOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Session* discreteSession() const;

  // some internal methods
  void getSelectedVertsAndEdges(
    std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
    std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
    const smtk::attribute::MeshSelectionItemPtr& inSelectionItem,
    smtk::bridge::discrete::Session* opsession);
  bool  convertSelectedEndNodes(
    const std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
    vtkDiscreteModelWrapper* modelWrapper,
    smtk::bridge::discrete::Session* opsession,
    smtk::model::EntityRefArray& srcsRemoved,
    smtk::model::EntityRefArray& srcsModified,
    vtkMergeOperator* mergOp);
  bool splitSelectedEdgeNodes(
    const std::map< smtk::common::UUID,
      std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
    vtkDiscreteModelWrapper* modelWrapper,
    smtk::bridge::discrete::Session* opsession,
    smtk::model::EntityRefArray& srcsCreated,
    smtk::model::EntityRefArray& srcsModified,
    vtkEdgeSplitOperator* splitOp);

  vtkNew<vtkEdgeSplitOperator> m_splitOp;
  vtkNew<vtkMergeOperator> m_mergeOp;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_EdgeOperator_h
