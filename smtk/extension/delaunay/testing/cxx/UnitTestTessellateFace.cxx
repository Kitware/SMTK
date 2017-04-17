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

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/Session.h"

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

int UnitTestTessellateFace(int, char** const)
{
  // Somehow grab an EntityRef with an associated tessellation
  smtk::model::EntityRef eRef;
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  smtk::bridge::polygon::Session::Ptr session =
    smtk::bridge::polygon::Session::create();
  modelManager->registerSession(session);

  smtk::model::Model model;
  {
    std::string file_path(data_root);
    file_path += "/mesh/2d/boxWithHole.smtk";

    std::ifstream file(file_path.c_str());
    if(file.good())
    { //just make sure the file exists
      file.close();

      smtk::model::Operator::Ptr op = session->op("load smtk model");

      op->findFile("filename")->setValue(file_path.c_str());
      smtk::model::OperatorResult result = op->operate();
      if (result->findInt("outcome")->value() !=  smtk::model::OPERATION_SUCCEEDED)
      {
      std::cerr << "Could not load smtk model!\n";
      return 1;
      }
      model = result->findModelEntity("mesh_created")->value();
    }
  }

  {
    smtk::model::EntityRefs currentEnts =
      modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(
        smtk::model::FACE);
    removeRefsWithoutTess(currentEnts);
    if (currentEnts.empty())
    {
      std::cerr<<"No tessellation!"<<std::endl;
      return 1;
    }

    // We only extract the first face
    eRef = *currentEnts.begin();

    const smtk::model::Face& face = eRef.as<smtk::model::Face>();

    if (!face.isValid())
    {
      std::cerr << "Face is invald\n";
      return 1;
    }

    smtk::model::OperatorPtr tessellateFace = session->op("tessellate face");
    if (!tessellateFace)
    {
      std::cerr << "No tessellate face operator\n";
      return 1;
    }
    tessellateFace->specification()->associateEntity(model);
    tessellateFace->specification()->findModelEntity("face")->
      setValue(face);

    if (!tessellateFace->ableToOperate())
    {
      std::cerr << "Tessellate face operator cannot operate\n";
      return 1;
    }

    smtk::model::OperatorResult result = tessellateFace->operate();
    if (result->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Tessellate face operator failed\n";
      return 1;
    }

    const smtk::model::Face& tessellatedFace =
      result->findModelEntity("tess_changed")->value();
    if (face != tessellatedFace)
    {
      std::cerr << "Tessellate face operator did something strange\n";
      return 1;
    }

    const smtk::model::Tessellation* tess = tessellatedFace.hasTessellation();
    if (!tess)
    {
      std::cerr << "Tessellate face operator did not create a tessellation\n";
      return 1;
    }

    if (tess->coords().size() != 8*3 ||
        tess->conn().size() != 8*4)
    {
      std::cerr << "Tessellate face operator did something wrong\n";
      return 1;
    }
  }

  return 0;
}

smtkComponentInitMacro(smtk_delaunay_tessellate_face_operator);
