# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================
import smtk
import smtk.attribute
import smtk.operation
import smtk.session.polygon
import smtk.testing

invokedCount = 0
message = 'test'
addSelf = False


def handler(op, result):
    global message, invokedCount
    invokedCount += 1
    print('%s: Handler invoked (%d)' % (message, invokedCount))
    if addSelf:
        op.addHandler(handler, 0)


class TestOperationHandler(smtk.testing.TestCase):

    def testSingleHandler(self):
        import os
        global message, invokedCount
        self.mgr = smtk.model.Resource.create()
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = smtk.session.polygon.Read.create()
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        print
        op.addHandler(handler, 0)
        op.addHandler(handler, 1)
        op.removeHandler(handler, 1)
        invokedCount = 0
        message = 'testSingleHandler'
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 1, 'Expected to be called exactly once.')

    def testMultipleHandlers(self):
        import os
        global message, invokedCount
        self.mgr = smtk.model.Resource.create()
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = smtk.session.polygon.Read.create()
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        op.addHandler(handler, 0)
        op.addHandler(handler, 1)
        invokedCount = 0
        message = 'testMultipleHandlers'
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 2, 'Expected to be called exactly twice.')

    def testMultipleTimes(self):
        import os
        global message, invokedCount, addSelf
        self.mgr = smtk.model.Resource.create()
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = smtk.session.polygon.Read.create()
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        op.addHandler(handler, 0)
        op.addHandler(handler, 1)
        addSelf = True
        invokedCount = 0
        message = 'testMultipleTimes (first time)'
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 2, 'Expected to be called exactly twice.')

        # Invoke the operation a second time.
        invokedCount = 0
        message = 'testMultipleTimes (second time)'
        # Remove one of the handlers that got re-added since "addSelf" was true.
        op.removeHandler(handler, 0)
        addSelf = False
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 1, 'Expected to be called after first operation.')

        # Invoke the operation a third time (with no re-add of handler).
        invokedCount = 0
        message = 'testMultipleTimes (third time)'
        addSelf = False
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 0, 'Expected to be called no more after the first operation.')

        # Test that no handlers remain on the operation
        didRemove = op.clearHandlers()
        self.assertFalse(didRemove, 'Expected no handlers remaining.')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
