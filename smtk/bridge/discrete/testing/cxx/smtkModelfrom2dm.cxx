//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/io/ReadMesh.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/UUID.h"

#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "smtk/bridge/discrete/Registrar.h"
#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/bridge/discrete/operators/EdgeOperation.h"
#include "smtk/bridge/discrete/operators/ImportOperation.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/operation/Registrar.h"

#include <fstream>
#include <sstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string write_root = SMTK_SCRATCH_DIR;

void cleanupsmtkfiles(const std::string& file_path, const std::string& meshname)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }

  // clean up also the cmb model and h5m mesh file
  std::string smtkfilepath = path.parent_path().string();
  std::string smtkfilename = path.stem().string();

  std::ostringstream outfilename;
  outfilename << smtkfilename << ".cmb";
  path = ::boost::filesystem::path(smtkfilepath) / ::boost::filesystem::path(outfilename.str());
  std::cout << "Should remove " << path.string() << "\n";
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }

  std::ostringstream meshfilename;
  meshfilename << smtkfilename << "_" << meshname << ".h5m";
  path = ::boost::filesystem::path(smtkfilepath) / ::boost::filesystem::path(meshfilename.str());
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
}

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  if (argc == 1)
  {
    std::cout << "Not enough arguments" << std::endl;
    return 1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if (!file.good())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }
  file.close();

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  {
    smtk::bridge::discrete::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  {
    smtk::operation::Registrar::registerTo(operationManager);
    smtk::bridge::discrete::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create an import operator
  smtk::bridge::discrete::ImportOperation::Ptr importOp =
    operationManager->create<smtk::bridge::discrete::ImportOperation>();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  // Set the file path
  importOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  smtk::operation::Operation::Result importOpResult = importOp->operate();

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  // Test for success
  if (importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Manager::Ptr resource =
    std::static_pointer_cast<smtk::model::Manager>(model->resource());
  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading 2dm file failed!\n";
    return 1;
  }

  return 0;

  typedef std::vector<smtk::mesh::CollectionPtr> AssocCollections;

  smtk::mesh::ManagerPtr meshmgr = resource->meshes();
  AssocCollections assocCollections = meshmgr->collectionsWithAssociations();
  test(assocCollections.size() == 2, "expecting 2 mesh collections");

  smtk::mesh::CollectionPtr mc = assocCollections[0];
  if (mc->entity() == model2dm.entity())
  {
    // this collection has the same entity id as the model. It holds the
    // tessellation meshes for each model entity. We are looking for the
    // mesh that is affiliated with the model, not the one that represents
    // its tessellation.
    mc = assocCollections[1];
  }
  test((mc->meshes(smtk::mesh::Dims2)).size() == 4, "Expecting 4 face mesh");
  test((mc->meshes(smtk::mesh::Dims1)).size() == 10, "Expecting 10 edge mesh");
  test((mc->meshes(smtk::mesh::Dims0)).size() == 7, "Expecting 7 vertex mesh");

  // write out the smtk model after loading 2dm,
  // which uses mergeCoincidentContactPoints() in meshes
  std::string write_path(write_root);
  write_path += "/test2D_2dm_save.smtk";

  std::ostringstream model_path;
  model_path << write_root << "/test2D_2dm_save.cmb";
  model2dm.setStringProperty("url", model_path.str());

  std::ostringstream mesh_path;
  mesh_path << write_root << "/test2D_2dm_save_" << mc->name() << ".h5m";

  smtk::operation::WriteResource::Ptr writeOp =
    operationManager->create<smtk::operation::WriteResource>();

  writeOp->parameters()->findFile("filename")->setValue(write_path);
  writeOp->parameters()->findResource("resource")->setValue(resource);

  //write out the smtk model.
  auto result = writeOp->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Save resource failed: " << write_path << std::endl;
    return 1;
  }

  // Erase the original model before loading the saved smtk model
  resource->eraseModel(model2dm);

  smtk::operation::ReadResource::Ptr readOp =
    operationManager->create<smtk::operation::ReadResource>();
  readOp->parameters()->findFile("filename")->setValue(write_path);
  result = readOp->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    cleanupsmtkfiles(write_path, mc->name());
    std::cerr << "Read resource failed: " << write_path << std::endl;
    return 1;
  }
  //remove the file from disk
  cleanupsmtkfiles(write_path, mc->name());

  smtk::mesh::CollectionPtr c2;

  smtk::model::Manager::Ptr smtkresource2dm;
  Model smtkmodel2dm;
  {
    smtk::attribute::ResourceItemPtr resItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(result->findResource("resource"));

    smtkresource2dm = std::dynamic_pointer_cast<smtk::model::Manager>(resItem->value());

    smtk::attribute::ComponentItemPtr compItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(result->findComponent("created"));

    // Access the generated model
    smtkmodel2dm =
      std::dynamic_pointer_cast<smtk::model::Entity>(compItem->value())->referenceAs<Model>();
  }

  if (smtkmodel2dm.isValid())
  {
    AssocCollections newAssocCollections = meshmgr->associatedCollections(smtkmodel2dm);
    test(newAssocCollections.size() == 1, "expecting 1 associcated mesh collection");

    //reload the written file and verify the number of meshes are the same as the
    //input mesh
    c2 = newAssocCollections[0];

    //verify the meshes
    test(c2->isValid(), "collection should be valid");

    test(c2->name() == mc->name(), "collection should have same name");
    test(
      c2->numberOfMeshes() == mc->numberOfMeshes(), "collection should have same number of meshes");
    test(c2->types() == mc->types(), "collection should have same types");

    test(c2->meshes(smtk::mesh::Dims2).size() == mc->meshes(smtk::mesh::Dims2).size(),
      "collection should have same faces");
    test(c2->meshes(smtk::mesh::Dims1).size() == mc->meshes(smtk::mesh::Dims1).size(),
      "collection should have same edges");
    test(c2->meshes(smtk::mesh::Dims0).size() == mc->meshes(smtk::mesh::Dims0).size(),
      "collection should have same verts");

    // edge op
    smtk::bridge::discrete::EdgeOperation::Ptr edgeop =
      smtk::bridge::discrete::EdgeOperation::create();
    edgeop->parameters()->associations()->setObjectValue(smtkmodel2dm.component());

    typedef std::vector<Edge> Edges;

    Edges edgelist = resource->findEntitiesByPropertyAs<Edges>("name", "Edge1");
    test(!edgelist.empty() && edgelist.begin()->name() == "Edge1");
    smtk::common::UUID edge1(edgelist.begin()->entity());

    edgelist = resource->findEntitiesByPropertyAs<Edges>("name", "Edge10");
    test(!edgelist.empty() && edgelist.begin()->name() == "Edge10");
    smtk::common::UUID edge10(edgelist.begin()->entity());

    Vertices vertlist = edgelist.begin()->vertices();
    test(vertlist.size() == 1);
    smtk::common::UUID vertex4(vertlist.begin()->entity());

    // split Edge1 on point 15
    int ids[] = { 15 };
    std::set<int> pids(ids, ids + 1);
    smtk::attribute::MeshSelectionItemPtr meshItem =
      edgeop->parameters()->findMeshSelection("selection");
    meshItem->reset();
    meshItem->setValues(edge1, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeop->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Split Edge 1 Failed!\n";
      return 1;
    }
    EntityRefs edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    test(edges.size() == 11, "Expecting 11 edges");
    EntityRefs verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 8, "Expecting 8 vertices");
    test((c2->meshes(smtk::mesh::Dims1)).size() == 11, "Expecting 11 edge mesh");
    test((c2->meshes(smtk::mesh::Dims0)).size() == 8, "Expecting 8 vertex mesh");

    // split Edge10 on point 9
    pids.clear();
    pids.insert(9);
    meshItem->reset();
    meshItem->setValues(edge10, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeop->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Split Edge 10 Failed!\n";
      return 1;
    }
    edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    test(edges.size() == 12, "Expecting 12 edges");
    verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 9, "Expecting 9 vertices");
    test((c2->meshes(smtk::mesh::Dims1)).size() == 12, "Expecting 12 edge mesh");
    test((c2->meshes(smtk::mesh::Dims0)).size() == 9, "Expecting 9 vertex mesh");

    // demote Vertex4
    pids.clear();
    pids.insert(0);
    meshItem->reset();
    meshItem->setValues(vertex4, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeop->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Demote Vertex 4 on Edge 10 Failed!\n";
      return 1;
    }
    edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    test(edges.size() == 11, "Expecting 11 edges");
    verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 8, "Expecting 8 vertices");
    test((c2->meshes(smtk::mesh::Dims1)).size() == 11, "Expecting 11 edge mesh");
    test((c2->meshes(smtk::mesh::Dims0)).size() == 8, "Expecting 8 vertex mesh");
  }
  else
  {
    std::cerr << "failed to create a valid model when loading saved 2dm smtk!\n";
    return 1;
  }

  smtk::common::UUID collectionID = c2->entity();
  vtkNew<vtkActor> act;
  vtkNew<vtkMeshMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper2> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  src->SetMeshManager(meshmgr);
  src->SetMeshCollectionID(collectionID.toString().c_str());
  if (debug)
  {
    win->SetMultiSamples(16);
    src->AllowNormalGenerationOn();
  }
  else
  {
    win->SetMultiSamples(0);
  }
  map->SetInputConnection(src->GetOutputPort());

  act->SetMapper(map.GetPointer());
  act->GetProperty()->SetPointSize(5);
  act->GetProperty()->SetLineWidth(1);
  act->GetProperty()->SetEdgeVisibility(1);
  act->GetProperty()->SetEdgeColor(0, 0, 0.5);
  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());

  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
    ->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);
  /*
    if (debug && argc > 3)
    {
    vtkNew<vtkXMLMultiBlockDataWriter> wri;
    wri->SetInputConnection(src->GetOutputPort());
    wri->SetFileName(argv[3]);
    wri->Write();
    }
  */
  win->Render();
  ren->ResetCamera();

  int status = !vtkRegressionTestImage(win.GetPointer());
  if (debug)
  {
    iac->Start();
  }

  return status;
}
