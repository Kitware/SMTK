
##General##

0. We need the concept of an entity, which is the superset of
   cell types, points, mesh sets, dimensionality, etc. The entity concept
   is how people query. While stuff like CellTraits are used by internal
   containers ( or ripped out if required )

1. Organize the concept of cell types and querying by cell types. Something
  like the following should be possible:
```
  meshSet.allCellsOfType( smtk::mesh::Hexahedron );
  cellSet.allCellsOfType( smtk::mesh::Pyramid );
```
which means that we need the ability of cell types be derived from a common
parent so that this is possible ( or use templates? ). I think using enums /
flags is the best way for this, and than a Traits and CellTypes to convert
from enum to concrete type

2. We need to also expand the concepts to dimensionality. We should
be able to do the following:
```
  meshSet.AllCellsOfType( smtk::mesh::Dims2 );
  cellSet.AllCellsOfType( smtk::mesh::Dims3 );
```

3. Ability to walk the mesh in some manner. This decomposes int

##IO##

1. We need the ability to read a moab dataset and construct a smtk::mesh
from it. The code should roughly look like:

```
  smtk::mesh::Manager manager;
  smtk::common::uuid meshSetUUID = smtk::io::load_mesh(file_path, manager);

```

2. Like wise we will have to be save a dataset from the manager back
into moab.

```
  smtk::mesh::Manager manager;
  ...
  //save the last mesh in the collection
  const std::size_t size = manager.numberOfMeshCollections();
  smtk::mesh::MeshCollection collection = manager.meshCollection(size-1);

  //now
  smtk::io::save_mesh(collection);

  //or
  smtk::io::save_mesh( manager, collection.entity() );
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
  b. Point
  c. Mesh
  d. Collection
  e. Set of Cell's, Point's, Mesh's


2. Query given a model cursor find what are the associated types:

```
  smtk::mesh::Manager manager;

  smtk::mesh::TypeSet ts = manager.findAssociatedTypes( modelCursor );
```

3. Query given a model cursor find all elements of a specific type:

```
  smtk::mesh::Manager manager;

  smtk::mesh::CellSet cs = manager.findAssociatedCells( modelCursor );
  smtk::mesh::PointSet ps = manager.findAssociatedPoints( modelCursor );
  smtk::mesh::MeshSet ms = manager.findAssociatedMeshes( modelCursor );

  //find only hexahedron cells
  smtk::mesh::CellSet cs = manager.findAssociatedCells( modelCursor,
                                                          smtk::mesh::Hexahedron );

  //find only meshes that contain 2d cells
  smtk::mesh::MeshSet ms = manager.findAssociatedMeshes( modelCursor,
                                                           smtk::mesh::Dim2 );
```

##General Manager API examples##

```
  smtk::mesh::Manager manager;
  smtk::common::uuid meshSetUUID = smtk::io::load_mesh(file_path, manager);

  manager.numberOfMeshCollections();
  manager.numberOfMeshesInCollection(meshSetUUID);

  smtk::mesh::MeshCollection collection = manager.meshCollection(meshSetUUID);

  collection.numberOfMeshes();
  collection.highestDimension();

  smtk::mesh::MeshSet = collection.allMeshes( smtk::mesh::Dims2 );
  smtk::mesh::MeshSet = collection.allMeshes( smtk::mesh::Dims3 );

  smtk::mesh::CellSet cells = collection.all2DCells();
```
