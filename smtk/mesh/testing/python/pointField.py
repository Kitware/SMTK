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
import smtk.common
import smtk.io
import smtk.mesh
import smtk.testing
import sys


def test_create_point_field():

    # Load the mesh file
    mesh_path = os.path.join(smtk.testing.DATA_DIR, 'mesh', '2d/twoMeshes.h5m')
    c = smtk.mesh.Resource.create()
    smtk.io.importMesh(mesh_path, c)
    if not c.isValid():
        raise RuntimeError("Failed to read valid mesh")

    mesh = c.meshes()
    field = [i for i in range(c.meshes().points().size())]
    mesh.createPointField('point field', 1, field)

    write_path = ''
    write_path = os.path.join(smtk.testing.TEMP_DIR,
                              str(smtk.common.UUID.random()) + ".h5m")

    smtk.io.exportMesh(write_path, c)
    return write_path


def test_read_point_field(mesh_path):

    # Load the mesh file
    c = smtk.mesh.Resource.create()
    smtk.io.importMesh(mesh_path, c)
    if not c.isValid():
        raise RuntimeError("Failed to read back valid mesh")

    mesh = c.meshes()
    pointfields = mesh.pointFields()

    if not pointfields:
        raise RuntimeError("No point fields associated with the mesh")

    pointfield = next(iter(pointfields))
    data = pointfield.get()

    for i in range(pointfield.size()):
        if i != data[i]:
            raise RuntimeError(
                "point field was not correctly saved and retrieved")

    os.remove(mesh_path)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    resource_url = test_create_point_field()
    test_read_point_field(resource_url)
