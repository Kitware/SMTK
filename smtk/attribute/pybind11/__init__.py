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

import smtk.common
import smtk.resource
from _smtkPybindAttribute import *

"""
Several methods in smtk.attribute return a base item when queried. We wrap these
methods in a python <to_concrete> function that returns the most derived version
of that item.
"""
func_list = \
    [(Attribute, Attribute._find, "find"),
     (Attribute, Attribute._item, "item"),
     (Attribute, Attribute._itemAtPath, "itemAtPath"),
     (GroupItem, GroupItem._find, "find"),
     (ValueItem, ValueItem._activeChildItem, "activeChildItem"),
     (ValueItem, ValueItem._findChild, "findChild")]

type_dict = \
    {Item.ATTRIBUTE_REF:
     (RefItem, RefItemDefinition),
     Item.DOUBLE:
     (DoubleItem, DoubleItemDefinition),
     Item.GROUP:
     (GroupItem, GroupItemDefinition),
     Item.INT:
     (IntItem, IntItemDefinition),
     Item.STRING:
     (StringItem, StringItemDefinition),
     Item.VOID:
     (VoidItem, VoidItemDefinition),
     Item.FILE:
     (FileItem, FileItemDefinition),
     Item.DIRECTORY:
     (DirectoryItem, DirectoryItemDefinition),
     Item.COLOR:
     (None, None),
     Item.MODEL_ENTITY:
     (ModelEntityItem, ModelEntityItemDefinition),
     Item.MESH_ENTITY:
     (MeshItem, MeshItemDefinition),
     Item.MESH_SELECTION:
     (MeshSelectionItem, MeshSelectionItemDefinition),
     Item.DATE_TIME:
     (DateTimeItem, DateTimeItemDefinition)
     }


def to_concrete(item):
    '''
      Returns concrete (leaf) object for input, which is smtk.Item
    '''
    def fun(i):
        concrete_item = None
        for item_type, class_ in type_dict.items():
            if i.type() == item_type:
                try:
                    concrete_item = class_[0].CastTo(i)
                    break
                except TypeError:
                    concrete_item = class_[1].CastTo(i)
                    break
        if concrete_item is None:
            print('WARNING - unsupported type %s, item %s' %
                  (i.type(), i.name()))
        return concrete_item
    if isinstance(item, list):
        return [fun(x) for x in item]
    elif isinstance(item, tuple):
        return tuple([fun(x) for x in item])
    elif isinstance(item, set):
        return set([fun(x) for x in item])
    return fun(item)


def get_wrapped_func(meth):
    def wrapped_func(*args, **kwargs):
        return to_concrete(meth(*args, **kwargs))
    return wrapped_func

for (cls, meth, new_func_name) in func_list:
    setattr(cls, new_func_name, get_wrapped_func(meth))


def to_concrete_passthrough(item):
    import warnings
    warnings.warn(
        "Call to deprecated function \"to_concrete()\". As of 1.1, smtk.attribute methods return derived item types.")
    return item

to_concrete = to_concrete_passthrough
