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
from smtk import attribute
from smtk import model

if __name__ == '__main__':
    import sys
    import os

    status = 0

    # Create smtk model with 1 group item
    mmgr = smtk.model.Manager.create()
    mask = smtk.model.FACE | smtk.model.GROUP_ENTITY
    group_item = mmgr.addGroup(mask, 'TopFaceBCS')

    # Create attribute resource with 1 def
    resource = smtk.attribute.Resource.create()
    defn = resource.createDefinition('testdef')
    resource.setRefModelManager(mmgr)
    defn.setLocalAssociationMask(int(mask))

    # Create attribute and associate to group item
    att = resource.createAttribute('testatt', defn)
    att.associateEntity(group_item.entity())

    sys.exit(status)
