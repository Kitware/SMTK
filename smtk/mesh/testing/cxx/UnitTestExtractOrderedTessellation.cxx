//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/discrete/Operator.h"
#include "smtk/bridge/discrete/Session.h"

#include <fstream>

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

//----------------------------------------------------------------------------
void create_simple_mesh_model( smtk::model::ManagerPtr mgr )
{
  std::string file_path(data_root);
  file_path += "/model/2d/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::ImportJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

//------------------------------------------------------------------------------
void create_discrete_mesh_model( smtk::model::ManagerPtr mgr )
{
  std::string file_path(data_root);
  file_path += "/mesh/2d/test2D.2dm";
  std::cout << "create_discrete_mesh_model of file: " << file_path << std::endl;

  std::ifstream file(file_path.c_str());
  if(file.good())
    { //just make sure the file exists
    file.close();

    smtk::bridge::discrete::Session::Ptr brg =
      smtk::bridge::discrete::Session::create();
    mgr->registerSession(brg);

    smtk::model::Operator::Ptr op = brg->op("import");

    op->findFile("filename")->setValue(file_path.c_str());
    smtk::model::OperatorResult result = op->operate();
    if (result->findInt("outcome")->value() !=  smtk::model::OPERATION_SUCCEEDED)
      {
      std::cout << "Import 2dm Failed!" << std::endl;
      }
    }
}

//----------------------------------------------------------------------------
void removeOnesWithoutTess(smtk::model::EntityRefs& ents)
{
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  std::vector< smtk::model::EntityRef > withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    if(!it->hasTessellation())
      {
      withoutTess.push_back(it.current());
      }
    }

  typedef std::vector< smtk::model::EntityRef >::const_iterator c_it;
  for(c_it i=withoutTess.begin(); i < withoutTess.end(); ++i)
    {
    ents.erase(*i);
    }
}

}

//----------------------------------------------------------------------------
int UnitTestExtractOrderedTessellation(int, char** const)
{
  // Somehow grab an EntityRef with an associated tessellation
  smtk::model::EntityRef eRef;
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  // rather than remove this code path, I will simply shunt it to satisfy
  // warnings about unused functions.
  if (false)
    {
    create_simple_mesh_model(modelManager);
    }
  else
    {
    create_discrete_mesh_model(modelManager);
    }

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);

  typedef smtk::model::EntityRefs EntityRefs;

  EntityRefs currentEnts =
    modelManager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::FACE);
  removeOnesWithoutTess(currentEnts);
  if (currentEnts.empty())
    {
    std::cerr<<"No tessellation!"<<std::endl;
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
  if(exteriorLoops.size() == 0)
    {
    //if we don't have loops we are bailing out!
    std::cerr<<"No loops!"<<std::endl;
    return 1;
    }

  smtk::model::Loop exteriorLoop = exteriorLoops[0];

  std::int64_t connectivityLength= -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    exteriorLoop, c, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn( connectivityLength );
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);

  smtk::mesh::extractOrderedTessellation(exteriorLoop, c, ftess);
  }

  {
  smtk::model::Edges edges = face.edges();

  for (auto& edge : edges)
    {
    std::int64_t connectivityLength= -1;
    std::int64_t numberOfCells = -1;
    std::int64_t numberOfPoints = -1;

    //query for all cells
    smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
      edge, c, connectivityLength, numberOfCells, numberOfPoints);

    std::vector<std::int64_t> conn( connectivityLength );
    std::vector<float> fpoints(numberOfPoints * 3);

    smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

    ftess.disableVTKStyleConnectivity(true);
    ftess.disableVTKCellTypes(true);

    smtk::mesh::extractOrderedTessellation(edge, c, ftess);
    }
  }

  return 0;
}
