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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/discrete/Resource.h"
#include "smtk/session/discrete/Session.h"
#include "smtk/session/discrete/operators/EdgeOperation.h"
#include "smtk/session/discrete/operators/ImportOperation.h"

#include "smtk/common/UUID.h"

#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Vertex.h"

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

using namespace smtk::model;
using namespace smtk::attribute;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  if (argc > 1)
  {
    std::ifstream file;
    file.open(argv[1]);
    if (!file.good())
    {
      std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
      return 1;
    }
  }

  // Create an import operator
  smtk::session::discrete::ImportOperation::Ptr importOp =
    smtk::session::discrete::ImportOperation::create();
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

  smtk::model::Resource::Ptr resource =
    std::static_pointer_cast<smtk::model::Resource>(model->resource());
  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  if (model2dm.isValid())
  {
    // Every time a 2dm file is loaded, new UUIDs will be created for entities,
    // but the entity names should be consistent.
    /*
Edge1  117b10e2-c32b-4b65-92fc-72ac5d6aba59
Edge6  146cc049-b104-48a2-8104-43423b1e5ab7
Face1  17a75230-31bb-4b8f-9892-ecedcd3a86c1
Edge9  212b5060-ef18-4e68-81f2-7e7c99b745d2
Model A, vertex 0  229f5c5d-2d74-427e-b547-951cac7d94eb
Edge10  3458e1ea-af84-48b6-856a-a6d6f109d839
Face4  356dd08a-11b3-4c2c-b053-1b1fc936df6d
Model A, vertex 1  3d7888fb-687e-4f8a-8461-f318baa67329
Model A, vertex 2  3e243306-861f-4f0e-ad52-f5296e6f0257
Edge8  4091e1b3-7bc9-42f8-ab65-4eb11e4eb4c5
Model A, vertex 3  4acdcc0e-0c10-4246-a728-db1e622b5ab1
Face3  561c18d3-64ce-4c79-9da0-693685d7a8fa
Face2  5df15024-903e-4a2b-9ae2-fe6e278c4682
Model A, vertex 4  6cb4185f-711d-4588-938b-8b63f1202822
Model A, vertex 5  8d5ccaca-bfc6-4708-b514-f697d412fc9c
Edge3  9ccf0f49-82dd-44a9-a23e-3bfbbd69bc9c
Edge2  a05a29aa-2d27-4b45-802e-971f598cf65a
Edge7  cea41c07-fc45-440b-b5f3-379f6d138aae
Edge4  d7c9e266-db94-49c1-9f83-77f3258ca130
Edge5  e6c2e063-e290-40ef-b9e2-19f8a78e8a9d
Model A, vertex 6  ff3c9b49-bf3f-4fd1-a906-3d40db14736b
*/

    // The first resource is associated with the created model. The second
    // resource is the created mesh collection.
    auto resources = std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resource"));

    // Access the created mesh collection.
    smtk::mesh::CollectionPtr mc =
      std::dynamic_pointer_cast<smtk::mesh::Collection>(resources->value(1));

    test((mc->meshes(smtk::mesh::Dims2)).size() == 4, "Expecting 4 face mesh");
    test((mc->meshes(smtk::mesh::Dims1)).size() == 10, "Expecting 10 edge mesh");
    test((mc->meshes(smtk::mesh::Dims0)).size() == 7, "Expecting 7 vertex mesh");

    // edge op
    smtk::session::discrete::EdgeOperation::Ptr edgeOp =
      smtk::session::discrete::EdgeOperation::create();

    edgeOp->parameters()->associations()->setObjectValue(model2dm.component());

    Edges edgelist = resource->findEntitiesByPropertyAs<Edges>("name", "Edge1");
    test(!edgelist.empty() && edgelist.begin()->name() == "Edge1");
    smtk::common::UUID edge1(edgelist.begin()->entity());

    edgelist = resource->findEntitiesByPropertyAs<Edges>("name", "Edge10");
    test(!edgelist.empty() && edgelist.begin()->name() == "Edge10");
    smtk::common::UUID edge10(edgelist.begin()->entity());

    Vertices vertlist = edgelist.begin()->vertices();
    test(vertlist.size() == 1);
    smtk::common::UUID vertex4(vertlist.begin()->entity());

    // split Edge1 on point 22
    int ids[] = { 22 };
    std::set<int> pids(ids, ids + 1);
    smtk::attribute::MeshSelectionItemPtr meshItem =
      edgeOp->parameters()->findMeshSelection("selection");

    // Explicitly list the mesh collection that is associated with the model.
    edgeOp->parameters()->findResource("associated mesh collections")->appendValue(mc);

    meshItem->reset();
    meshItem->setValues(edge1, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    smtk::session::discrete::EdgeOperation::Result result = edgeOp->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Split Edge 1 Failed!\n";
      return 1;
    }
    /*
    // later on we will demote this new vert
    smtk::common::UUID newVertId = smtk::common::UUID::null();
    ModelEntityItemPtr newEnts = result->findModelEntity("created");
    std::cout << "New entities created: " << newEnts->numberOfValues() << std::endl;
    typedef smtk::model::EntityRefArray::const_iterator const_iterator;
    for(const_iterator it = newEnts->begin(); it != newEnts->end(); ++it)
    if(it->isVertex())
    {
    newVertId = it->entity();
    std::cout << "New vertex id is created!\n";
    break;
    }
    */
    EntityRefs edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    test(edges.size() == 11, "Expecting 11 edges");
    EntityRefs verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 8, "Expecting 8 vertices");
    test((mc->meshes(smtk::mesh::Dims1)).size() == 11, "Expecting 11 edge mesh");
    test((mc->meshes(smtk::mesh::Dims0)).size() == 8, "Expecting 8 vertex mesh");

    // split Edge10 on point 6
    pids.clear();
    pids.insert(6);
    meshItem->reset();
    meshItem->setValues(edge10, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeOp->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Split Edge 10 Failed!\n";
      return 1;
    }
    edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    std::cout << "after split Edge10, number of edges: " << edges.size() << std::endl;
    test(edges.size() == 12, "Expecting 12 edges");
    verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 9, "Expecting 9 vertices");
    test((mc->meshes(smtk::mesh::Dims1)).size() == 12, "Expecting 12 edge mesh");
    test((mc->meshes(smtk::mesh::Dims0)).size() == 9, "Expecting 9 vertex mesh");

    // demote Vertex4
    pids.clear();
    pids.insert(0);
    meshItem->reset();
    meshItem->setValues(vertex4, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeOp->operate();
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
    test((mc->meshes(smtk::mesh::Dims1)).size() == 11, "Expecting 11 edge mesh");
    test((mc->meshes(smtk::mesh::Dims0)).size() == 8, "Expecting 8 vertex mesh");
    /*
    // demote new vertex from first split on Edge1, then split Edge1 again on point 15,
    if(!newVertId.isNull())
    {
    pids.clear();
    pids.insert(0);
    meshItem->reset();
    meshItem->setValues(newVertId, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeOp->operate();
    if (result->findInt("outcome")->value() != static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
    std::cerr << "Demote new Vertex from first Split on Edge1 Failed!\n";
    return 1;
    }

    pids.clear();
    pids.insert(15);
    meshItem->reset();
    meshItem->setValues(edge1, pids);
    meshItem->setModifyMode(smtk::attribute::ACCEPT);
    result = edgeOp->operate();
    if (result->findInt("outcome")->value() != static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
    std::cerr << "Second Split Edge 1 Failed!\n";
    return 1;
    }
    edges = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
    test(edges.size() == 11, "Expecting 11 edges");
    verts = resource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
    test(verts.size() == 8, "Expecting 8 vertices");
    test((mc->meshes(smtk::mesh::Dims1)).size() == 11, "Expecting 11 edge mesh");
    test((mc->meshes(smtk::mesh::Dims0)).size() == 8, "Expecting 8 vertex mesh");

    }
    */

    smtk::common::UUID collectionID = mc->entity();
    vtkNew<vtkActor> act;
    vtkNew<vtkMeshMultiBlockSource> src;
    vtkNew<vtkCompositePolyDataMapper2> map;
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> win;
    src->SetMeshCollection(mc);
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
    if (status != 0)
    {
      return status;
    }
  }

  return 0;
}
