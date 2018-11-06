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

from . import _smtkPybindVTKSourceFns as _srcfns
import vtkSMTKSourceExtPython


class vtkMeshMultiBlockSource(vtkSMTKSourceExtPython.vtkMeshMultiBlockSource):

    def GetModelResource(self):
        return _srcfns._vtkMeshMultiBlockSource_GetModelResource(self)

    def SetModelResource(self, resource):
        return _srcfns._vtkMeshMultiBlockSource_SetModelResource(self, resource)

    def GetMeshResource(self):
        return _srcfns._vtkMeshMultiBlockSource_GetMeshResource(self)

    def SetMeshResource(self, resource):
        return _srcfns._vtkMeshMultiBlockSource_SetMeshResource(self, resource)


class vtkModelMultiBlockSource(vtkSMTKSourceExtPython.vtkModelMultiBlockSource):

    def GetModelResource(self):
        return _srcfns._vtkModelMultiBlockSource_GetModelResource(self)

    def SetModelResource(self, resource):
        return _srcfns._vtkModelMultiBlockSource_SetModelResource(self, resource)
