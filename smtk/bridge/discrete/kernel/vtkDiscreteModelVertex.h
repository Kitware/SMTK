//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelVertex - CMB specific model vertex class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkDiscreteModelVertex_h
#define __smtkdiscrete_vtkDiscreteModelVertex_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkModelVertex.h"


class vtkInformationIdTypeKey;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelVertex : public vtkModelVertex
{
public:
  vtkTypeMacro(vtkDiscreteModelVertex,vtkModelVertex);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Pure virtual function to get the point location of the
  // model vertex.  Returns true for success and false for failure.
  // Fills the x, y, and z values in xyz if success.
  virtual bool GetPoint(double*);

  // Description:
  // The point id of the model vertex defined on the master polydata.
  // This will not create geometry automatically anymore.
  void SetPointId(vtkIdType id);
  vtkIdType GetPointId();

  static vtkInformationIdTypeKey* POINTID();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  //virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Create geometry using the point id of the model vertex defined on the master polydata.
  void CreateGeometry();

protected:
  static vtkDiscreteModelVertex* New();
  vtkDiscreteModelVertex();
  virtual ~vtkDiscreteModelVertex();

  friend class vtkDiscreteModel;
  friend class vtkModel;
  friend class vtkModelVertexUse;
  friend class vtkModelEdgeUse;
  friend class vtkModelEdge;
  friend class vtkXMLModelReader;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCMBMapToCMBModel;

private:
  vtkDiscreteModelVertex(const vtkDiscreteModelVertex&);  // Not implemented.
  void operator=(const vtkDiscreteModelVertex&);  // Not implemented.
};

#endif

