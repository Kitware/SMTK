/*  =========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME ModelVertexClassification -
// .SECTION Description
// Query on mesh point ids to see what model vertex it represents.
// Can also add new model vertices to the DiscerteModel

#ifndef __ModelVertexClassification_H
#define __ModelVertexClassification_H

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkType.h" //needed for vtkIdType
#include <map> //need to store the set of point ids

class vtkDiscreteModel;
class vtkDiscreteModelVertex;

class VTKDISCRETEMODEL_EXPORT ModelVertexClassification
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
