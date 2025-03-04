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
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/operators/SetMeshName.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::ResourcePtr load_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  return mr;
}

void verify_num_meshes(const smtk::mesh::ResourcePtr& mr)
{
  std::size_t numMeshes = mr->numberOfMeshes();

  test(numMeshes != 0, "dataset once loaded should have more than zero meshes");
  test(numMeshes == 53, "dataset once loaded should have 53 meshes");

  smtk::mesh::MeshSet all_meshes = mr->meshes();
  test(numMeshes == all_meshes.size());
  test(!all_meshes.is_empty());
  test(all_meshes.isValid());
}

void verify_constructors(const smtk::mesh::ResourcePtr& mr)
{
  std::vector<std::string> mesh_names = mr->meshNames();

  smtk::mesh::MeshSet ms = mr->meshes(mesh_names[0]);

  const smtk::mesh::MeshSet& ms2(ms);
  smtk::mesh::MeshSet ms3 = mr->meshes("bad_name");
  test(ms3.is_empty());
  test(!ms3.isValid());
  test(ms3.size() == 0);

  test(ms.size() == ms2.size());
  test(ms.size() != ms3.size());

  ms3 = ms; //test assignment operator
  test(ms.size() == ms3.size());

  test(!ms.is_empty());
  test(!ms2.is_empty());
  test(!ms3.is_empty());
}

void verify_comparisons(const smtk::mesh::ResourcePtr& mr)
{
  std::vector<std::string> mesh_names = mr->meshNames();

  smtk::mesh::MeshSet one = mr->meshes(mesh_names[0]);
  smtk::mesh::MeshSet two = mr->meshes(mesh_names[1]);

  test(one == one);
  test(!(one != one));
  test(two != one);
  test(!(two == one));

  const smtk::mesh::MeshSet& one_a(one);
  test(one_a == one);

  smtk::mesh::MeshSet two_b = one_a;
  two_b = two; //test assignment operator
  test(two_b == two);

  test(one_a != two_b);
}

void verify_typeset(const smtk::mesh::ResourcePtr& mr)
{
  //verify that empty meshset set has empty type set
  {
    smtk::mesh::CellTypes no_cell_types;
    smtk::mesh::MeshSet emptyMeshSet = mr->meshes("bad name string");
    smtk::mesh::TypeSet noTypes = emptyMeshSet.types();

    test(noTypes.cellTypes() == no_cell_types);
    test(!noTypes.hasMeshes());
    test(!noTypes.hasCells());
  }

  //verify that if we get all cells from the resource the type set is correct
  {
    smtk::mesh::TypeSet all_types = mr->types();
    smtk::mesh::MeshSet allMeshes = mr->meshes();
    smtk::mesh::TypeSet allMeshesTypes = allMeshes.types();

    test(allMeshesTypes.hasMeshes());
    test(allMeshesTypes.hasCells());
    test(allMeshesTypes.cellTypes() == all_types.cellTypes());
    test(allMeshesTypes == all_types);
  }
}

void verify_mesh_by_name(const smtk::mesh::ResourcePtr& mr)
{
  std::vector<std::string> mesh_names = mr->meshNames();
  std::size_t collec_numMeshes = mr->numberOfMeshes();

  //while we can't state that every mesh will have a name, we do know
  //that at least 1 will have a name
  test(collec_numMeshes >= mesh_names.size());
  test(!mesh_names.empty());

  //now iterate over all the mesh_name and verify we can
  //get a valid meshset out of it
  typedef std::vector<std::string>::const_iterator it;
  std::size_t numMeshesWithNames = 0;
  for (it i = mesh_names.begin(); i != mesh_names.end(); ++i)
  {
    std::cout << "Looking for mesh: " << *i << std::endl;
    smtk::mesh::MeshSet ms = mr->meshes(*i);
    test(ms.size() != 0);
    numMeshesWithNames += ms.size();
  }

  test(
    collec_numMeshes >= numMeshesWithNames,
    "Number of meshes with names should be less than total number of meshes");
}

