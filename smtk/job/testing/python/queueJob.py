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

import os
import sys
import unittest
import shutil
import tempfile

import smtk
import smtk.plugin
import smtk.testing

relevantPlugins = set([
    'smtkResourcePlugin',
    'smtkAttributePlugin',
    'smtkOperationPlugin',
    'smtkJobPlugin',
    'smtkQtPlugin',
    'smtkPVJobExtPlugin'
])


def choosePlugin(pluginPath):
    """Return true if pluginPath corresponds to a relevant plugin for this test."""
    global relevantPlugins
    for plugin in relevantPlugins:
        if plugin in pluginPath:
            return True
    return False


class TestQueueJob(smtk.testing.TestCase):

    def setUp(self):
        smtk.io.Logger.instance().setFlushToStdout()
        # Load only relevant plugins
        plugins = [xx for xx in smtk.findAvailablePlugins()
                   if choosePlugin(xx)]
        smtk.loadPlugins(plugins)
        self.app = smtk.applicationContext()
        smtk.job.Registrar.registerTo(self.app)
        self.job_mgr = self.app.get('smtk.job.Manager')
        self.job_registry = smtk.plugin.registerPluginsTo(self.job_mgr)
        self.job_def = smtk.job.Definition.create()
        self.job_def.setName('JobQueueTest')
        self.job_def.setDescription('Test queueing jobs from python.')
        self.job_def.setScript('run.sh')
        self.job_def.appendStage('echo', 'Echo hello world', 'logs/echo.log')
        self.job_mgr.jobTypes().manage(self.job_def)

    def test_shell_queue_job(self):
        docPath = str(smtk.common.Paths().userDocumentDirectory(
            'SMTK/Testing/testQueueJob_'))
        temp_dir = tempfile.mkdtemp('_case', docPath)
        job = smtk.job.Job.create()
        job.setJobType(self.job_def)
        os.makedirs(os.path.join(temp_dir, 'logs'))
        job.setAutoSchedule(True)
        job.setCaseDirectory(temp_dir)
        # job.setLogParser(log, parser)
        # job.setLogs(['logs/echo.log'])
        queue = self.job_mgr.findQueueByName('shell_queue')
        job.setQueue(queue)
        scriptPath = os.path.join(job.caseDirectory(), job.jobType().script())
        with open(scriptPath, 'w') as script:
            print(f'#!/bin/bash', file=script)
            print(
                'SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )', file=script)
            print('cd ${SCRIPT_DIR}', file=script)
            print(f'echo 0 0 > logs/progress', file=script)
            print(f'echo Floopy > logs/echo.log', file=script)
            print(f'echo 1 0 > logs/progress', file=script)
        os.chmod(scriptPath, 0o755)
        job.setSize(2)
        print(f'queue is {queue.name()}')
        print(f'job in {scriptPath}')
        queue.schedule(job)
        print('job queue id', job.queueId())
        shutil.rmtree(temp_dir)

    def test_container_queue_job(self):
        docPath = str(smtk.common.Paths().userDocumentDirectory(
            'SMTK/Testing/testQueueJob_'))
        temp_dir = tempfile.mkdtemp('_case', docPath)
        job = smtk.job.Job.create()
        job.setJobType(self.job_def)
        os.makedirs(os.path.join(temp_dir, 'logs'))
        queue = self.job_mgr.findQueueByName('container_queue')
        job.setAutoSchedule(True)
        job.setCaseDirectory(temp_dir)
        job.setCaseDirectoryMountPoint('/home/openfoam')
        job.setContainerImage('docker.io/opencfd/openfoam-run:2412')
        # job.setLogParser(log, parser)
        # job.setLogs(['logs/echo.log'])
        job.setQueue(queue)
        scriptPath = os.path.join(job.caseDirectory(), job.jobType().script())
        with open(scriptPath, 'w') as script:
            print(f'#!/bin/bash', file=script)
            print(
                'SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )', file=script)
            print('cd ${SCRIPT_DIR}', file=script)
            print(f'echo 0 0 > logs/progress', file=script)
            print(f'echo Floopy > logs/echo.log', file=script)
            print(f'echo 1 0 > logs/progress', file=script)
        os.chmod(scriptPath, 0o755)
        job.setSize(2)
        print(f'queue is {queue.name()}')
        print(f'job in {scriptPath}')
        queue.schedule(job)
        print('job queue id', job.queueId())
        # TODO: Wait for job to complete
        shutil.rmtree(temp_dir)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
