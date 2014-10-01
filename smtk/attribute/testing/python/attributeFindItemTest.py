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
Demonstrates bug that reappeared Oct 2103
"""

import os
import smtk

if __name__ == '__main__':
    import sys

    status = 0

    try:
        groupdef = smtk.attribute.GroupItemDefinition.New('groupdef')
        intdef = smtk.attribute.IntItemDefinition.New('intdef')
        groupdef.addItemDefinition(intdef)

        system = smtk.attribute.System()
        defn = system.createDefinition('testdef')
        defn.addItemDefinition(groupdef)

        att = system.createAttribute('t1', 'testdef')

        # Retrieve GroupItem from attribute
        item = att.item(0)
        group_item = smtk.attribute.GroupItem.CastTo(item)
        print 'group_item:', group_item

        find_item = att.find('groupdef')
        find_group_item = smtk.attribute.GroupItem.CastTo(item)
        print 'find_group_item:', find_group_item

        # Retieve IntItem from GroupItem
        subitem = group_item.item(0)
        int_subitem = smtk.attribute.ValueItem.CastTo(subitem)
        print 'subitem:', int_subitem.valueAsString()

        find_subitem = find_group_item.find('intdef')
        find_int_subitem = smtk.attribute.ValueItem.CastTo(find_subitem)
        print 'find_int_subitem:', find_int_subitem.valueAsString()
    except Exception, ex:
        print 'Exception:'

        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        print
        print 'Exception: ', exc_type, fname, 'line', exc_tb.tb_lineno
        print

        print ex
        print
        status = -1

    sys.exit(status)
