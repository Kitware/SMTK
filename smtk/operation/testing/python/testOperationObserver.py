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
import smtk.string
import smtk.common
import smtk.io
import smtk.resource
import smtk.attribute
import smtk.operation
import smtk.session.polygon
import smtk.testing

invokedCount = 0
message = 'test'
addSelf = False


def observer(op, event, result):
    global message, invokedCount
    invokedCount += 1
    print(f'{message}: Observer invoked ({invokedCount}) for {op.typeName()} {str(event)}')
    return 0


class TestOperationObserver(smtk.testing.TestCase):

    def setUp(self):
        self.ctxt = smtk.applicationContext()
        self.resource_manager = smtk.resource.Manager.create()
        self.operation_manager = smtk.operation.Manager.create()
        self.operation_manager.registerResourceManager(self.resource_manager)
        self.ctxt.insertOrAssign(self.resource_manager)
        self.ctxt.insertOrAssign(self.operation_manager)

        smtk.resource.Registrar.registerTo(self.ctxt)
        # smtk.resource.Registrar.registerTo(self.resource_manager)

        smtk.operation.Registrar.registerTo(self.ctxt)
        smtk.operation.Registrar.registerTo(self.operation_manager)

        # smtk.attribute.Registrar.registerTo(self.ctxt)
        smtk.attribute.Registrar.registerTo(self.resource_manager)
        smtk.attribute.Registrar.registerTo(self.operation_manager)

        smtk.session.polygon.Registrar.registerTo(self.operation_manager)
        smtk.session.polygon.Registrar.registerTo(self.resource_manager)

        self.key = self.operation_manager.observers().insert(observer, 'Test observer')

    def testFreeObserver(self):
        import os
        global message, invokedCount

        invokedCount = 0
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = self.operation_manager.createOperation(
            'smtk::session::polygon::Read')
        print('op is', op)
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        print(os.path.join(*fpath))
        invokedCount = 0
        message = 'testFreeObserver'
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.assertEqual(
            invokedCount, 2, 'Expected to be called exactly twice.')

    def methodObserver(self, op, event, result):
        self.invokedCount += 1
        print(
            f'{self.message}: Observer invoked ({self.invokedCount}) for {op.typeName()} {str(event)}')
        if event == smtk.operation.EventType.WILL_OPERATE:
            # Abort the operation
            return 1
        return 0

    def testMethodObserver(self):
        import os
        self.key = self.operation_manager.observers().insert(
            self.methodObserver, 'Test method observer')
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = self.operation_manager.createOperation(
            'smtk::session::polygon::Read')
        print('op is', op)
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        print(os.path.join(*fpath))
        self.invokedCount = 0
        self.message = 'testFreeObserver'
        res = op.operate()
        self.assertEqual(smtk.operation.outcome(res), smtk.operation.Operation.CANCELED,
                         'Expected to be canceled.')
        self.assertEqual(self.invokedCount, 2,
                         'Expected to be called exactly once.')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
