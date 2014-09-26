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
// .NAME vtkModelVertexUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkModelVertexUse_h
#define __vtkModelVertexUse_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"
#include "cmbSystemConfig.h"

class vtkModelEdgeUse;
class vtkModelVertex;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelVertexUse : public vtkModelEntity
{
public:
  static vtkModelVertexUse* New();
  vtkTypeMacro(vtkModelVertexUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  vtkModelVertex* GetModelVertex();

  int GetNumberOfModelEdgeUses();
  vtkModelItemIterator* NewModelEdgeUseIterator();

  using Superclass::Initialize;
  void Initialize(vtkModelVertex* vertex);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelVertexUse();
  virtual ~vtkModelVertexUse();


  void AddModelEdgeUse(vtkModelEdgeUse* edgeUse);
  void RemoveModelEdgeUse(vtkModelEdgeUse* edgeUse);
//BTX
  friend class vtkModelEdge;
  friend class vtkModelEdgeUse;
//ETX

  virtual bool Destroy();
  // for destroying
//BTX
  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkModelVertex;
//ETX

private:
  vtkModelVertexUse(const vtkModelVertexUse&);  // Not implemented.
  void operator=(const vtkModelVertexUse&);  // Not implemented.
};

#endif

