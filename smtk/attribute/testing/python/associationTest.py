"""
Tests Attribute::associateEntity()
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    # Create smtk model with 1 group item
    model = smtk.model.Model.New()
    mask = smtk.model.Item.FACE | smtk.model.Item.GROUP  # 36
    group_item = model.createModelGroup('TopFaceBCS', 37, mask)

    # Create attribute manager with 1 def
    manager = smtk.attribute.Manager()
    manager.setRefModel(model)
    defn = manager.createDefinition('testdef')
    defn.setAssociationMask(mask)

    # Create attribute and associate to group item
    att = manager.createAttribute('testatt', defn)
    att.associateEntity(group_item)

    sys.exit(status)
