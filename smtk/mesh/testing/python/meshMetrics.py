# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================


import os
import smtk
import smtk.io
import smtk.mesh
import smtk.testing
import sys


def test_mesh_metrics():
    # Load the mesh file
    print('data_dir', smtk.testing.DATA_DIR)
    mesh_path = os.path.join(smtk.testing.DATA_DIR,
                             'mesh', '3d/cube_with_hole.exo')
    mr = smtk.mesh.Resource.create()
    mr.setName('foo')
    smtk.io.importMesh(mesh_path, mr)
    if not mr.isValid():
        raise RuntimeError("Failed to read valid mesh")

    ext = smtk.mesh.extent(mr.meshes())
    tolerance = 1.e-8
    for i in range(0, 6):
        if i % 2 == 0:
            assert (abs(ext[i] + .5) < tolerance)
        else:
            assert (abs(ext[i] - .5) < tolerance)

    dim = smtk.mesh.highestDimension(mr.meshes())
    assert (dim == smtk.mesh.Dims3)

    euler_characteristic = smtk.mesh.eulerCharacteristic(mr.meshes())
    assert (euler_characteristic == 0)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_mesh_metrics()
