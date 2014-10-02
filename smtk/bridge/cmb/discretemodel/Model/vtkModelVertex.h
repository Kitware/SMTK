//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelVertex - Abstract generic model vertex class.
// .SECTION Description

#ifndef __smtkcmb_vtkModelVertex_h
#define __smtkcmb_vtkModelVertex_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGeometricEntity.h"


class vtkModelItemIterator;
class vtkModelVertexUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelVertex : public vtkModelGeometricEntity
{
public:
  vtkTypeMacro(vtkModelVertex,vtkModelGeometricEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  // Description:
  // Pure virtual function to get the point location of the
  // model vertex.  Returns true for success and false for failure.
  // Fills the x, y, and z values in xyz if success.
  virtual bool GetPoint(double* xyz) = 0;

  virtual bool GetBounds(double bounds[6]);

  int GetNumberOfModelVertexUses();
  vtkModelItemIterator* NewModelVertexUseIterator();

  // Description:
  // Return an iterator for getting the adjacent model
  // edges of this model vertex.  The caller must make
  // sure that the iterator is deleted when finished.
  // Note that a model edge can "start" and "end" on
  // the same model vertex but this model edge
  // will only be listed once in the iterator.  The model
  // edges are ordered by their unique persistent Id
  // in order to get consistent results.
  vtkModelItemIterator* NewAdjacentModelEdgeIterator();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelVertex();
  virtual ~vtkModelVertex();

  virtual bool IsDestroyable();
  virtual bool Destroy();

  // Description:
  // Build and destroy a vtkModelVertexUse.  The vtkModelVertex
  // is responsible for the management of the associated
  // vtkModelVertexUse.
  vtkModelVertexUse* BuildModelVertexUse();
  void DestroyModelVertexUse(vtkModelVertexUse* vertexUse);
//BTX
  friend class vtkDiscreteModel;
  friend class vtkDiscreteModelGeometricEntity;
  friend class vtkModel;
  friend class vtkModelVertexUse;
  friend class vtkModelEdgeUse;
  friend class vtkModelEdge;
  friend class vtkXMLModelReader;
//ETX

private:
  vtkModelVertex(const vtkModelVertex&);  // Not implemented.
  void operator=(const vtkModelVertex&);  // Not implemented.
};

#endif

