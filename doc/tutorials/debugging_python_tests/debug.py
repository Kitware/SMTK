"""
Verify that our documentation on running unit tests in the interpreter is correct.
"""
import smtk.testing


class DebugAPythonTest(smtk.testing.TestCase):

    def testDebugReadFile(self):
        import smtk.testing
        srcdir = smtk.testing.SOURCE_DIR
        # ++ 1 ++
        import smtk
        import smtk.testing
        import os
        import sys

        # Make sure the test is in our Python path:
        # srcdir = '/src'
        sys.path.append(
            os.path.join(
                srcdir, 'smtk', 'bridge', 'discrete', 'testing', 'python'))
        # Import the test case class:
        from discreteReadFile import TestDiscreteSession

        # Prepare smtk.testing as needed:
        # smtk.testing.DATA_DIR='/path/to/data'
        # -- 1 --
        # ++ 2 ++
        # Pass the name ('testRead') of the test you want to run:
        x = TestDiscreteSession('testRead')
        x.setUp()
        # We can now inspect test members created by the setup:
        print [y.name() for y in x.mgr.sessions()]
        # ... or run the test:
        x.testRead()
        # -- 2 --


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
