
##General##

0. We need the concept of an entity, which is the superset of
   cell types, points, mesh sets, dimensionality, etc. The entity concept
   is how people query. While stuff like CellTraits are used by internal
   containers ( or ripped out if required )

1. Organize the concept of cell types and querying by cell types. Something
  like the following should be possible:
```
  collection.cells( smtk::mesh::Hexahedron );
  meshset.cells( smtk::mesh::Hexahedron );
```
which means that we need the ability of cell types be derived from a common
parent so that this is possible ( or use templates? ). I think using enums /
flags is the best way for this, and than a Traits and CellTypes to convert
from enum to concrete type

2. We need to also expand the concepts to dimensionality. We should
be able to do the following:
```
  collection.cells( smtk::mesh::Dims2 );
  meshset.cells( smtk::mesh::Dims3 );
```

3. Ability to walk the mesh in some manner.


4. Ability to get and set the names for meshCollections and specific meshes


5. Ability to query Collection based on a UUID

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  smtk::mesh::CollectionPtr manager.collection( uuid_of_collection );

```

6. Ability to extract the shells for each region.
   Most likely done by using the code written for the moab reader as options
   on the collection.

7. Ability to do a union, intersect, and difference of meshsets.
```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  smtk::mesh::CollectionPtr collection = manager.collection( uuid_of_collection );
  ...
  smtk::mesh::MeshSet m2 = collection->meshes( smtk::mesh::Dims2 );
  smtk::mesh::MeshSet m3 = collection->meshes( smtk::mesh::Dims3 );

  smtk::mesh::MeshSet overlapping = smtk::mesh::set_intersect(m2,m3);
  smtk::mesh::MeshSet not_overlapping = smtk::mesh::set_difference(m2,m3);
```

8. Ability to do a union, intersect, and difference of cellsets.
```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  smtk::mesh::CollectionPtr collection = manager.collection( uuid_of_collection );
  ...
  smtk::mesh::CellSet c2 = collection->cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet c3 = collection->cells( smtk::mesh::Dims3 );

  smtk::mesh::CellSet overlapping = smtk::mesh::set_intersect(c2,c3);
  smtk::mesh::MeshSet not_overlapping = smtk::mesh::set_difference(c2,c3);
```

9. Ability to do intersection and difference of two distinct mesh or cell sets
 that share nothing in common but point coordinates ( aka topology ids ).

 The query will need to take in three input parameters. 1 and 2 are the sets
 to query against and the third is how we determine if a cell from set 2 is
 contained in set 1. Currently the two options are Partial Contained and
 Fully Contained, with the former meaning at least 1 point per cell needs to be
 in set 1 to be contained in the result, while the latter requires all points
 per cell.

 The reason for this functionality is the ability to find usages between
 two distinct sets. Say for example trying to find the lines from set b whose
 points are used by any triangle in set a. So in code it would look like:

 ```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  smtk::mesh::CollectionPtr collection = manager.collection( uuid_of_collection );
  ...
  smtk::mesh::CellSet c1 = collection->cells( smtk::mesh::Lines );
  smtk::mesh::CellSet c2 = collection->cells( smtk::mesh::Triangles );


  //overlapping now contains all cells from c1 that share any point with
  //a cell from C2
  smtk::mesh::CellSet overlapping = smtk::mesh::point_intersect(
                            c2,c1, PartiallyContained );


 ```



##IO##

1. We need the ability to read a moab dataset and construct a smtk::mesh
from it. The code should roughly look like:

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, manager);
  smtk::mesh::CollectionPtr c = manager->collection( entity );

```

2. Like wise we will have to be save a dataset from the manager back
into moab.

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  //save the last mesh in the collection
  const std::size_t size = manager.numberOfCollections();
  smtk::mesh::Manager::const_iterator collection = manager.collectionBegin();

  //now
  smtk::io::SaveMesh::intoFile(*collection, file_path);

```


##Meshing to Mesh##

Current goals of the system are:

1. Being able to serialize and deserialize an single mesh to an std::string
  so that we can transmit it to a remus worker.

2. Send both a model and related meshes to a remus worker. This means
  a more complex serialization strategy than number 1. We need to talk about
  this in more detail.

  But basically what we want is to send an entire model, all meshes that
  are related to the model, and those relationships ( classification ).

  The best way to do this might be the ability to save a model + mesh to a
  file and push that through remus

3. Ability for remus workers to link to smtk and decompose a mesh / model
   that has been sent to it

##Model to Mesh##

1. Able to associate model cursors / uuids to mesh elements, which can be:
  a. Cell
  b. Mesh
  c. Collection
  d. Set of Cell's, Point's, Mesh's


2. Query given a model cursor find what are the associated types:

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = manager.associatedCollection(modelCursor);
  smtk::mesh::TypeSet ts = collection->findAssociatedTypes( modelCursor );
```

3. Query given a model cursor find all elements of a specific type:

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = manager.associatedCollection(modelCursor);

  smtk::mesh::CellSet cs = collection->findAssociatedCells( modelCursor );
  smtk::mesh::PointSet ps = collection->findAssociatedPoints( modelCursor );
  smtk::mesh::MeshSet ms = collection->findAssociatedMeshes( modelCursor );

  //find only hexahedron cells
  smtk::mesh::CellSet cs = collection->findAssociatedCells( modelCursor,
                                                           smtk::mesh::Hexahedron );

  //find only meshes that contain 2d cells
  smtk::mesh::MeshSet ms = collection->findAssociatedMeshes( modelCursor,
                                                            smtk::mesh::Dim2 );
```

##General Manager API examples##

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::common::uuid meshSetUUID = smtk::io::load_mesh(file_path, manager);

  manager.numberOfCollections();

  smtk::mesh::CollectionPtr collection = manager.collection( meshSetUUID );

  collection->numberOfMeshes();

  smtk::mesh::MeshSet m2 = collection->meshes( smtk::mesh::Dims2 );
  smtk::mesh::MeshSet m3 = collection->meshes( smtk::mesh::Dims3 );
```
