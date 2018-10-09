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
    mmgr = smtk.model.Resource.create()
    mask = smtk.model.FACE | smtk.model.GROUP_ENTITY
    group_item = mmgr.addGroup(mask, 'TopFaceBCS')

    # Create attribute resource with 1 def
    resource = smtk.attribute.Resource.create()
    defn = resource.createDefinition('testdef')
    resource.associate(mmgr)
    defn.setLocalAssociationMask(int(mask))
    defn.associationRule().setIsExtensible(True)

    # Create attribute and associate to group item
    att = resource.createAttribute('testatt', defn)
    a = att.associateEntity(group_item.entity())

    if a:
        print("Association Worked")
    else:
        print("Association Failed")
        status = -1

    uuids = att.associatedModelEntityIds()
    print("the number of ents associated with the att is ", len(uuids))

    # Check to see if the attribute is associated with the entity
    if att.isEntityAssociated(group_item):
        print("Attribute knows about the entity via UUID")
    else:
        print("Attribute does not know about the entity via UUID")
        status = -1

    # Check to see if the model entity knows about the attribute
    if group_item.hasAttributes():
        print("Model Entity does have attributes")
        if group_item.hasAttribute(att.id()):
            print("Model Entity has this attribute associated")
        else:
            print(
                "Model Entity does not have this attribute associated with it")
            status = -1
        uuids = group_item.attributeIds()
        print(
            "the number of attributes associated with the ent is ", len(uuids))
        # There should be only 1 attribute on it
        if len(uuids) == 1:
            print("Model Entity return correct attribute list size")
        if att.id() in uuids:
            print(
                "Model Entity did return the attribute via attributes() call")
    else:
        print("Model Entity does not have any attributes")

    sys.exit(status)
