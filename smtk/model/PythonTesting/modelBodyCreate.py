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
    # Add some models to storage:
    model = store.addModel(3, 3, 'Test Model')
    model2 = store.addModel(3, 3, 'Submodel A')
    model3 = store.addModel(3, 3, 'Submodel B')
    # Add a group
    group = store.addGroup(0, 'Test Group')
    # Add a volume and 4 faces the hard way:
    volume = smtk.model.Entity(smtk.model.CELL_ENTITY, 3)
    f1 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f2 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f3 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    f4 = smtk.model.Entity(smtk.model.CELL_ENTITY, 2)
    u01 = store.addEntity(f1)
    u02 = store.addEntity(f2)
    u03 = store.addEntity(f3)
    u04 = store.addEntity(f4)
    volume.appendRelation(u01).\
      appendRelation(u02).\
      appendRelation(u03).\
      appendRelation(u04).\
      appendRelation(model.entity())
    u00 = store.addEntity(volume)
    # Now verify that the faces refer back to the volume:
    status = False if store.findEntity(u01).relations()[0] == u00 else True
    # Add cells to the group, the group and cells to the model, submodels to the model:
    [store.findOrAddEntityToGroup(group.entity(), x) for x in [u00,u01,u02,u03,u04]]
    [model.addCell(smtk.model.CellEntity(x)) for x in group.members()]
    store.assignDefaultNames()
    model.addGroup(group)
    model.addSubmodel(model2)
    model.addSubmodel(model3)
    # Does the model contain the cells we just added?
    enames = sorted([x.name() for x in model.cells()])
    print '\n'.join(enames)
    status = status or len(enames) != 5 or \
      (enames[0] != 'Test Model, face 0')
    # Does the model contain the group we added?
    status = status or len(model.groups()) != 1 or \
      model.groups()[0].name() != 'Test Group'
    # Does the model contain the submodels we added?
    status = status or len(model.submodels()) != 2 or \
      sorted([x.name() for x in model.submodels()])[0] != 'Submodel A'
  except Exception, ex:
    print 'Exception:'

    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print
    print 'Exception: ', exc_type, fname, 'line', exc_tb.tb_lineno
    print
    print ex
    print
    status = True

  sys.exit(0 if not status else 1)
