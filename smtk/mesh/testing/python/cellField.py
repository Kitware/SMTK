#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

import os
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.common
    import smtk.io
    import smtk.mesh
else:
    import uuid
import smtk.testing
import sys


def test_create_cell_field():
    m = smtk.mesh.Manager.create()

    # Load the mesh file
    mesh_path = os.path.join(smtk.testing.DATA_DIR,
                             'mesh', '2d/twoMeshes.h5m')
    c = smtk.io.importMesh(mesh_path, m)
    if not c.isValid():
        raise RuntimeError("Failed to read valid mesh")

    mesh = c.meshes()
    field = [i for i in xrange(c.meshes().cells().size())]
    mesh.createCellField('cell field', 1, field)

    write_path = ''
    if smtk.wrappingProtocol() == 'pybind11':
        write_path = os.path.join(smtk.testing.TEMP_DIR,
                                  str(smtk.common.UUID.random()) + ".h5m")
    else:
        write_path = os.path.join(smtk.testing.TEMP_DIR,
                                  str(uuid.uuid4()) + ".h5m")

    smtk.io.exportMesh(write_path, c)
    return write_path


def test_read_cell_field(mesh_path):
    m = smtk.mesh.Manager.create()

    # Load the mesh file
    c = smtk.io.importMesh(mesh_path, m)
    if not c.isValid():
        raise RuntimeError("Failed to read back valid mesh")

    mesh = c.meshes()
    if smtk.wrappingProtocol() == 'pybind11':
        cellfields = mesh.cellFields()
    else:
        cellfields = mesh.cellFieldsForShiboken()

    if not cellfields:
        raise RuntimeError("No cell fields associated with the mesh")

    cellfield = next(iter(cellfields))
    data = cellfield.get()

    for i in xrange(cellfield.size()):
        if i != data[i]:
            raise RuntimeError(
                "cell field was not correctly saved and retrieved")

    os.remove(mesh_path)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    collection_url = test_create_cell_field()
    test_read_cell_field(collection_url)
