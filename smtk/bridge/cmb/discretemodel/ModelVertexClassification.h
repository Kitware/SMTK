//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME ModelVertexClassification -
// .SECTION Description
// Query on mesh point ids to see what model vertex it represents.
// Can also add new model vertices to the DiscerteModel

#ifndef __smtkcmb_ModelVertexClassification_H
#define __smtkcmb_ModelVertexClassification_H

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkType.h" //needed for vtkIdType
#include <map> //need to store the set of point ids


class vtkDiscreteModel;
class vtkDiscreteModelVertex;

class VTKSMTKDISCRETEMODEL_EXPORT ModelVertexClassification
{
public:
  ModelVertexClassification(vtkDiscreteModel* model);

  vtkDiscreteModelVertex* GetModelVertex( vtkIdType pointId );
  vtkIdType GetModelId( vtkIdType pointId );

  bool HasModelVertex( vtkIdType pointId ) const;

  //given a point Id will Add a new model vertex to the associated discrete model
  //and return both the ModelVertex and ModelId. If a model vertex is already
  //associated with that pointId this will return the exists ModelVertex and
  //ModelId
  std::pair<vtkIdType, vtkDiscreteModelVertex*>
  AddModelVertex( vtkIdType pointId, bool bCreateGeometry=false);


private:
  typedef std::pair<vtkIdType, vtkDiscreteModelVertex*> ModelVertexInfo;
  std::map<vtkIdType,ModelVertexInfo> ModelVertInfo;
  vtkDiscreteModel* Model;
};

#endif //__ModelVertexClassification_H
