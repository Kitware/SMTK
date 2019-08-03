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
"""Utility functions for simulation-export tests"""

import smtk
import smtk.view


def _instantiate_instanced_view(att_resource, view):
    """Initialize attributes specified in instanced view

    """
    comp = view.details()
    atts_comp = comp.child(0)
    for i in range(atts_comp.numberOfChildren()):
        att_comp = atts_comp.child(i)
        att_name = att_comp.attributes().get('Name')
        att_type = att_comp.attributes().get('Type')

        # Attributes can appean in multiple instanced views, so check if
        # att has already been created.
        att = att_resource.findAttribute(att_name)
        if att is not None:
            return

        defn = att_resource.findDefinition(att_type)
        if defn is None:
            raise RuntimeError('Definition {} not found'.format(att_type))
        att = att_resource.createAttribute(att_name, defn)
        # print('Created attribute \"{}\" type \"{}\"').format(att_name,
        # att_type)


def _recursive_create_instanced_atts(att_resource, comp):
    """Traverse view components to find/create instanced attributes

    """
    if comp is None:
        raise RuntimeError('Component is None')

    if comp.name() == 'View':
        title = comp.attributes().get('Title')
        view = att_resource.findView(title)
        if view is None:
            raise RuntimeError('View {} not found'.format(title))

        if view.type() == 'Instanced':
            _instantiate_instanced_view(att_resource, view)
        else:
            _recursive_create_instanced_atts(att_resource, view.details())
        return

    # (else) process component children
    for i in range(comp.numberOfChildren()):
        child = comp.child(i)
        _recursive_create_instanced_atts(att_resource, child)


def create_instanced_atts(att_resource):
    """Instantiates all attributes referenced in Instanced views

    Code doesn't instantiate *all* instanced views, but instead
    traverses from top-level view to find those instanced views
    that will be displayed.
    """
    top_view = att_resource.findTopLevelView()
    if top_view is None:
        print('WARNING: Attribute resource has no top-level view')
        return
    _recursive_create_instanced_atts(att_resource, top_view.details())
