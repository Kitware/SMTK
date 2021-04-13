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

from ._smtkPybindMesh import *


def _get(self, handleRange=None):
    if handleRange is None:
        if self.type() == FieldType.Double:
            return self._get_double()
        if self.type() == FieldType.Integer:
            return self._get_int()
    else:
        if self.type() == FieldType.Double:
            return self._get_double(handleRange)
        if self.type() == FieldType.Integer:
            return self._get_int(handleRange)


CellField.get = _get
PointField.get = _get
