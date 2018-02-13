//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/polygon/Operation.h"
#include "smtk/bridge/polygon/RegisterSession.h"
#include "smtk/bridge/polygon/Resource.h"

#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/mesh/utility/ExtractTessellation.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/operation/LoadResource.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

#include <fstream>

namespace
{

//SMTK_DATA_DIR and SMTK_SCRATCH_DIR are defined by cmake
std::string data_root = SMTK_DATA_DIR;
std::string scratch_root = SMTK_SCRATCH_DIR;

void removeRefsWithoutTess(smtk::model::EntityRefs& ents)
{
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  std::vector<smtk::model::EntityRef> withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    if (!it->hasTessellation())
    {
      withoutTess.push_back(it.current());
    }
  }

  typedef std::vector<smtk::model::EntityRef>::const_iterator c_it;
  for (c_it i = withoutTess.begin(); i < withoutTess.end(); ++i)
  {
    ents.erase(*i);
  }
}
}

int UnitTestTriangulateFaces(int, char** const)
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  {
    smtk::bridge::polygon::registerResources(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import and write operators to the operation manager
  {
    operationManager->registerOperation<smtk::operation::LoadResource>(
      "smtk::operation::LoadResource");
    operationManager->registerOperation<smtk::extension::delaunay::TriangulateFaces>(
      "smtk::extension::delaunay::TriangulateFaces");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::bridge::polygon::Resource::Ptr resource;

  {
    // Create an import operator
    smtk::operation::LoadResource::Ptr loadOp =
      operationManager->create<smtk::operation::LoadResource>();
    if (!loadOp)
    {
      std::cerr << "No load operator\n";
      return 1;
    }

    std::string file_path(data_root);
    file_path += "/mesh/2d/boxWithHole.smtk";

    std::ifstream file(file_path.c_str());
    if (file.good())
    { //just make sure the file exists
      file.close();

      loadOp->parameters()->findFile("filename")->setValue(file_path.c_str());
      smtk::operation::LoadResource::Result result = loadOp->operate();
      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Could not load smtk model!\n";
        return 1;
      }

      resource = smtk::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(
        result->findResource("resource")->value(0));
    }
  }

  {
    smtk::model::EntityRefs currentEnts =
      resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::FACE);
    removeRefsWithoutTess(currentEnts);
    if (currentEnts.empty())
    {
      std::cerr << "No tessellation!" << std::endl;
      return 1;
    }

    // We only extract the first face
    auto eRef = *currentEnts.begin();

    const smtk::model::Face& face = eRef.as<smtk::model::Face>();

    if (!face.isValid())
    {
      std::cerr << "Face is invald\n";
      return 1;
    }

    smtk::extension::delaunay::TriangulateFaces::Ptr triangulateFacesOp =
      operationManager->create<smtk::extension::delaunay::TriangulateFaces>();
    if (!triangulateFacesOp)
    {
      std::cerr << "No triangulate faces operator\n";
      return 1;
    }

    triangulateFacesOp->parameters()->associateEntity(face);

    if (!triangulateFacesOp->ableToOperate())
    {
      std::cerr << "Triangulate face operator cannot operate\n";
      return 1;
    }

    smtk::extension::delaunay::TriangulateFaces::Result result = triangulateFacesOp->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Triangulate face operator failed\n";
      return 1;
    }

    const smtk::model::Model& meshedModel = result->findModelEntity("mesh_created")->value();

    if (face.owningModel() != meshedModel)
    {
      std::cerr << "Triangulate face operator did something strange\n";
      return 1;
    }

    auto associatedCollections = face.manager()->meshes()->associatedCollections(face);
    smtk::mesh::CollectionPtr triangulatedFace = associatedCollections[0];

    if (triangulatedFace->points().size() != 8 || triangulatedFace->cells().size() != 8)
    {
      std::cerr << "Triangulate faces operator did something wrong\n";
      return 1;
    }

    if (false)
    {
      smtk::io::ExportMesh exportMesh;
      std::string output_path(scratch_root);
      output_path += "/boxWithHole.vtk";
      exportMesh(output_path, triangulatedFace);
    }
  }

  return 0;
}
