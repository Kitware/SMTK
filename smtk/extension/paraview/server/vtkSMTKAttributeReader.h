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

#include "smtk/extension/paraview/server/vtkSMTKResourceReader.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

class vtkTable;

/**\brief Use SMTK to provide a ParaView-friendly attribute source.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKAttributeReader : public vtkSMTKResourceReader
{
public:
  vtkTypeMacro(vtkSMTKAttributeReader, vtkSMTKResourceReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKAttributeReader* New();

  /// Set/get whether to include the parent directory of \a FileName in the include path.
  vtkGetMacro(IncludePath, bool);
  vtkSetMacro(IncludePath, bool);

  /// Return the SMTK resource that holds data read from \a FileName.
  smtk::resource::ResourcePtr GetResource() const override;

  /// Return the SMTK attribute resource that holds data read from \a FileName.
  smtk::attribute::CollectionPtr GetSMTKResource() const;

protected:
  vtkSMTKAttributeReader();
  ~vtkSMTKAttributeReader() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool LoadFile();

  bool IncludePath;
  smtk::attribute::CollectionPtr AttributeResource;
  vtkSmartPointer<vtkTable> Defs;

private:
  vtkSMTKAttributeReader(const vtkSMTKAttributeReader&) = delete;
  void operator=(const vtkSMTKAttributeReader&) = delete;
};

#endif
