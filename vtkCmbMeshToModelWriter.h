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
// .NAME vtkCmbMeshToModelWriter - outputs a m2m file in XML format
// .SECTION Description
// Filter to output an XML file with mapping from analysis mesh info to
// model topology.

#ifndef __vtkCmbMeshToModelWriter_h
#define __vtkCmbMeshToModelWriter_h

#include "vtkXMLWriter.h"

class vtkCMBModelWrapper;

class VTK_EXPORT vtkCmbMeshToModelWriter : public vtkXMLWriter
{
public:
  static vtkCmbMeshToModelWriter *New();
  vtkTypeRevisionMacro(vtkCmbMeshToModelWriter,vtkXMLWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetDefaultFileExtension();

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

  // Description:
  // Set/get functions for the ModelWrapper.
//BTX
  vtkGetMacro(ModelWrapper, vtkCMBModelWrapper*);
//ETX
  void SetModelWrapper(vtkCMBModelWrapper* Wrapper);

protected:
  vtkCmbMeshToModelWriter();
  ~vtkCmbMeshToModelWriter();

  virtual int WriteData();
  virtual int WriteHeader(vtkIndent* parentindent);
  virtual int Write3DModelMeshInfo(vtkIndent* indent);
  virtual int Write2DModelMeshInfo(vtkIndent* indent);
  virtual int WriteFooter(vtkIndent* parentindent);

  const char* GetDataSetName();

private:
  vtkCmbMeshToModelWriter(const vtkCmbMeshToModelWriter&);  // Not implemented.
  void operator=(const vtkCmbMeshToModelWriter&);  // Not implemented.

  // Description:
  // The vtkCMBModelWrapper for the algorithm to extract the model
  // information from.
  vtkCMBModelWrapper* ModelWrapper;

};

#endif
