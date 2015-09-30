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
    "70ec982c-9562-44bd-a7e7-bd12b84a3271": {
     "formatVersion":  1,
     "name": "",
     "fileType": "moab",
     "location": "/tmp/output.0.h5m",
     "nc": 40,
     "np": 28,
     "cell_types": "000000100",
     "domains":  [],
     "boundary_conditions":  {
       "0": {
         "value":  2,
         "type": "dirichlet"
       },
       "1":  {
         "value":  2,
         "type": "neumann"
       },
     },
     "modelEntityIds": ["0442f22c-26dc-4e6b-bdd8-1e77b75e5d36", "7d42284b-c7e0-4777-8836-3b77d6aed0e3", "8cdcf988-36bd-43ed-bb60-c76443907f16", "c7a90a24-f058-4d79-8b75-bb58470547bf"],
     "meshes": {
       "0":  {
         "nc": 10,
         "np": 7,
         "cell_types": "000000100",
         "domains":  [],
         "boundary_conditions":  {
           "0":  {
             "value":  2,
             "type": "neumann"
           }
         },
         "modelEntityIds": ["0442f22c-26dc-4e6b-bdd8-1e77b75e5d36"]
       },
       "1":  {
         "nc": 10,
         "np": 7,
         "cell_types": "000000100",
         "domains":  [],
         "boundary_conditions":  { },
         "modelEntityIds": ["7d42284b-c7e0-4777-8836-3b77d6aed0e3"]
       },
       "2":  {
         "nc": 10,
         "np": 7,
         "cell_types": "000000100",
         "domains":  [],
         "boundary_conditions":  {
           "0":  {
             "value":  2,
             "type": "dirichlet"
           }
           "1":  {
             "value":  2,
             "type": "neumann"
           }
         },
         "modelEntityIds": ["8cdcf988-36bd-43ed-bb60-c76443907f16"]
       },
       "3":  {
         "nc": 10,
         "np": 7,
         "cell_types": "000000100",
         "domains":  [],
         "boundary_conditions":  { },
         "modelEntityIds": ["c7a90a24-f058-4d79-8b75-bb58470547bf"]
       }
     }
   }


:smtk:io:`ImportJSON <smtk::io::ImportJSON>`
  Imports all the :smtk:`Collection <smtk::mesh::Collection>` that are listed
  in the provided JSON string. Each Collection will be marked as being associated
  with the provided model :smtk:`Manager <smtk::model::Manager>`.
