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
import os
import sys

BASELINES = []
DATA_DIR = ''
TEMP_DIR = '.'
SOURCE_DIR = ''
WORKER_DIR = ''
INTERACTIVE = False


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


class TestCase:
    __metaclass__ = TestCaseMeta

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
        if smtk.wrappingProtocol() == 'pybind11':
            import smtk.extension.vtk.source
            mbs = smtk.extension.vtk.source.vtkModelMultiBlockSource()
            mbs.SetModelManager(self.mgr)
        else:
            import vtkSMTKSourceExtPython
            mbs = vtkSMTKSourceExtPython.vtkModelMultiBlockSource()
            mbs.SetModelManager(self.mgr.pointerAsString())
        mbs.SetModelEntityID(str(model.entity()))
        # mbs.ShowAnalysisTessellationOff()
        return self.addToScene(mbs)

    def addMeshToScene(self, mesh):
        import smtk
        if smtk.wrappingProtocol() == 'pybind11':
            import vtk
            import smtk.io.vtk
            exprt = smtk.io.vtk.ExportVTKData()
            pd = vtk.vtkPolyData()
            exprt(mesh, pd)
            trivialProducer = vtk.vtkTrivialProducer()
            trivialProducer.SetOutput(pd)
            return self.addToScene(trivialProducer)
        else:
            print "Shiboken does not support vtk/smtk compatibility layer"

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
