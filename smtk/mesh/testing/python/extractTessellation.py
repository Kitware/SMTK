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
    import smtk.io
    import smtk.mesh
import smtk.testing
import sys


def test_file_load():
    m = smtk.mesh.Manager.create()

    # Load the mesh file
    print 'data_dir', smtk.testing.DATA_DIR
    mesh_path = os.path.join(smtk.testing.DATA_DIR,
                             'mesh', '3d/sixth_hexflatcore.h5m')
    c = smtk.io.importMesh(mesh_path, m)
    if not c.isValid():
        raise RuntimeError("Failed to read valid mesh")
    print c
    print c.numberOfMeshes()

    # tasks
    # 0. Generate a shell for the entire mesh
    meshes = c.meshes()
    shell = meshes.extractShell()
    if not (shell.cells().size() > 0):
        raise RuntimeError("Extract Shell failed")

    # 1. extract the tessellation of the shell
    tess = smtk.mesh.Tessellation()
    tess.extract(shell)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_file_load()
