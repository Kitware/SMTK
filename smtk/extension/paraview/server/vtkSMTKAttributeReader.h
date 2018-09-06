//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKAttributeReader_h
#define smtk_extension_paraview_server_vtkSMTKAttributeReader_h

#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkNew.h"
#include "vtkSmartPointer.h"
#include "vtkTrivialProducer.h"

class vtkSMTKWrapper;
class vtkTable;

/**\brief Use SMTK to provide a ParaView-friendly attribute source.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKAttributeReader : public vtkSMTKResourceGenerator
{
public:
  vtkTypeMacro(vtkSMTKAttributeReader, vtkSMTKResourceGenerator);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKAttributeReader* New();

  /// Set/get the URL of the SMTK attribute resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Set/get whether to include the parent directory of \a FileName in the include path.
  vtkSetMacro(IncludePathToFile, bool);
  vtkGetMacro(IncludePathToFile, bool);

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GenerateResource() const override;

protected:
  vtkSMTKAttributeReader();
  ~vtkSMTKAttributeReader() override;

  char* FileName;
  bool IncludePathToFile;

private:
  vtkSMTKAttributeReader(const vtkSMTKAttributeReader&) = delete;
  void operator=(const vtkSMTKAttributeReader&) = delete;
};

#endif
