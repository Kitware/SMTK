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
    {Item.AttributeRefType:
     (RefItem, RefItemDefinition),
     Item.DoubleType:
     (DoubleItem, DoubleItemDefinition),
     Item.GroupType:
     (GroupItem, GroupItemDefinition),
     Item.IntType:
     (IntItem, IntItemDefinition),
     Item.StringType:
     (StringItem, StringItemDefinition),
     Item.VoidType:
     (VoidItem, VoidItemDefinition),
     Item.FileType:
     (FileItem, FileItemDefinition),
     Item.DirectoryType:
     (DirectoryItem, DirectoryItemDefinition),
     Item.ColorType:
     (None, None),
     Item.ModelEntityType:
     (ModelEntityItem, ModelEntityItemDefinition),
     Item.MeshEntityType:
     (MeshItem, MeshItemDefinition),
     Item.MeshSelectionType:
     (MeshSelectionItem, MeshSelectionItemDefinition),
     Item.DateTimeType:
     (DateTimeItem, DateTimeItemDefinition)
     }


def _to_concrete(item):
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
        return _to_concrete(meth(*args, **kwargs))
    return wrapped_func

for (cls, meth, new_func_name) in func_list:
    setattr(cls, new_func_name, get_wrapped_func(meth))


def to_concrete_passthrough(item):
    from inspect import getframeinfo, stack
    import warnings
    caller = getframeinfo(stack()[1][0])
    warnings.warn(
        "%s:%d: Call to deprecated function \"to_concrete()\". As of 1.1, smtk.attribute methods return derived item types." % (caller.filename, caller.lineno))
    return item

to_concrete = to_concrete_passthrough
