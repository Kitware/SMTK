
##General ToDo##

1. Ability to set the names for meshCollections and specific meshes

2. Ability to query for the connectivity of a meshset from a given dimension
   to another dimension. This would return all the connectivity of
   all elements of the given dimension to the requested dimension.

```
  smtk::mesh::MeshSet ms;
  smtk::mesh::MeshSet ms2d = ms.connectivity( smtk::mesh::Dims3,
                                              smtk::mesh::Dims2);

```

  We will also need to support the ability to create the connectivity
  if it doesn't exist.

```
  smtk::mesh::MeshSet ms;
  smtk::mesh::MeshSet ms2d = ms.createConnectivity( smtk::mesh::Dims3,
                                                    smtk::mesh::Dims2);

```

3. Create easy to use Tags of arbitrary types (float/int/etc) with varying
   values for all points/cells in a meshset.


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

# Finished Tasks #

1. Organize the concept of cell types and querying by cell types.
```
  collection.cells( smtk::mesh::Hexahedron );
  meshset.cells( smtk::mesh::Hexahedron );
```

2. We need to be able query based on dimensionality.
```
  collection.cells( smtk::mesh::Dims2 );
  meshset.cells( smtk::mesh::Dims3 );
```

3. Ability to query Collection based on a UUID

```
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  ...
  smtk::mesh::CollectionPtr manager.collection( uuid_of_collection );

```

4. Ability to do a union, intersect, and difference of meshsets.
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

5. Ability to do a union, intersect, and difference of cellsets.
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

6. Ability to do intersection and difference of two distinct mesh or cell sets
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

7. Add and Remove meshsets from a collection.
  ```
    smtk::mesh::CollectionPtr c = manager.collection( uuid_of_collection );
    smtk::mesh::CellSet cells = c->cells( smtk::mesh::Dims2 );
    cells.append(c->cells( smtk::mesh::Dims3 ) );

    //this adds a mesh to the collection
    smtk::mesh::MeshSet mesh = c->createMesh(cells);

    //this removes a mesh from the collection, and any
    //cells that no other mesh uses
    const bool removed = c->removeMeshes(mesh);

  ```

8. Mark meshset(s) as being Boundary, Dirichlet or Neumann. Plus they can
   have a value for that tag!
  ```
  smtk::mesh::CollectionPtr c = manager.collection( uuid_of_collection );
  smtk::mesh::Meshset ms = c->meshes( smtk::mesh::Dims2 );
  ms.append(c->meshes( smtk::mesh::Dims3 ) );

  c->setDomainOnMeshes( ms, smtk::mesh::Domain(4) );
  ```

9. Get all the meshes with a given domain value

  ```
  typedef std::vector< smtk::mesh::Domain > DomainVecType;
  DomainVecType domains = c->domains();

  smtk::mesh::MeshSet domainMeshes = c->meshes( domains[0] );
  ```

9. Extract the shell of any given collection of meshesets.

  ```
  smtk::mesh::CollectionPtr c = manager.collection( uuid_of_collection );
  smtk::mesh::MeshSet shell = c->extractShell( c->meshes( smtk::mesh::Dims3 ) );

  ```