void verify_meshset_by_dim(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::MeshSet all_meshes = mr->meshes();

  //verify that the meshset given back from the resource is the same size
  //as the number of meshes in the entire dataset
  std::size_t collec_numMeshes = mr->numberOfMeshes();
  std::size_t size = all_meshes.size();
  test(size == collec_numMeshes);

  //query for all meshsets of dimension 0, 1, 2, and 3
  smtk::mesh::MeshSet all_dims;
  std::size_t numMeshesFoundByDimCalls = 0;
  for (int i = 0; i < 4; ++i)
  {
    smtk::mesh::DimensionType d(static_cast<smtk::mesh::DimensionType>(i));
    smtk::mesh::MeshSet meshesWithDim = mr->meshes(d);
    numMeshesFoundByDimCalls += meshesWithDim.size();
    all_dims.append(meshesWithDim);
  }

  //since a mesh can have multiple dimensions numMeshesFoundByDimCalls
  //is larger than all_meshes.size
  test(all_meshes.size() <= numMeshesFoundByDimCalls);

  //all_dims and all_meshes should be the same
  test(all_dims == all_meshes);
}

void verify_meshset_of_only_a_dim(const smtk::mesh::ResourcePtr& mr)
{
  //verify that we can extract a meshset whose cells are all of the same
  //dimension. without any complications

  smtk::mesh::MeshSet meshesWithDim3 = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet otherMeshes = mr->meshes(smtk::mesh::Dims2);
  otherMeshes.append(mr->meshes(smtk::mesh::Dims1));

  //meshesWithOnlyDim3 will contain meshsets that are pure 3d cells
  smtk::mesh::MeshSet meshesWithOnlyDim3 = smtk::mesh::set_difference(meshesWithDim3, otherMeshes);

  //verify that we have zero cells of 1 or 2 dim
  test(meshesWithOnlyDim3.cells(smtk::mesh::Dims0).is_empty());
  test(meshesWithOnlyDim3.cells(smtk::mesh::Dims1).is_empty());
  test(meshesWithOnlyDim3.cells(smtk::mesh::Dims2).is_empty());
  test(!meshesWithOnlyDim3.cells(smtk::mesh::Dims3).is_empty());

  //verify that the associated types comes back to properly
  smtk::mesh::TypeSet types = meshesWithOnlyDim3.types();
  test(types.hasMeshes());
  test(types.hasCells());

  test(!types.hasDimension(smtk::mesh::Dims0));
  test(!types.hasDimension(smtk::mesh::Dims1));
  test(!types.hasDimension(smtk::mesh::Dims2));
  test(types.hasDimension(smtk::mesh::Dims3));

  test(!types.hasCell(smtk::mesh::Vertex));
  test(!types.hasCell(smtk::mesh::Line));
  test(!types.hasCell(smtk::mesh::Triangle));
  test(!types.hasCell(smtk::mesh::Quad));
  test(!types.hasCell(smtk::mesh::Polygon));
  test(!types.hasCell(smtk::mesh::Tetrahedron));
  test(!types.hasCell(smtk::mesh::Pyramid));
  test(!types.hasCell(smtk::mesh::Wedge));
  test(types.hasCell(smtk::mesh::Hexahedron));
}

void verify_meshset_subset_dim(const smtk::mesh::ResourcePtr& mr)
{
  //verify that we can subset by dimension
  smtk::mesh::MeshSet allMeshes = mr->meshes();

  test(allMeshes.subset(smtk::mesh::Dims3) == mr->meshes(smtk::mesh::Dims3));
  test(allMeshes.subset(smtk::mesh::Dims2) == mr->meshes(smtk::mesh::Dims2));
  test(allMeshes.subset(smtk::mesh::Dims1) == mr->meshes(smtk::mesh::Dims1));
  test(allMeshes.subset(smtk::mesh::Dims0) == mr->meshes(smtk::mesh::Dims0));
}

template<typename T>
void verify_subset_tag(
  const smtk::mesh::ResourcePtr& mr,
  smtk::mesh::MeshSet allMeshes,
  std::vector<T> tag_values)
{
  for (std::size_t i = 0; i < tag_values.size(); ++i)
  {
    test(allMeshes.subset(tag_values[i]) == mr->meshes(tag_values[i]));
  }
}

void verify_meshset_subset_tag(const smtk::mesh::ResourcePtr& mr)
{
  //verify that we can extract a sub meshset based on the tags
  //inside a meshet.
  smtk::mesh::MeshSet allMeshes = mr->meshes();

  //verify the lengths before we check the size of each
  //tag
  test(allMeshes.domains().size() == mr->domains().size());
  test(allMeshes.dirichlets().size() == mr->dirichlets().size());
  test(allMeshes.neumanns().size() == mr->neumanns().size());

  //domains
  verify_subset_tag(mr, allMeshes, mr->domains());

  //dirichlets
  verify_subset_tag(mr, allMeshes, mr->dirichlets());

  //neumann
  verify_subset_tag(mr, allMeshes, mr->neumanns());
}

