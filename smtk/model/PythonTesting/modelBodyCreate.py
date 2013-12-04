"""
Demonstrate model construction from within Python.
"""

import os
import smtk

if __name__ == '__main__':
  import sys
  status = 0
  try:
    volume = smtk.model.Entity(smtk.model.CELL_ENTITY, 3)
    f1 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f2 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f3 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f4 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    mb = smtk.model.Storage()
    u01 = mb.addEntity(f1)
    u02 = mb.addEntity(f2)
    u03 = mb.addEntity(f3)
    u04 = mb.addEntity(f4)
    volume.appendRelation(u01).appendRelation(u02).appendRelation(u03).appendRelation(u04)
    u00 = mb.addEntity(volume)
    # Now verify that the faces refer back to the volume:
    status = 0 if mb.findEntity(u01).relations()[0] == u00 else 1
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
