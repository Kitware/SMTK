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

from . import _smtkPybindVTKSourceFns as _srcfns
from .._modules import vtkSMTKSourceExt


class vtkResourceMultiBlockSource(vtkSMTKSourceExt.vtkResourceMultiBlockSource):

    @staticmethod
    def GetDataObjectUUID(info):
        return _srcfns._vtkResourceMultiBlockSource_GetDataObjectUUID(info)

    @staticmethod
    def SetDataObjectUUID(info, uuid):
        return _srcfns._vtkResourceMultiBlockSource_SetDataObjectUUID(info, uuid)

    def GetComponent(self, info):
        return _srcfns._vtkResourceMultiBlockSource_GetComponent(self, info)

    def GetResource(self):
        return _srcfns._vtkResourceMultiBlockSource_GetResource(self)

    def SetResource(self, resource):
        return _srcfns._vtkResourceMultiBlockSource_SetResource(self, resource)


class vtkModelMultiBlockSource(vtkSMTKSourceExt.vtkModelMultiBlockSource):

    @staticmethod
    def GetDataObjectUUID(info):
        return _srcfns._vtkModelMultiBlockSource_GetDataObjectUUID(info)

    @staticmethod
    def SetDataObjectUUID(info, uuid):
        return _srcfns._vtkModelMultiBlockSource_SetDataObjectUUID(info, uuid)

    def GetComponent(self, info):
        return _srcfns._vtkModelMultiBlockSource_GetComponent(self, info)

    def GetModelResource(self):
        return _srcfns._vtkModelMultiBlockSource_GetModelResource(self)

    def SetModelResource(self, resource):
        return _srcfns._vtkModelMultiBlockSource_SetModelResource(self, resource)