void verify_meshset_add_tags(const smtk::mesh::ResourcePtr& mr)
{
  //verify that we can add tags to a meshset
  smtk::mesh::MeshSet allMeshes = mr->meshes();

  smtk::mesh::MeshSet verts = allMeshes.subset(smtk::mesh::Dims0);
  const bool applied = verts.setDirichlet(smtk::mesh::Dirichlet(42));
  const std::size_t numDirValues = mr->dirichlets().size();

  test(applied, "didn't apply the dirichlet property");
  test(numDirValues > 0, "should have more than zero dirichlet sets");
}

void verify_meshset_intersect(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::MeshSet all_meshes = mr->meshes();

  { //intersection of self should produce self
    smtk::mesh::MeshSet result = smtk::mesh::set_intersect(all_meshes, all_meshes);
    test(result == all_meshes, "Intersection of self should produce self");
  }

  { //intersection with nothing should produce nothing
    smtk::mesh::MeshSet no_meshes = mr->meshes("bad name string");
    smtk::mesh::MeshSet result = smtk::mesh::set_intersect(all_meshes, no_meshes);
    test(result == no_meshes, "Intersection with nothing should produce nothing");
  }

  //find meshes that have volume elements
  smtk::mesh::MeshSet volumeMeshes = mr->meshes(smtk::mesh::Dims3);

  //verify that the size of the intersection + size of difference
  //equal size
  smtk::mesh::MeshSet intersect_result = smtk::mesh::set_intersect(all_meshes, volumeMeshes);

  smtk::mesh::MeshSet difference_result = smtk::mesh::set_difference(all_meshes, volumeMeshes);

  const std::size_t summed_size = intersect_result.size() + difference_result.size();
  test(
    summed_size == all_meshes.size(), "Size of intersect + difference needs to be the same as total\
       number of unique items");
}

void verify_meshset_union(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::MeshSet all_meshes = mr->meshes();

  { //union with self produces self
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes, all_meshes);
    test(result == all_meshes, "Union of self should produce self");
  }

  { //union with nothing should produce self
    smtk::mesh::MeshSet no_meshes = mr->meshes("bad name string");
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes, no_meshes);
    test(result == all_meshes, "Union with nothing should produce self");
  }

  //construct empty meshset(s)
  smtk::mesh::MeshSet all_dims = mr->meshes("bad name string");
  smtk::mesh::MeshSet append_output;
  //verify that append and union produce the same result
  for (int i = 0; i < 4; ++i)
  {
    smtk::mesh::DimensionType d(static_cast<smtk::mesh::DimensionType>(i));
    all_dims = smtk::mesh::set_union(all_dims, mr->meshes(d));
    append_output.append(mr->meshes(d));
  }

  test(all_dims == append_output, "Result of union should be the same as append");

  {
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes, all_dims);
    smtk::mesh::MeshSet result2 = all_meshes;
    result2.append(all_dims);
    test(result == result2);
  }
}

void verify_meshset_subtract(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::MeshSet all_meshes = mr->meshes();

  { //subtract of self should produce empty
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(all_meshes, all_meshes);
    test(result.size() == 0, "Subtraction of self should produce nothing");
    test(result != all_meshes, "Subtraction of self should produce nothing");
  }

  { //subtract with nothing should produce self
    smtk::mesh::MeshSet no_meshes = mr->meshes("bad name string");
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(all_meshes, no_meshes);
    test(result == all_meshes, "Subtraction with nothing should produce self");
  }

  { //subtract with something from nothing should produce nothing
    smtk::mesh::MeshSet no_meshes = mr->meshes("bad name string");
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(no_meshes, all_meshes);
    test(result == no_meshes, "Subtraction of something from nothing should nothing");
  }

  //find meshes that have volume elements
  smtk::mesh::MeshSet volumeMeshes = mr->meshes(smtk::mesh::Dims3);

  std::size_t size_difference = all_meshes.size() - volumeMeshes.size();
  smtk::mesh::MeshSet non_dim_meshes = smtk::mesh::set_difference(all_meshes, volumeMeshes);
  test(non_dim_meshes.size() == size_difference, "subtract of two meshes produced wrong size");
}

