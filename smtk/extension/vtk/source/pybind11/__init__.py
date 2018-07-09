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

import _smtkPybindVTKSourceFns as _srcfns
import vtkSMTKSourceExtPython


class vtkMeshMultiBlockSource(vtkSMTKSourceExtPython.vtkMeshMultiBlockSource):

    def GetModelResource(self):
        return _srcfns._vtkMeshMultiBlockSource_GetModelResource(self)

    def SetModelResource(self, resource):
        return _srcfns._vtkMeshMultiBlockSource_SetModelResource(self, resource)

    def GetMeshManager(self):
        return _srcfns._vtkMeshMultiBlockSource_GetMeshManager(self)

    def SetMeshManager(self, manager):
        return _srcfns._vtkMeshMultiBlockSource_SetMeshManager(self, manager)


class vtkModelMultiBlockSource(vtkSMTKSourceExtPython.vtkModelMultiBlockSource):

    def GetModelResource(self):
        return _srcfns._vtkModelMultiBlockSource_GetModelResource(self)

    def SetModelResource(self, resource):
        return _srcfns._vtkModelMultiBlockSource_SetModelResource(self, resource)
