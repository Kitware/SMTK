//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelFaceUse -
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelFaceUse_h
#define __smtkdiscrete_vtkModelFaceUse_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"


class vtkModelEdgeUse;
class vtkModelFace;
class vtkModelLoopUse;
class vtkModelShellUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelFaceUse : public vtkModelEntity
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

