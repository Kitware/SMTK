//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Reclassify.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Face.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include <sstream>
#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

//----------------------------------------------------------------------------
void create_simple_2d_model( smtk::model::ManagerPtr mgr )
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

//----------------------------------------------------------------------------
void add_model_edge_and_vert(smtk::model::ManagerPtr modelManager,
                             smtk::model::Edge& oe,
                             smtk::model::Edge& ne,
                             smtk::model::Vertex& nv)
{
  //verify that we only have a single model, that allows the subsequent logic
  //to be valid
  smtk::model::Models models = modelManager->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY);
  test(models.size() ==1, "should only have a single model");
  smtk::model::Model& model = models[0];

  //we need to find the edge whose vertices are:
  // {3, 0, 0}
  // {3, 5, 0}
  //and whose tessellation has 5 points ( length of 15 )
  //
  // because we are going to add a new model vertex at
  // {0, 2, 0}
  // and create a new edge
  //
  smtk::model::EntityRefs edges = modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::EDGE);
  smtk::model::EntityIterator it;
  it.traverse(edges.begin(), edges.end());
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    smtk::model::Edge edge(modelManager, it->entity());
    smtk::model::Vertices verts = edge.vertices();

    if(verts.size() != 2)
      { //not the edge you are looking for
      continue;
      }

    smtk::model::Vertex v1(modelManager, verts[0].entity());
    smtk::model::Vertex v2(modelManager, verts[1].entity());

    double* cv1 = v1.coordinates();
    double* cv2 = v2.coordinates();
    const smtk::model::Tessellation* tess = edge.gotMesh();

    if(cv1[0] == 3.0 && cv1[1] == 0.0 &&
       cv2[0] == 3.0 && cv2[1] == 5.0 &&
       tess->coords().size() == 15)
      {
      //mark this edge as our original edge
      oe = edge;

      //create the new vertex
      nv = modelManager->addVertex();
      model.addCell(nv);

      //vertex tess
      smtk::model::Tessellation vertTess;
      vertTess.addCoords(0, 2, 0);
      vertTess.addPoint(0);
      modelManager->setTessellationAndBoundingBox(nv.entity(), vertTess);


      ne = modelManager->addEdge();
      model.addCell(ne);

      //edge tess
      smtk::model::Tessellation edgeTess;
      edgeTess.addCoords(0, 2, 0);
      edgeTess.addCoords(1, 0, 0);
      edgeTess.addCoords(3, 0, 0);
      edgeTess.addLine(0, 1);
      edgeTess.addLine(1, 2);
      modelManager->setTessellationAndBoundingBox(ne.entity(), edgeTess);

      break;
      }
    }
}

struct xyz_view
{
  double* m_ptr;

  xyz_view(): m_ptr(NULL) {}
  xyz_view(double* ptr): m_ptr(ptr) {}

  //ignore z as our dataset is all axis aligned
  bool operator<(const xyz_view& r) const
  { return this->m_ptr[0] != r.m_ptr[0] ? (this->m_ptr[0] < r.m_ptr[0]) : (this->m_ptr[1] < r.m_ptr[1]); }
};

//----------------------------------------------------------------------------
void all_points_are_valid(smtk::mesh::CollectionPtr collection)
{
  //no two points should be identical, that is our litmus test
  //we know that we can't have duplicate points because
  smtk::mesh::PointSet ps = collection->points();
  std::vector<double> points(ps.size()*3);
  ps.get(&points[0]);


  std::set<xyz_view> unique_xyz;
  for(std::size_t i=0; i < ps.size(); ++i)
    {
    unique_xyz.insert( xyz_view( &points[0] + i*3) );
    }

  test( (ps.size() == unique_xyz.size()), "sizes should match");
}

//----------------------------------------------------------------------------
void verify_split()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);

  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have 21 mesh elements");
  test( c->points().size() == 32, "should have 32 points");
  test( c->cells(smtk::mesh::Dims0).size() == 7, "should have 7 vertex cells");
  test( c->cells(smtk::mesh::Dims1).size() == 32, "should have 32 edge cells");
  all_points_are_valid(c);

  //we need to split the model first.
  //this is done by manually doing the following.
  //create a new model edge and model vertex
  smtk::model::Edge originalEdge;
  smtk::model::Edge newEdge;
  smtk::model::Vertex promotedVertex;
  add_model_edge_and_vert(modelManager, originalEdge, newEdge, promotedVertex);

  bool valid = smtk::mesh::split(c, originalEdge, newEdge, promotedVertex);
  test( valid, "split should pass");

  all_points_are_valid(c);
  test( c->points().size() == 32, "should still have 32 points after split");
  test( c->numberOfMeshes() == 23, "collection should have 23 mesh elements after split");
  test( c->cells(smtk::mesh::Dims0).size() == 8, "should now have 8 vertex cells");
  test( c->cells(smtk::mesh::Dims1).size() == 32, "should have 32 edge cells");
}

//----------------------------------------------------------------------------
void verify_merge()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_2d_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == 21, "collection should have 21 mesh elements");
  test( c->points().size() == 32, "should have 32 points before split and merge");
  all_points_are_valid(c);

  //we need to split the model first.
  //this is done by manually doing the following.
  //create a new model edge and model vertex
  smtk::model::Edge originalEdge;
  smtk::model::Edge newEdge;
  smtk::model::Vertex promotedVertex;
  add_model_edge_and_vert(modelManager, originalEdge, newEdge, promotedVertex);

  bool svalid = smtk::mesh::split(c, originalEdge, newEdge, promotedVertex);
  test( svalid, "split should pass");
  bool mvalid = smtk::mesh::merge(c, promotedVertex, newEdge, originalEdge);
  test( mvalid, "merge should pass");

  all_points_are_valid(c);
  test( c->points().size() == 32, "should have 32 points after split&merge");
  test( c->numberOfMeshes() == 21, "collection should have 23 mesh elements after split");
  test( c->cells(smtk::mesh::Dims0).size() == 7, "should now have 7 vertex cells");
  test( c->cells(smtk::mesh::Dims1).size() == 32, "should have 32 edge cells");
}

}

//----------------------------------------------------------------------------
int UnitTestReclassifyEdges(int, char** const)
{
  verify_split();
  verify_merge();

  return 0;
}
