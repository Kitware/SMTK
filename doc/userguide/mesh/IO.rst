IO
============

:smtk:io:`WriteMesh <smtk::io::WriteMesh>`
  Writes out a given :smtk:`Collection <smtk::mesh::Collection>`, or only
  the elements that match a given Domain, Neumann, or Dirichlet property.

  Supported formats:
      + MOAB (h5m, mhdf)
      + Exodus II (exo exoII exo2 g gen)


:smtk:io:`ImportMesh <smtk::io::ImportMesh>`
  Load a given file in as a new :smtk:`Collection <smtk::mesh::Collection>` or
  part of an existing :smtk:`Collection <smtk::mesh::Collection>`. Also
  supports loading just elements that a given Domain, Neumann, or Dirichlet
  property.

  Supported formats:
      + MOAB (h5m, mhdf)
      + Exodus II (exo exoII exo2 g gen)

Serialization
============

:smtk:io:`ExportJSON <smtk::io::ExportJSON>`
  Export all the :smtk:`Collection <smtk::mesh::Collection>` that have
  associations with the any model that is part of the passed in
  :smtk:`Manager <smtk::model::Manager>`. The exporter will save each
  Collection using :smtk:io:`WriteMesh <smtk::io::WriteMesh>` with the file
  type MOAB and extension h5m.

  The format of the created json is:

.. highlight:: json
.. code-block:: json
    b0e336a9-5fbf-4dae-b419-3664c17ef402": {
      "formatVersion":  1,
      "name": "",
      "fileType": "moab",
      "location": "/Users/robert/Work/smtk/data/mesh/tmp/output.0.h5m"
    }


:smtk:io:`ImportJSON <smtk::io::ImportJSON>`
  Imports all the :smtk:`Collection <smtk::mesh::Collection>` that are listed
  in the provided JSON string. Each Collection will be marked as being associated
  with the provided model :smtk:`Manager <smtk::model::Manager>`.
