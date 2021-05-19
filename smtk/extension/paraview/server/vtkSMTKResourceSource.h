//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceSource_h
#define smtk_extension_paraview_server_vtkSMTKResourceSource_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/paraview/server/vtkSMTKResource.h"

#include "vtkMultiBlockDataSetAlgorithm.h"

/**\brief A class for SMTK-based sources exposed in ParaView.
 *
 * SMTK resources are represented in ParaView pipelines as the result of a
 * vtkSMTKResourceSource algorithm. Internally, vtkSMTKResourceSource contains a
 * resource generating class (vtkSMTKResourceGenerator) whose job is to
 * construct a resource and create a vtkMultiBlockDataSet from the resource. The
 * resulting resource is then manipulated in situ by SMTK operations; these
 * operations flag the source (vtkSMTKResourceSource) for update. By
 * encapsulating the resource generator within this class, we can trigger an
 * execution of the visualizaion pipeline without affecting a regeneration of
 * the resource. The resource will be regenerated only if the exposed methods of
 * the resource generator (e.g. FileName) are modified. This design is based off
 * of ParaView's vtkMetaReader.
 */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkSMTKResourceSource, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKResourceSource* New();

  vtkSMTKResourceSource(const vtkSMTKResourceSource&) = delete;
  vtkSMTKResourceSource& operator=(const vtkSMTKResourceSource&) = delete;

  /**
   * Set/get the internal resource.
   */
  vtkSetObjectMacro(VTKResource, vtkSMTKResource);
  vtkGetObjectMacro(VTKResource, vtkSMTKResource);

  /**
   * Return the MTime when also considering the internal resource.
   */
  vtkMTimeType GetMTime() override;
  void Modified() override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

protected:
  int FillOutputPortInformation(int port, vtkInformation* info) override;

  vtkSMTKResourceSource();
  ~vtkSMTKResourceSource() override;

  vtkSMTKResource* VTKResource{ nullptr };
};

#endif
