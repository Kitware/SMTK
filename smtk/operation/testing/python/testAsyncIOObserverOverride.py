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
import smtk.common
import smtk.operation
import smtk.resource
import smtk.string
import smtk.testing

message = 'test'
addSelf = False
handlerCount = 0
observerCount = 0


class TestAsyncIOObserverOverride(smtk.testing.TestCase):
    """Test that python asynchronous observers run things in the proper order with
    SMTK resource locks held as needed."""

    def setUp(self):
        """Set up an environment with a resource and operation manager held
        by an application context (common manager) and an attribute resource
        with a single simple attribute definition."""
        self.app = smtk.common.Managers.create()
        smtk.resource.Registrar.registerTo(self.app)
        smtk.operation.Registrar.registerTo(self.app)
        rsrcMgr = self.app.get('smtk.resource.Manager')
        operMgr = self.app.get('smtk.operation.Manager')
        smtk.attribute.Registrar.registerTo(self.app)
        smtk.attribute.Registrar.registerTo(rsrcMgr)
        smtk.attribute.Registrar.registerTo(operMgr)
        # Create an attribute resource with a simple attribute definition
        self.rsrc = rsrcMgr.createResource('smtk::attribute::Resource')
        self.matDef = self.rsrc.createDefinition('Material')
        self.futures = []
        self.key = None

    def opHandler(self, op, result):
        """This method is added to some operations as a one-time "handler"
        for processing the operation's result."""
        global handlerCount
        handlerCount += 1
        print(f'  Handler for {op.typeName()}.')
        if addSelf:
            op.addHandler(self.opHandler, 0)
        return 0

    def opObserver(self, op, event, result):
        """This method is added to the operation manager as an observer of
        all operations and is called before and after the operation runs."""
        global observerCount
        observerCount += 1
        print(f'  Operation event {str(event)}')
        return 0

    def testSimple(self):
        """Create a python asyncio event loop and run it until a future completes.
        The future (submitOps()) launches operations whose observers/handlers
        must be called while locks are held and while the event loop is running.
        This is accomplished by SMTK's observer/handler code yielding rather
        than blocking."""
        global observerCount, handlerCount
        import asyncio
        loop = asyncio.new_event_loop()

        async def submitOps():
            print('Submitting operations.')
            operMgr = self.app.get('smtk.operation.Manager')
            op = operMgr.createOperation('smtk::attribute::CreateAttribute')
            op.parameters().associate(self.rsrc)
            op.parameters().findString('definition').setValue(0, 'Material')
            op.addHandler(self.opHandler, 0)
            op.addHandler(self.opHandler, 2)
            resultFuture = operMgr.launch(op)
            await resultFuture
            print('Operations completed.')
            return resultFuture

        operMgr = self.app.get('smtk.operation.Manager')
        operMgr.overrideObserversUsingAsyncIO(loop)
        self.key = operMgr.observers().insert(self.opObserver, 0, False, 'test observer')
        print('About to enter loop.')
        loop.run_until_complete(submitOps())
        print('Finished loop')
        loop.run_until_complete(loop.shutdown_asyncgens())
        loop.close()
        print('Test loop shutdown complete.')

        print(f'Observer fired {observerCount} times.')
        print(f'Handler fired {handlerCount} times.')
        self.assertEqual(observerCount, 2,
                         'Observer should be called exactly twice.')
        self.assertEqual(
            handlerCount, 2, 'Handler should be called exactly twice.')

        # Ensure we can shut down the event-loop-based observer override:
        operMgr.removeObserversOverride()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
