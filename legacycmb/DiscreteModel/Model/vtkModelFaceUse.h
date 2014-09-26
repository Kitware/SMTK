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
// .NAME vtkModelFaceUse -
// .SECTION Description

#ifndef __vtkModelFaceUse_h
#define __vtkModelFaceUse_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"
#include "cmbSystemConfig.h"

class vtkModelEdgeUse;
class vtkModelFace;
class vtkModelLoopUse;
class vtkModelShellUse;

class VTKDISCRETEMODEL_EXPORT vtkModelFaceUse : public vtkModelEntity
{
public:
  static vtkModelFaceUse *New();
  vtkTypeMacro(vtkModelFaceUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  // Description:
  // Adjacency information.
  vtkModelShellUse* GetModelShellUse();
  vtkModelFace* GetModelFace();
  vtkModelLoopUse* GetOuterLoopUse();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Get all of the  loop uses for the model face use.
  int GetNumberOfLoopUses();
  vtkModelItemIterator* NewLoopUseIterator();

protected:
  vtkModelFaceUse();
  virtual ~vtkModelFaceUse();

  // Description:
  // Add a loop to the face use.
  void AddLoopUse(vtkModelLoopUse* loopUse);
  //bool DestroyModelLoopUse(vtkModelLoopUse* LoopUse);
  virtual bool Destroy();
  virtual bool DestroyLoopUses();

private:
  vtkModelFaceUse(const vtkModelFaceUse&);  // Not implemented.
  void operator=(const vtkModelFaceUse&);  // Not implemented.

//BTX
  friend class vtkXMLModelReader;
  friend class vtkModel;
  friend class vtkModelShellUse;
  friend class vtkModelFace;
//ETX

};

#endif

