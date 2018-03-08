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

#include "smtk/bridge/polygon/RegisterSession.h"
#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/LegacyRead.h"

#include "smtk/extension/delaunay/operators/TessellateFaces.h"

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

int UnitTestTessellateFaces(int, char** const)
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
    operationManager->registerOperation<smtk::bridge::polygon::LegacyRead>(
      "smtk::bridge::polygon::LegacyRead");
    operationManager->registerOperation<smtk::extension::delaunay::TessellateFaces>(
      "smtk::extension::delaunay::TessellateFaces");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::bridge::polygon::Resource::Ptr resource;

  {
    // Create an import operator
    smtk::bridge::polygon::LegacyRead::Ptr readOp =
      operationManager->create<smtk::bridge::polygon::LegacyRead>();
    if (!readOp)
    {
      std::cerr << "No read operator\n";
      return 1;
    }

    std::string file_path(data_root);
    file_path += "/mesh/2d/boxWithHole.smtk";

    std::ifstream file(file_path.c_str());
    if (file.good())
    { //just make sure the file exists
      file.close();

      readOp->parameters()->findFile("filename")->setValue(file_path.c_str());
      smtk::bridge::polygon::LegacyRead::Result result = readOp->operate();
      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Could not read smtk model!\n";
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

    smtk::extension::delaunay::TessellateFaces::Ptr tessellateFacesOp =
      operationManager->create<smtk::extension::delaunay::TessellateFaces>();
    if (!tessellateFacesOp)
    {
      std::cerr << "No tessellate faces operator\n";
      return 1;
    }

    tessellateFacesOp->parameters()->associateEntity(face);

    if (!tessellateFacesOp->ableToOperate())
    {
      std::cerr << "Tessellate faces operator cannot operate\n";
      return 1;
    }

    smtk::extension::delaunay::TessellateFaces::Result result = tessellateFacesOp->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Tessellate faces operator failed\n";
      return 1;
    }

    const smtk::model::Face& tessellatedFace = result->findModelEntity("tess_changed")->value();
    if (face != tessellatedFace)
    {
      std::cerr << "Tessellate faces operator did something strange\n";
      return 1;
    }

    const smtk::model::Tessellation* tess = tessellatedFace.hasTessellation();
    if (!tess)
    {
      std::cerr << "Tessellate faces operator did not create a tessellation\n";
      return 1;
    }

    if (tess->coords().size() != 8 * 3 || tess->conn().size() != 8 * 4)
    {
      std::cerr << "Tessellate faces operator did something wrong\n";
      return 1;
    }
  }

  return 0;
}
