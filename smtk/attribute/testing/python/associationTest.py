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
"""
Tests Attribute::associateEntity()
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    # Create smtk model with 1 group item
    mmgr = smtk.model.Manager.create()
    mask = smtk.model.FACE | smtk.model.GROUP_ENTITY
    group_item = mmgr.addGroup(mask, 'TopFaceBCS')

    # Create attribute manager with 1 def
    manager = smtk.attribute.Manager()
    manager.setRefModelManager(mmgr)
    defn = manager.createDefinition('testdef')
    defn.setAssociationMask(mask)

    # Create attribute and associate to group item
    att = manager.createAttribute('testatt', defn)
    att.associateEntity(group_item.entity())

    sys.exit(status)
