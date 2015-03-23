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
import sys

BASELINES=[]
DATA_DIR=''
TEMP_DIR='.'
SOURCE_DIR=''
WORKER_DIR=''

def process_arguments():
    """Process common options to python tests.

    This module parses command line arguments and sets module
    variable values based on them. It then removes the options
    from sys.argv so that the unittest framework will not treat
    them as module names.
    """

    global BASELINES, DATA_DIR, TEMP_DIR, WORKER_DIR, SOURCE_DIR

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

    args = parser.parse_args()

    if args.datadir:
      DATA_DIR=args.datadir

    if args.workerdir:
      WORKER_DIR=args.workerdir

    if args.srcdir:
      SOURCE_DIR=args.srcdir

    if args.tempdir:
      TEMP_DIR=args.tempdir

    if args.validresult:
      BASELINES.append(args.validresult)

    sys.argv = sys.argv[:1]
