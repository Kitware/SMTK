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

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkModelMultiBlockSource;
class vtkTable;

/**\brief Use SMTK to provide a ParaView-friendly attribute source.
  */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKAttributeReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  /// When the reader creates (true) and destroys (false) model resources, this function is called.
  using Observer = std::function<void(smtk::attribute::CollectionPtr, bool)>;

  vtkTypeMacro(vtkSMTKAttributeReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKAttributeReader* New();

  /// Set/get the URL of the SMTK model resource.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  /// Set/get whether to include the parent directory of \a FileName in the include path.
  vtkGetMacro(IncludePath, bool);
  vtkSetMacro(IncludePath, bool);

  /// Return the SMTK model resource that holds data read from \a FileName.
  smtk::attribute::CollectionPtr GetSMTKResource() const;

  void ObserveResourceChanges(const Observer& fn) { this->ResourceObserver = fn; }
  void UnobserveResourceChanges() { this->ResourceObserver = nullptr; }

protected:
  vtkSMTKAttributeReader();
  ~vtkSMTKAttributeReader() override;

  int RequestData(
    vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo) override;

  bool LoadFile();

  char* FileName;
  bool IncludePath;
  Observer ResourceObserver;
  smtk::attribute::CollectionPtr AttributeResource;
  vtkSmartPointer<vtkTable> Defs;

private:
  vtkSMTKAttributeReader(const vtkSMTKAttributeReader&) = delete;
  void operator=(const vtkSMTKAttributeReader&) = delete;
};

#endif
