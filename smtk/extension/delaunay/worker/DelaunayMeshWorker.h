//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef __smtk_extension_delaunay_worker_MeshWorker_h
#define __smtk_extension_delaunay_worker_MeshWorker_h

#include "remus/worker/Worker.h"
#include "smtk/PublicPointerDefs.h"

//forward delcare AuxiliaryGeometry && ElementSizing && EdgeMeshes
class AuxiliaryGeometry;
class EdgeMeshes;
class ElementSizing;

//forward declare smtk::model::Model && Face
namespace smtk
{
namespace model
{
class Model;
class Face;
}
}

class DelaunayMeshWorker : public remus::worker::Worker
{
public:
  DelaunayMeshWorker(
    remus::worker::ServerConnection const& connection, remus::common::FileHandle const& fhandle);

  //will get a tetgen job from the remus server
  //and call tetgen
  void meshJob();

private:
  bool meshFace(const smtk::model::Face& face, bool validatePolygons,
    const smtk::mesh::CollectionPtr& collection);
};
#endif
