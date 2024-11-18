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
import os
import sys


class TestTaskAgents(smtk.testing.TestCase):

    def setUp(self):
        """Load plugins"""
        # Reset the smtk.common.Managers object used by smtk.read() to eliminate
        # previously-read resources from memory:
        if smtk.appContext:
            rsrcMgr = smtk.appContext.get('smtk.resource.Manager')
            for rsrc in rsrcMgr.resources():
                rsrcMgr.remove(rsrc)

        self.context = smtk.applicationContext()
        plugins = smtk.findAvailablePlugins()
        smtk.loadPlugins(plugins)
        sys.path.append(os.path.join(smtk.testing.DATA_DIR, 'operations'))

    def testTaskAgents(self):
        import smtk.common
        import smtk.string
        import smtk.plugin
        import smtk.io
        import smtk.resource
        import smtk.attribute
        import smtk.project
        import smtk.task
        import json
        import os
        from smtk.string import Token as token

        print('Test agent creation via project load.')
        ctxt = self.context
        projFile = os.path.join(
            smtk.testing.DATA_DIR, 'projects', 'PortDataObjects', 'portDataToReferenceItem.smtk')
        resources = smtk.read(projFile, ctxt)
        self.assertEqual(len(resources), 1)
        proj = resources[0]

        print('Fetch tasks from project''s task manager.')
        taskInst = proj.taskManager().taskInstances().instances()
        # taskDict = {x.name(): x for x in taskInst}
        # opTask1 = taskDict['Stage 1 Operation']
        # opTask2 = taskDict['Stage 2 Operation']
        # atTask1 = taskDict['Stage 1 Attributes']
        # atTask2 = taskDict['Stage 2 Attributes']

        print('Test agent API')
        for task in taskInst:
            print(' '*4 + task.name())
            agents = [x for x in task.agents()]
            for agent in agents:
                if agent.matchesType(token('smtk::task::GatherObjectsAgent')):
                    role = smtk.string.Token('project')
                    agent.addObjectInRole(proj, role, True)
                    data = agent.data().data()
                    print(' '*8 + 'GatherObjectsAgent')
                    for key, value in data.items():
                        print(' '*12 + 'Role', key.data())
                        for vv in value:
                            print(' '*16 + '', vv.typeName(), '  ', vv.name())
                    assert (role in data)
                    assert (len(data[role]) == 1)
                    assert (proj in data[role])
                    agent.clearOutputPort()
                    assert (agent.data() == None)
                    print(' '*12 + 'Output port', agent.outputPort().name())
                elif agent.matchesType(token('smtk::task::FillOutAttributesAgent')):
                    print(' '*8 + 'FillOutAttributesAgent')
                    print(' '*12 + 'Input  port', agent.inputPort().name())
                    print(' '*12 + 'Output port', agent.outputPort().name())
                elif agent.matchesType(token('smtk::task::SubmitOperationAgent')):
                    op = agent.operation()
                    print(' '*8 + 'SubmitOperationAgent')
                    print(' '*12 + 'Operation', op.typeName() if op else 'null')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
