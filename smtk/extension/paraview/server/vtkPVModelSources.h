//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkPVModelSources_h
#define smtk_extension_paraview_server_vtkPVModelSources_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/model/EntityRef.h"

#include "vtkObject.h"

class vtkDataObject;
class vtkMultiBlockDataSetAlgorithm;
class vtkModelMultiBlockSource;
class vtkResourceMultiBlockSource;

/**\brief Track instances of vtk{Model,Mesh}MultiBlockSource on the Paraview server.
  *
  * This object, exposed to ParaView (PV) by the smconfig.xml file in this directory,
  * is intended to be constructed as a singleton on the server and informed by
  * your PV-based application whenever a vtk{Model,Mesh}MultiBlockSource instance is
  * constructed or destroyed.
  *
  * Server-side extensions can then ask this singleton for the multiblock holding
  * graphics primitives for any model or mesh entity by UUID, making it possible
  * to use VTK filters for queries on renderable entities.
  */
class SMTKPVSERVEREXT_EXPORT vtkPVModelSources : public vtkObject
{
public:
  static vtkPVModelSources* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkPVModelSources, vtkObject);

  static vtkPVModelSources* GetInstance();

  /*
  virtual bool AddSource(vtkModelMultiBlockSource*);
  virtual bool AddSource(vtkResourceMultiBlockSource*);
  virtual bool RemoveSource(vtkModelMultiBlockSource*);
  virtual bool RemoveSource(vtkResourceMultiBlockSource*);
  virtual bool RemoveAllSources();
  */

  static std::pair<vtkMultiBlockDataSetAlgorithm*, vtkIdType> findModelEntitySource(
    const smtk::model::EntityRef&);
  /*
  static std::pair<vtkResourceMultiBlockSource*, vtkIdType> findMeshSetSource(
    const smtk::mesh::MeshSet&);
*/
  static vtkDataObject* findModelEntity(const smtk::model::EntityRef&);
  /*
  static vtkDataObject* findMeshSet(const smtk::mesh::MeshSet&);
*/

protected:
  vtkPVModelSources();
  virtual ~vtkPVModelSources();

  class Internal;
  Internal* m_p;

  static void destroySingleton();
  static vtkPVModelSources* s_instance;
};

#endif
