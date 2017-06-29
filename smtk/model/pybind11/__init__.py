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

"""
operator: a decorator for adding a python-defined operator to a specific
session.

Use:

import smtk
import smtk.model

@smtk.operator(smtk.model.Session)
class MyOperator(smtk.model.Operator):
    ...
"""


def operator(nickname, opsession):
    def decorator(opclass):
        # check if a description function exists
        try:
            opclass.description()
        except AttributeError:
            # if the description function does not exist, next try to import
            # the compile-time generated description that resides in
            # <opclass_xml.py>
            import importlib
            try:
                desc_module = importlib.import_module(
                    opclass.__module__ + '_xml')

                def desc():
                    return desc_module.description

                opclass.description = staticmethod(desc)
            except:
                module = opclass.__module__.split(".")[-1]
                raise ImportError(
                    'Class "%s" has no description method and no "%s.sbt" file found.' % (module, module))

        from functools import partial

        # register this operator with the session
        opsession.registerStaticOperator(
            nickname, opclass.description(),
            partial(_smtkPybindModel.Operator.create, opclass.__module__,
                    opclass.__name__))

        return opclass
    return decorator

"""
smtk.model.Manager.findEntitiesOfType: a method that returns a list of queried
entity types. This method is composed in python (rather than in the bindings)
to exploit python's ability to return different objects, depending on the query.
"""
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


def _findEntitiesOfType(self, flags, exactMatch=True):
    array = self._findEntitiesOfType(int(flags), exactMatch)
    if len(array) == 0 or flags not in _entity_map:
        return array
    ent = _entity_map[flags]
    return [ent(x) for x in array]


Manager.findEntitiesOfType = _findEntitiesOfType

del _findEntitiesOfType
