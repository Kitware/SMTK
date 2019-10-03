//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResource_h
#define smtk_extension_paraview_server_vtkSMTKResource_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/resource/Resource.h"

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkSMTKWrapper;

/**\brief A base class for manipulating SMTK resources in ParaView.
 *
 * SMTK resources are presented through ParaView as the result of
 * vtkSMTKResourceSource, but the actual ownership of the resource and its
 * conversion into a vtkMultiBlockDataSet is handled by vtkSMTKResource and its
 * derived classes. The source and resource are separated into two different
 * filters to facilitate the update of the source filter without triggering a
 * regeneration of the resource.
 */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResource : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKResource, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKResource* New();

  /// Set the SMTK resource by the string representation of its resource Id.
  void SetResourceById(const char* resourceIdStr);

  /// Set/get the SMTK resource.
  void SetResource(const smtk::resource::ResourcePtr& resource);
  smtk::resource::ResourcePtr GetResource() const { return Resource; }

  /// This accessor facilitates the lazy construction of the converter.
  virtual vtkAlgorithm* GetConverter();

  /// Set/get the SMTK wrapper to which the resource is added (and whose
  /// operators are used to read)
  virtual void SetWrapper(vtkSMTKWrapper*);
  vtkGetObjectMacro(Wrapper, vtkSMTKWrapper);

  /// Remove any loaded resource from the resource manager being used by the reader.
  virtual void DropResource();

protected:
  vtkSMTKResource();
  ~vtkSMTKResource() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  smtk::resource::ResourcePtr Resource;
  smtk::common::UUID ResourceId;
  vtkSmartPointer<vtkAlgorithm> Converter;
  vtkSMTKWrapper* Wrapper;

private:
  vtkSMTKResource(const vtkSMTKResource&) = delete;
  void operator=(const vtkSMTKResource&) = delete;
};

#endif
