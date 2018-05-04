Delaunay Mesh Worker
============
The Delaunay Mesh Worker is an example [Remus][Remus] mesh worker for [Delaunay][Delaunay], a simple project for discretizing 2-dimensional surfaces. It consists of the following core components:

* [DelaunayMeshingDefs.sbt](./DelaunayMeshingDefs.sbt): an SMTK attribute template file describing the input requirements of the mesh worker.

* [DelaunayMesh.rw](./DelaunayMesh.rw): a [Remus Worker File][WorkerFile] describing the name of the mesh worker executable, its input and output types, the SMTK attribute file describing its inputs and the file format for this attribute file.

* [DelaunayMeshWorker.h](./DelaunayMeshWorker.h) & [DelaunayMeshWorker.cxx](./DelaunayMeshWorker.cxx): a subclass of a Remus `worker::Worker` adapted to interface with the Delaunay project. The method `DelaunayMeshWorker::meshJob()` parses an input model received from a query process, constructs a mesh collection with mesh elements for each meshed face, and sends the result back to the query process.

* [DelaunayMeshWorkerMain.cxx](./DelaunayMeshWorkerMain.cxx): The main function for the mesh worker, which constructs an instance of `DelaunayMeshWorker` and starts an event loop.




[Remus]: https://gitlab.kitware.com/cmb/remus
[Delaunay]: https://github.com/tjcorona/Delaunay
[WorkerFile]: https://gitlab.kitware.com/cmb/remus/blob/master/remus/worker/Readme.md#constructing-a-remus-worker-file
