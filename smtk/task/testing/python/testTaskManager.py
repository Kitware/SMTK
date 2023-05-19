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
import unittest
import smtk
import smtk.attribute
import smtk.task
import smtk.testing
import re


class TestTaskManager(smtk.testing.TestCase):

    def testTaskManager(self):
        import smtk.common
        import smtk.plugin
        import smtk.task
        import json

        print('Test task-manager creation.')
        # Verify we can create a task manager
        mgr = smtk.task.Manager.create()
        assert (mgr)
        assert (mgr.typeName() == 'smtk::task::Manager')

        print('Test task-manager registration.')
        # Register core task types to manager
        smtk.task.Registrar.registerTo(mgr)

        # Verify we can create tasks from JSON
        cfg = {
            'name': 'Task 5',
            'auto-configure': True,
            'resources': [
                {
                    'type': 'smtk::model::Resource',
                    'role': 'simulation attribute'
                }
            ]
        }
        print('Test task creation.')
        foa = mgr.taskInstances().createFromName(
            'smtk::task::FillOutAttributes', json.dumps(cfg))
        assert (foa != None)
        print('{:1} (an instance of {:2}), {:3}'.format(
            foa.name(), foa.typeName(), str(foa.state())))
        assert (foa.name() == cfg['name'])
        assert (foa.typeName() == 'smtk::task::FillOutAttributes')
        assert (foa.state() == smtk.task.State.Completable)
        foa.markCompleted(True)
        assert (foa.state() == smtk.task.State.Completed)

        # Verify the instance we created is managed
        print('All managed task instances:')
        tasks = mgr.taskInstances()
        print('\n'.join(['  {:1} (an instance of {:2}'.format(
            x.name(), x.typeName()) for x in tasks.instances()]))
        assert (len(tasks.instances()) == 1)

        # Test getting/setting the manager's active task.
        # Initially there should be no active task.
        activeTask = mgr.active().task()
        print('Active task is {:1}'.format(
            activeTask.name() if activeTask else '(null)'))
        assert (activeTask == None)

        # We should be able to switch the active task.
        mgr.active().switchTo(foa)
        activeTask = mgr.active().task()
        print('Active task is {:1}'.format(
            activeTask.name() if activeTask else '(null)'))
        assert (activeTask == foa)

        # We should be able to reset the active task to its initial state.
        mgr.active().switchTo(None)
        activeTask = mgr.active().task()
        print('Active task is {:1}'.format(
            activeTask.name() if activeTask else '(null)'))
        assert (activeTask == None)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
