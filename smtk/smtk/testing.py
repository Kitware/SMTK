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
from __future__ import absolute_import, division, print_function

import os
import sys

BASELINES = []
DATA_DIR = ''
TEMP_DIR = '.'
SOURCE_DIR = ''
WORKER_DIR = ''
INTERACTIVE = False
# If the return code for the entire process is SKIP_ENTIRE,
# then CTest will show the test as NotRun rather than succeeded
# but with some/all tests skipped. This number must match the
# SKIP_RETURN_CODE value specified in the smtk_add_test_python
# cmake macro (in the top-level CMakeLists.txt).
SKIP_ENTIRE = 42


def find_data(path):
    """Find the full path to a test-data file.
    """
    global DATA_DIR

    p = path
    if type(p) != str:
        p = os.path.join(*path)

    fname = os.path.join(*[DATA_DIR, p])
    if os.path.isfile(fname):
        return fname

    # Found no matches, return the joined string.
    return p


def run_interactive():
    "Should the test run interactively or in batch mode?"
    return INTERACTIVE


def process_arguments():
    """Process common options to python tests.

    This module parses command line arguments and sets module
    variable values based on them. It then removes the options
    from sys.argv so that the unittest framework will not treat
    them as module names.
    """

    global BASELINES, DATA_DIR, TEMP_DIR, WORKER_DIR, SOURCE_DIR, INTERACTIVE

    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("-D", "--data-dir",
                        action="store", dest="datadir", default='',
                        help="Top-level testing data directory.")

    parser.add_argument("-W", "--worker-dir",
                        action="store", dest="workerdir", default='',
                        help="Directory containing SMTK's Remus worker files.")

    parser.add_argument("-S", "--src-dir",
                        action="store", dest="srcdir", default='',
                        help="Directory containing the SMTK source code.")

    parser.add_argument("-T", "--temp-dir",
                        action="store", dest="tempdir", default='',
                        help="Directory where test files may be written.")

    parser.add_argument("-V", "--valid-result",
                        action="store", dest="validresult", default='',
                        help="Path to a valid result (baseline) for comparison.")

    parser.add_argument("-I", "--interactive",
                        action="store_true", dest="interactive",
                        help="Run interactively rather than exiting immediately.")

    args = parser.parse_args()

    if args.datadir:
        DATA_DIR = args.datadir

    if args.workerdir:
        WORKER_DIR = args.workerdir

    if args.srcdir:
        SOURCE_DIR = args.srcdir

    if args.tempdir:
        TEMP_DIR = args.tempdir

    if args.validresult:
        BASELINES.append(find_data(args.validresult))

    if args.interactive:
        INTERACTIVE = True

    sys.argv = sys.argv[:1]


def compare_image(render_window, baseline_path):
    try:
        import vtk.test.Testing
        vtk.test.Testing.compareImage(
            render_window, find_data(baseline_path))
    except RuntimeError as e:
        raise AssertionError(*e.args)


class TestCaseMeta(type):

    """A metaclass for tests.

    This is used to make TestCase inherit vtk.test.Testing.vtkTest
    (which is derived from unittest.TestCase) when VTK is available
    and unittest.TestCase when not.
    """
    def __new__(cls, name, bases, attrs):
        try:
            import vtk.test.Testing
            bases += (vtk.test.Testing.vtkTest,)
        except ImportError:
            try:
                import unittest
                bases += (unittest.TestCase,)
            except ImportError:
                pass
        return super(TestCaseMeta, cls).__new__(cls, name, bases, attrs)


def with_metaclass(meta, *bases):
    """Shamelessly stolen from six.
       See https://github.com/benjaminp/six for this function's license.
    """
    class metaclass(type):

        def __new__(cls, name, this_bases, d):
            return meta(name, bases, d)

        @classmethod
        def __prepare__(cls, name, this_bases):
            return meta.__prepare__(name, bases)

    return type.__new__(metaclass, 'temporary_class', (), {})


