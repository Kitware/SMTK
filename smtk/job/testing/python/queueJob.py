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
from pathlib import Path
import shutil
import tempfile
import time
import unittest
import uuid

# Supplied by queueJobPythonDriver, which owns the Qt application for this test.
from _queueJobTest import process_events

import smtk
import smtk.attribute
import smtk.io
import smtk.plugin
import smtk.testing


class TestQueueJob(smtk.testing.TestCase):
    """Exercise Python queue bindings through job completion and output validation."""

    @classmethod
    def setUpClass(cls):
        # Load queue support without smtkPVJobExtPlugin: its artifact callbacks
        # require a full ParaView GUI application, which this headless test lacks.
        relevant_plugins = {
            'smtkResourcePlugin', 'smtkAttributePlugin', 'smtkOperationPlugin',
            'smtkJobPlugin', 'smtkQtPlugin'
        }
        plugins = [path for path in smtk.findAvailablePlugins()
                   if any(name in path for name in relevant_plugins)]
        loaded, skipped = smtk.loadPlugins(plugins)
        if not loaded or skipped:
            raise RuntimeError(f'Could not load job test plugins: {skipped}')
        from smtk import qt
        cls.process_events = staticmethod(process_events)
        cls.qt = qt
        cls.app = smtk.applicationContext()
        smtk.job.Registrar.registerTo(cls.app)
        cls.job_mgr = cls.app.get('smtk.job.Manager')
        cls.job_registry = smtk.plugin.registerPluginsTo(cls.job_mgr)

    def setUp(self):
        self.job_def = smtk.job.Definition.create()
        self.job_def.setName('JobQueueTest')
        self.job_def.setScript('run.sh')
        self.job_def.appendStage('echo', 'Echo hello world', 'logs/echo.log')
        self.job_mgr.jobTypes().manage(self.job_def)

    def run_job(self, container=False, exit_code=0):
        """Run a script and check its terminal state, exit status, and output files."""
        # Keep cases inside the build tree, including the directory mounted in
        # Podman's VM. Do not use or remove the user's Documents directory.
        root = Path(tempfile.mkdtemp(prefix='queueJobPy_', dir=os.getcwd()))
        job = None
        terminal = (smtk.job.State.Completed, smtk.job.State.Canceled)
        try:
            # A fresh UUID prevents restoring database records from earlier runs.
            # Test queues remove their own records when they are destroyed.
            kwargs = dict(
                name='queueJobPy_container' if container else 'queueJobPy_shell',
                description='Python queue integration test', location='localhost',
                max_job_size=2, remove_queue_on_destruction=True, uid=str(uuid.uuid4()),
                resource_manager=self.app.get('smtk.resource.Manager'),
                operation_manager=self.app.get('smtk.operation.Manager'),
                job_manager=self.job_mgr)
            if container:
                # Initialize synchronously below, rather than launching a second
                # machine-update operation from the constructor on macOS.
                operation_manager = kwargs['operation_manager']
                kwargs['operation_manager'] = None
                queue = self.qt.ContainerQueue.create_or_restore(
                    **kwargs, container_engine_executable=shutil.which('podman'),
                    root_job_directory=str(root))
                queue.setOperationManager(operation_manager)
                operation = self.app.get('smtk.operation.Manager').createOperation(
                    'smtk::qt::job::UpdateContainerQueueMachine')
                self.assertIsNotNone(operation)
                self.assertTrue(operation.parameters().associate(queue))
                result = operation.operate()
                self.assertEqual(result.findInt('outcome').value(),
                                 int(smtk.operation.Operation.Outcome.SUCCEEDED),
                                 'Container runtime initialization failed:\n' +
                                 operation.log().convertToString())
                image = 'docker.io/library/ubuntu:26.04'
                # Pull explicitly so runtime and registry failures are reported
                # before attempting to schedule a container job.
                self.assertTrue(queue.pullContainerImage(image),
                                'Container image pull failed:\n' +
                                smtk.io.Logger.instance().convertToString())
            else:
                queue = self.qt.ShellQueue.create_or_restore(**kwargs)
                queue.setInterpreter(shutil.which('bash'))
                queue.setInterpreterArguments(['--noprofile', '--norc'])

            self.job_mgr.queues().manage(queue)
            job = smtk.job.Job.create()
            job.setJobType(self.job_def)
            job.setQueue(queue)
            job.setCaseDirectory(str(root))
            job.setSize(1)
            if container:
                job.setContainerImage(image)
                job.setCaseDirectoryMountPoint('/work')
            (root / 'logs').mkdir()
            # Leave the failed process alive across several progress polls after
            # reporting its final stage. Progress alone must not imply success.
            # The "1 0" record deliberately disagrees with the eventual exit 7:
            # this exposed premature success when polling won the race with exit.
            before_exit = 'sleep 1\n' if exit_code != 0 else ''
            (root / 'run.sh').write_text(
                '#!/bin/bash\nset -eu\n'
                'cd -- "$(dirname -- "$0")"\n'
                'echo "0 0" > logs/progress\n'
                'echo Floopy > logs/echo.log\n'
                'echo "1 0" > logs/progress\n'
                f'{before_exit}'
                f'exit {exit_code}\n')
            (root / 'run.sh').chmod(0o755)
            try:
                self.assertTrue(queue.schedule(job), 'Queue rejected the job')
                # Scheduling only confirms acceptance. QProcess notifications and
                # progress timers need Qt events to advance the job to completion.
                # Use a monotonic deadline so clock adjustments cannot extend it.
                deadline = time.monotonic() + 30
                while job.state() not in terminal and time.monotonic() < deadline:
                    self.process_events()
                    time.sleep(0.01)
                self.assertEqual(job.state(), smtk.job.State.Completed,
                                 f'Job did not complete: state={job.state()}, status={job.status()}')
                # Completed includes unsuccessful exits; check status separately.
                expected_status = (smtk.job.Status.Succeeded if exit_code == 0
                                   else smtk.job.Status.Failed)
                self.assertEqual(job.status(), expected_status)
                self.assertTrue(job.queueId(), 'Job never received a queue ID')
                self.assertEqual(
                    (root / 'logs/echo.log').read_text().strip(), 'Floopy')
                self.assertEqual(
                    (root / 'logs/progress').read_text().strip(), '1 0')
            finally:
                # Stop an unfinished job before deleting the files it is using.
                if job.queueId() and job.state() not in terminal:
                    self.assertTrue(queue.cancel(
                        job), 'Could not cancel unfinished test job')
                    deadline = time.monotonic() + 5
                    while job.state() not in terminal and time.monotonic() < deadline:
                        self.process_events()
                        time.sleep(0.01)
                    self.assertIn(job.state(), terminal,
                                  'Test job did not stop after cancellation')
        finally:
            # Preserve inputs and logs if cancellation failed: a live process may
            # still need them, and they help diagnose the failure.
            if job is None or not job.queueId() or job.state() in terminal:
                shutil.rmtree(root)
            else:
                print(f'Preserving {root}: the test job may still be running')

    @unittest.skipUnless(shutil.which('bash'), 'ShellQueue requires bash on PATH')
    def test_shell_queue_job(self):
        self.run_job()

    @unittest.skipUnless(shutil.which('bash'), 'ShellQueue requires bash on PATH')
    def test_shell_queue_failure(self):
        # Writing the expected output must not hide a nonzero process exit code.
        self.run_job(exit_code=7)

    # Skip only this case when Podman is absent, leaving shell coverage active.
    # Once Podman is found, setup, image-pull, and execution errors must fail.
    @unittest.skipIf(os.name == 'nt', 'ContainerQueue is not supported on Windows')
    @unittest.skipUnless(shutil.which('podman'), 'ContainerQueue requires podman on PATH')
    def test_container_queue_job(self):
        self.run_job(container=True)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
