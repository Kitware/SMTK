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

from smtk.attribute import SearchStyle
from _smtkPybindModel import *

_entity_map = {CELL_ENTITY: CellEntity,
               VERTEX: Vertex,
               EDGE: Edge,
               FACE: Face,
               VOLUME: Volume,
               USE_ENTITY: UseEntity,
               VERTEX_USE: VertexUse,
               EDGE_USE: EdgeUse,
               FACE_USE: FaceUse,
               VOLUME_USE: VolumeUse,
               SHELL_ENTITY: ShellEntity,
               CHAIN: Chain,
               LOOP: Loop,
               SHELL: Shell,
               GROUP_ENTITY: Group,
               MODEL_ENTITY: Model
               }

def _findEntitiesOfType(self, flags, exactMatch = True):
    array = self._findEntitiesOfType(int(flags), exactMatch)
    if len(array) == 0 or flags not in _entity_map:
        return []
    ent = _entity_map[flags]
    return [ent(x) for x in array]

Manager.findEntitiesOfType = _findEntitiesOfType

del _findEntitiesOfType
