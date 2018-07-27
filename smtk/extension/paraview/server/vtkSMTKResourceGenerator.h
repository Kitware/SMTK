//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceGenerator_h
#define smtk_extension_paraview_server_vtkSMTKResourceGenerator_h

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief An abstract base class for generating SMTK resources in ParaView.
 *
 * SMTK resources are presented through ParaView as the result of vtkSMTKSource,
 * but the actual construction of the resource and its conversion into a
 * vtkMultiBlockDataSet is handled by classes derived from
 * vtkSMTKResourceGenerator. The source and generation actions are separated
 * into two different filters to facilitate the update of the source filter
 * without triggering a regeneration of the resource.
 */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceGenerator : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKResourceGenerator, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Return the SMTK resource that holds data read from \a FileName.
  virtual smtk::resource::ResourcePtr GetResource() const;

  /// Set/get the SMTK wrapper to which the resource is added (and whose
  /// operators are used to read)
  virtual void SetWrapper(vtkSMTKWrapper*);
  vtkGetObjectMacro(Wrapper, vtkSMTKWrapper);

  /// Remove any loaded resource from the resource manager being used by the reader.
  virtual void DropResource();

  /// Return the VTK algorithm used to convert the SMTK resource into a multiblock.
  virtual vtkAlgorithm* GetConverter() const = 0;

protected:
  vtkSMTKResourceGenerator();
  ~vtkSMTKResourceGenerator() override;

  vtkSMTKWrapper* Wrapper;

private:
  vtkSMTKResourceGenerator(const vtkSMTKResourceGenerator&) = delete;
  void operator=(const vtkSMTKResourceGenerator&) = delete;
};

#endif
