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
import smtk.io
import smtk.mesh
import smtk.testing
import sys


def test_file_load():

    # Load the mesh file
    print('data_dir', smtk.testing.DATA_DIR)
    mesh_path = os.path.join(smtk.testing.DATA_DIR,
                             'mesh', '3d/sixth_hexflatcore.h5m')
    mr = smtk.mesh.Resource.create()
    smtk.io.importMesh(mesh_path, mr)
    if not mr.isValid():
        raise RuntimeError("Failed to read valid mesh")
    print(mr)
    print(mr.numberOfMeshes())

    # now dump some very basic info about the resource
    print(mr.meshes().size())
    print(mr.cells().size())
    print(mr.points().size())

    # now dump some very basic info about all the meshes
    m = mr.meshes()
    print('info on all meshes')
    print(len(m.domains()))
    print(len(m.dirichlets()))
    print(len(m.neumanns()))
    print(m.cells().size())
    print(m.points().size())


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_file_load()