class CountMeshesAndCells : public smtk::mesh::MeshForEach
{
  //keep the range of cells we have seen so we can verify that we
  //seen all the cells that we expect to be given
  smtk::mesh::CellSet cellsSeen;
  //keep a physical count of number of meshes so that we can verify we
  //don't iterate over a mesh more than once
  int numMeshesIteratedOver{ 0 };

public:
  CountMeshesAndCells(smtk::mesh::ResourcePtr resource)
    : cellsSeen(resource->meshes("InvalidName").cells())

  {
  }

  void forMesh(smtk::mesh::MeshSet& mesh) override
  {
    this->numMeshesIteratedOver++;
    this->cellsSeen.append(mesh.cells());
  }

  [[nodiscard]] int numberOfMeshesVisited() const { return numMeshesIteratedOver; }

  [[nodiscard]] smtk::mesh::CellSet cells() const { return cellsSeen; }
};

void verify_meshset_for_each(const smtk::mesh::ResourcePtr& mr)
{
  CountMeshesAndCells functor(mr);
  smtk::mesh::MeshSet volMeshes = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::for_each(volMeshes, functor);

  test(static_cast<std::size_t>(functor.numberOfMeshesVisited()) == volMeshes.size());
  test(functor.cells() == volMeshes.cells());
}

void verify_meshset_visit(const smtk::mesh::ResourcePtr& mr)
{
  std::size_t numMeshesIteratedOver = 0;
  smtk::resource::Component::Visitor countMeshesAndCells =
    [&](const smtk::resource::ComponentPtr& component) {
      auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
      numMeshesIteratedOver++;
    };

  mr->visit(countMeshesAndCells);

  test(numMeshesIteratedOver == mr->meshes().size());
}

void verify_meshset_set_names(const smtk::mesh::ResourcePtr& mr)
{
  std::size_t numMeshesIteratedOver = 0;
  smtk::resource::Component::Visitor setMeshNames =
    [&](const smtk::resource::ComponentPtr& component) {
      auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
      std::stringstream s;
      s << "meshset " << numMeshesIteratedOver;
      meshComponent->mesh().setName(s.str());
      numMeshesIteratedOver++;
    };

  mr->visit(setMeshNames);

  numMeshesIteratedOver = 0;
  smtk::resource::Component::Visitor checkMeshNames =
    [&](const smtk::resource::ComponentPtr& component) {
      auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
      std::stringstream s;
      s << "meshset " << numMeshesIteratedOver;
      test(meshComponent->name() == s.str());
      numMeshesIteratedOver++;
    };

  mr->visit(checkMeshNames);
}

void verify_meshset_set_name_op(const smtk::mesh::ResourcePtr& mr)
{
  std::size_t numMeshesIteratedOver = 0;
  smtk::resource::Component::Visitor setMeshNames =
    [&](const smtk::resource::ComponentPtr& component) {
      auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
      std::stringstream s;
      s << "my_meshset " << numMeshesIteratedOver;
      smtk::mesh::SetMeshName::Ptr setMeshNameOp = smtk::mesh::SetMeshName::create();
      setMeshNameOp->parameters()->associate(meshComponent);
      setMeshNameOp->parameters()->findString("name")->setValue(s.str());
      auto result = setMeshNameOp->operate();
      test(
        result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED));
      numMeshesIteratedOver++;
    };

  mr->visit(setMeshNames);

  numMeshesIteratedOver = 0;
  smtk::resource::Component::Visitor checkMeshNames =
    [&](const smtk::resource::ComponentPtr& component) {
      auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
      std::stringstream s;
      s << "my_meshset " << numMeshesIteratedOver;
      test(meshComponent->name() == s.str());
      numMeshesIteratedOver++;
    };

  mr->visit(checkMeshNames);
}
} // namespace

int UnitTestMeshSet(int /*unused*/, char** const /*unused*/)
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  verify_num_meshes(mr);
  verify_constructors(mr);
  verify_comparisons(mr);
  verify_typeset(mr);
  verify_mesh_by_name(mr);
  verify_meshset_by_dim(mr);
  verify_meshset_of_only_a_dim(mr);
  verify_meshset_subset_dim(mr);
  verify_meshset_subset_tag(mr);
  verify_meshset_add_tags(mr);
  verify_meshset_intersect(mr);
  verify_meshset_union(mr);
  verify_meshset_subtract(mr);

  verify_meshset_for_each(mr);
  verify_meshset_visit(mr);
  verify_meshset_set_names(mr);
  verify_meshset_set_name_op(mr);

  return 0;
}
