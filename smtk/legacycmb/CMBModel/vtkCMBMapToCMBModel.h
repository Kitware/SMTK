/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkCMBMapToCMBModel.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBMapToCMBModel

#ifndef __vtkCMBMapToCMBModel_h
#define __vtkCMBMapToCMBModel_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <vector>
#include <map>
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkAlgorithm;
class vtkModelEdge;
class vtkModelVertex;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBMapToCMBModel : public vtkObject
{
  public:
    static vtkCMBMapToCMBModel *New();
    vtkTypeMacro(vtkCMBMapToCMBModel,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    void Operate(vtkDiscreteModelWrapper* ModelWrapper,
        vtkAlgorithm* inputPoly);

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
    //BTX
    void CreateEdgeList(std::vector<std::pair<vtkModelEdge*,int> >& edge_list,
                    std::vector<int>& used_ids,
                    std::vector<vtkModelEdge*>& edges,
                    std::map<int,vtkModelVertex*>& nodeIdToModelVertex,
                    std::map<int, int>& loopNodeIdToCount,
                    std::map<vtkModelEdge*, std::pair<int,int> >& modelEdgeToNodes);
    //ETX
    vtkCMBMapToCMBModel(const vtkCMBMapToCMBModel&);  // Not implemented.
    void operator=(const vtkCMBMapToCMBModel&);  // Not implemented.
};

#endif

