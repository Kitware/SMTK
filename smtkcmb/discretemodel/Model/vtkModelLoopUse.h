/*=========================================================================

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
// .NAME vtkModelLoopUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkModelLoopUse_h
#define __vtkModelLoopUse_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"
#include "cmbSystemConfig.h"

class vtkModelEdgeUse;
class vtkModelFace;
class vtkModelItemIterator;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelLoopUse : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelLoopUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  int GetNumberOfModelEdgeUses();
  vtkModelEdgeUse* GetModelEdgeUse(int index);
  vtkModelItemIterator* NewModelEdgeUseIterator();

  vtkModelFace* GetModelFace();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Get the index of edgeUse in this loop.  Returns -1
  // if edgeUse is not associated with this loop.
  int GetModelEdgeUseIndex(vtkModelEdgeUse* edgeUse);

  // Description:
  // Get the number of vertices in this loop.  Note that
  // if a vertex is in the loop more than once it is still
  // only counted once.
  int GetNumberOfModelVertices();

protected:
  static vtkModelLoopUse* New();
  vtkModelLoopUse();
  virtual ~vtkModelLoopUse();

  void InsertModelEdgeUse(int Index, vtkModelEdgeUse* edgeUse);

  void RemoveModelEdgeUseAssociation(vtkModelEdgeUse* edgeUse);

//BTX
  friend class vtkDiscreteModelGeometricEntity;
  friend class vtkXMLModelReader;
  friend class vtkModelEdge;
  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkModel;
//ETX

  virtual bool Destroy();

private:
  vtkModelLoopUse(const vtkModelLoopUse&);  // Not implemented.
  void operator=(const vtkModelLoopUse&);  // Not implemented.
};

#endif