class TestCase(with_metaclass(TestCaseMeta)):

    """A base class for SMTK unit tests.

    This class extends either vtk.test.Testing.vtkTest (when VTK
    is available) or unittest.TestCase (otherwise). If VTK is
    available, then image generation and comparison functions
    will be available to tests.
    """

    def haveVTK(self):
        """Return True if VTK can be imported.

        NB. This does not indicate whether VTK support has been enabled in SMTK.
        """
        try:
            import vtk
            import vtk.test
            return True
        except:
            pass
        return False

    def haveVTKExtension(self):
        """Return True if SMTK was built with the VTK extension enabled.
        """
        try:
            import vtkSMTKSourceExtPython
            return True
        except:
            pass
        return False

    def startRenderTest(self):
        try:
            import vtk
            self.renderWindow = vtk.vtkRenderWindow()
            self.renderer = vtk.vtkRenderer()
            self.interactor = vtk.vtkRenderWindowInteractor()
            self.renderWindow.AddRenderer(self.renderer)
            self.renderWindow.SetInteractor(self.interactor)
            self.renderWindow.SetMultiSamples(0)
        except ImportError:
            self.skipTest('VTK is not available')

    def addToScene(self, msource, **kwargs):
        import vtk
        vsource = msource
        if 'translate' in kwargs:
            tf = vtk.vtkTransformFilter()
            tf.SetTransform(vtk.vtkTransform())
            delta = kwargs['translate']
            tf.GetTransform().Translate(delta[0], delta[1], delta[2])
            tf.SetInputConnection(msource.GetOutputPort())
            vsource = tf
        ac = vtk.vtkActor()
        mp = vtk.vtkCompositePolyDataMapper()
        ac.SetMapper(mp)
        mp.SetInputConnection(vsource.GetOutputPort())
        self.renderer.AddActor(ac)
        return [msource, vsource, mp, ac]

    def addModelToScene(self, model):
        import smtk
        import smtk.extension.vtk.source
        mbs = smtk.extension.vtk.source.vtkModelMultiBlockSource()
        mbs.SetModelResource(model.resource())
        # mbs.ShowAnalysisTessellationOff()
        addedToScene = self.addToScene(mbs)
        mp = addedToScene[2]
        mp.SetScalarModeToUseFieldData()
        mp.SelectColorArray('entity color')

        # When coloring by field data, the default action is to assume that the
        # selected field data has as many tuples as cells. In our case, we want
        # to color the entire block by a single tuple (the first tuple).
        mp.SetFieldDataTupleId(0)

        return addedToScene

    def addMeshToScene(self, mesh):
        import smtk
        import vtk
        import smtk.io.vtk
        exprt = smtk.io.vtk.ExportVTKData()
        pd = vtk.vtkPolyData()
        exprt(mesh, pd)
        trivialProducer = vtk.vtkTrivialProducer()
        trivialProducer.SetOutput(pd)
        return self.addToScene(trivialProducer)

    def addImageToScene(self, msource):
        import vtk
        vsource = msource
        self.renderWindow = vtk.vtkImageViewer2()
        self.renderer = self.renderWindow.GetRenderer()
        self.renderWindow.SetInputConnection(vsource.GetOutputPort())
        self.renderWindow.SetupInteractor(self.interactor)

    def interactive(self):
        """Return false if the test should exit at completion."""
        global INTERACTIVE
        return INTERACTIVE

    def interact(self):
        """Run the interactor if the test is marked as interactive.

        For VTK tests, the interactor is a vtkRenderWindowInteractor.
        """
        if self.interactive():
            self.interactor.Start()

    def assertImageMatch(self, baseline_path, threshold=10):
        try:
            import vtk.test.Testing
            vtk.test.Testing.compareImage(
                self.renderWindow, find_data(baseline_path), threshold)
        except ImportError as err:
            self.skipTest('VTK is unavailable')
        except RuntimeError as e:
            # return False
            raise AssertionError(*e.args)
        return True

    def assertImageMatchIfFileExists(self, baseline_path, threshold=10):
        full_baseline = find_data(baseline_path)
        if not os.path.isfile(full_baseline):
            return True
        return self.assertImageMatch(baseline_path, threshold)

    @staticmethod
    def hex2rgb(hexstr):
        hh = hexstr[1:] if hexstr[0] == '#' else hexstr
        rr = int(hh[0:2], 16) / 255.
        gg = int(hh[2:4], 16) / 255.
        bb = int(hh[4:6], 16) / 255.
        return (rr, gg, bb)


def main():
    import unittest
    unittest.main()
