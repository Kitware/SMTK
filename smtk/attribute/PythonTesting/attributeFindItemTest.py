"""
Demonstrates bug that reappeared Oct 2103
"""

import os
import smtk

if __name__ == '__main__':
    import sys

    status = 0

    try:
        intdef = smtk.attribute.IntItemDefinition.New('intdef')

        manager = smtk.attribute.Manager()
        defn = manager.createDefinition('testdef')
        defn.addItemDefinition(intdef)

        att = manager.createAttribute('t1', 'testdef')

        item = att.item(0)
        cast_item = smtk.attribute.ValueItem.CastTo(item)
        print 'item:', cast_item.valueAsString()

        find_item = att.find('intdef')
        cast_item = smtk.attribute.ValueItem.CastTo(find_item)
        print 'find_item:', cast_item.valueAsString()
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
