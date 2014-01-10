"""
Demonstrate model construction from within Python.
"""

import os
import smtk

if __name__ == '__main__':
  import sys
  status = 0
  try:
    store = smtk.model.Storage.create()
    #model = store.addModel(3, 3, 'Test Model')
    volume = smtk.model.Entity(smtk.model.CELL_ENTITY, 3)
    f1 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f2 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f3 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f4 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    u01 = store.addEntity(f1)
    u02 = store.addEntity(f2)
    u03 = store.addEntity(f3)
    u04 = store.addEntity(f4)
    volume.appendRelation(u01).appendRelation(u02).appendRelation(u03).appendRelation(u04)#.appendRelation(model.entity())
    u00 = store.addEntity(volume)
    # Now verify that the faces refer back to the volume:
    status = 0 if store.findEntity(u01).relations()[0] == u00 else 1
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
