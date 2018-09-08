//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMapToCMBModel

#ifndef __smtkdiscrete_vtkCMBMapToCMBModel_h
#define __smtkdiscrete_vtkCMBMapToCMBModel_h

#include "smtk/session/discrete/Exports.h" // For export macro
#include "vtkObject.h"
#include <map>
#include <vector>

class vtkDiscreteModelWrapper;
class vtkAlgorithm;
class vtkModelEdge;
class vtkModelVertex;

class SMTKDISCRETESESSION_EXPORT vtkCMBMapToCMBModel : public vtkObject
{
public:
  static vtkCMBMapToCMBModel* New();
  vtkTypeMacro(vtkCMBMapToCMBModel, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Operate(vtkDiscreteModelWrapper* ModelWrapper, vtkAlgorithm* inputPoly);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBMapToCMBModel();
  ~vtkCMBMapToCMBModel();

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

private:
  void CreateEdgeList(std::vector<std::pair<vtkModelEdge*, int> >& edge_list,
    std::vector<int>& used_ids, std::vector<vtkModelEdge*>& edges,
    std::map<int, vtkModelVertex*>& nodeIdToModelVertex, std::map<int, int>& loopNodeIdToCount,
    std::map<vtkModelEdge*, std::pair<int, int> >& modelEdgeToNodes);

  vtkCMBMapToCMBModel(const vtkCMBMapToCMBModel&); // Not implemented.
  void operator=(const vtkCMBMapToCMBModel&);      // Not implemented.
};

#endif
