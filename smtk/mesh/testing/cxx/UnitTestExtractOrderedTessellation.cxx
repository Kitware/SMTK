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

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/ImportOperator.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

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

#include <fstream>

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::bridge::discrete::Resource::Ptr create_discrete_mesh_model()
{
  std::string file_path(data_root);
  file_path += "/mesh/2d/test2D.2dm";
  std::cout << "create_discrete_mesh_model of file: " << file_path << std::endl;

  std::ifstream file(file_path.c_str());
  if (!file.good())
  {
    return smtk::bridge::discrete::Resource::Ptr();
  }

  file.close();

  smtk::bridge::discrete::ImportOperator::Ptr op = smtk::bridge::discrete::ImportOperator::create();

  op->parameters()->findFile("filename")->setValue(file_path.c_str());
  smtk::bridge::discrete::ImportOperator::Result result = op->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cout << "Import 2dm Failed!" << std::endl;
  }

  return std::dynamic_pointer_cast<smtk::bridge::discrete::Resource>(
    result->findResource("resource")->value());
}

void removeOnesWithoutTess(smtk::model::EntityRefs& ents)
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

int UnitTestExtractOrderedTessellation(int, char** const)
{
  smtk::model::EntityRef eRef;
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::bridge::discrete::Resource::Ptr resource = create_discrete_mesh_model();

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::CollectionPtr c = convert(meshManager, resource);

  typedef smtk::model::EntityRefs EntityRefs;

  EntityRefs currentEnts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::FACE);
  removeOnesWithoutTess(currentEnts);
  if (currentEnts.empty())
  {
    std::cerr << "No tessellation!" << std::endl;
    return 1;
  }

  // We only extract the first face
  eRef = *currentEnts.begin();

  const smtk::model::Face& face = eRef.as<smtk::model::Face>();

  {
    //step 1 get the face use for the face
    // smtk::model::FaceUse fu = face.positiveUse();
    smtk::model::FaceUse fu = face.negativeUse();

    //check if we have an exterior loop
    smtk::model::Loops exteriorLoops = fu.loops();
    if (exteriorLoops.size() == 0)
    {
      //if we don't have loops we are bailing out!
      std::cerr << "No loops!" << std::endl;
      return 1;
    }

    smtk::model::Loop exteriorLoop = exteriorLoops[0];

    std::int64_t connectivityLength = -1;
    std::int64_t numberOfCells = -1;
    std::int64_t numberOfPoints = -1;

    //query for all cells
    smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
      exteriorLoop, c, connectivityLength, numberOfCells, numberOfPoints);

    std::vector<std::int64_t> conn(connectivityLength);
    std::vector<float> fpoints(numberOfPoints * 3);

    smtk::mesh::utility::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

    ftess.disableVTKStyleConnectivity(true);
    ftess.disableVTKCellTypes(true);

    smtk::mesh::utility::extractOrderedTessellation(exteriorLoop, c, ftess);
  }

  {
    smtk::model::Edges edges = face.edges();

    for (auto& edge : edges)
    {
      std::int64_t connectivityLength = -1;
      std::int64_t numberOfCells = -1;
      std::int64_t numberOfPoints = -1;

      //query for all cells
      smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
        edge, c, connectivityLength, numberOfCells, numberOfPoints);

      std::vector<std::int64_t> conn(connectivityLength);
      std::vector<float> fpoints(numberOfPoints * 3);

      smtk::mesh::utility::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

      ftess.disableVTKStyleConnectivity(true);
      ftess.disableVTKCellTypes(true);

      smtk::mesh::utility::extractOrderedTessellation(edge, c, ftess);
    }
  }

  return 0;
}
