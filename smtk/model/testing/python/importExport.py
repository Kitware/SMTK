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
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.attribute
    import smtk.bridge.exodus
    import smtk.model
from smtk.simple import *
import smtk.testing
import sys
import tempfile

class TestExportImport(smtk.testing.TestCase):

  def setUp(self):
    self.mgr = smtk.model.Manager.create()

  def checkPillbox(self, original, imported):
    print '    {:1} models imported'.format(len(imported))
    self.assertEqual(len(original), len(imported), 'Expected number of models to match')

    for i in range(len(original)):
     print '    Model {:1}: {:2}'.format(original[i].name(), original[i].entity())
     self.assertEqual(original[i].entity(), imported[i].entity(), \
        'Entity IDs {:1} and {:2} differ'.format(original[i].entity(), imported[i].entity()))

    return True

  def checkExportImport(self, sessionType, filename, **kwargs):

    print 'Creating {:1} session'.format(sessionType)
    sess = self.mgr.createSession(sessionType)
    sess.assignDefaultName()
    SetActiveSession(sess)

    print 'Loading test model {:1}'.format(filename)
    models = Read(filename)
    numModels = len(models) if hasattr(models,'__iter__') else 0 if models == None else 1
    if numModels == 1 and not hasattr(models,'__iter__'):
      models = [models,]
    print '  Read {:1} top-level entities'.format(numModels)
    self.assertGreater(numModels, 0, 'Must read at least one model to test.')

    # Export and then import
    #   Generate a temporary filename
    ftmp = os.path.join(tempfile.mkdtemp(), 'test.json')
    #   Export to ftmp
    exp = sess.op('export smtk model')
    SetVectorValue(exp.findFile('filename', int(smtk.attribute.SearchStyle.ACTIVE_CHILDREN)), [ftmp,])
    SetVectorValue(exp.specification().associations(), models)
    result = exp.operate()
    PrintResultLog(result)
    self.assertEqual(result.findInt('outcome').value(0), smtk.model.OPERATION_SUCCEEDED, 'Could not export model')
    #   Create a new session and model manager
    mm2 = smtk.model.Manager.create()
    se2 = mm2.createSession(sessionType)
    SetActiveSession(se2)
    #   Import from ftmp
    imp = se2.op('import smtk model')
    SetVectorValue(imp.findFile('filename', int(smtk.attribute.ACTIVE_CHILDREN)), [ftmp,])
    result = imp.operate()
    PrintResultLog(result)
    self.assertEqual(result.findInt('outcome').value(0), smtk.model.OPERATION_SUCCEEDED, 'Could not import model')
    cre = result.findModelEntity('created')
    imported = [cre.value(i) for i in range(cre.numberOfValues())]

    # Check the re-imported model against the original:
    if 'modelChecker' in kwargs:
      print '  Checking model'
      self.assertTrue(kwargs['modelChecker'](models, imported), '    Model check failed.')

    # Try an image test if we have VTK and a baseline:
    canRender = self.haveVTK() and self.haveVTKExtension()
    haveBaseline = 'baseline' in kwargs and os.path.isfile(kwargs['baseline'])
    if imported is not None and canRender and haveBaseline:
      print '  Rendering exported-then-imported model'
      self.startRenderTest()
      [self.addModelToScene(model) for model in imported]
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      self.assertImageMatchIfFileExists(kwargs['baseline'])
      self.interact()

  def testImportExport(self):
    sequence = [ \
        {'sessionType':'exodus', 'filename':smtk.testing.find_data(['model', '3d', 'netcdf', 'pillbox.ncdf']), 'modelChecker':self.checkPillbox}, \
    ]
    for test in sequence:
      self.checkExportImport(**test)

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
