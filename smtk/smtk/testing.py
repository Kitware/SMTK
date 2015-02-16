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

DATA_DIR=''

def process_arguments():
  """Process common options to python tests.

  This module parses command line arguments and sets module
  variable values based on them. It then removes the options
  from sys.argv so that the unittest framework will not treat
  them as module names.
  """

  global DATA_DIR

  from argparse import ArgumentParser
  parser = ArgumentParser()
  parser.add_argument("-D", "--data-dir",
    action="store", dest="datadir", default='',
    help="Top-level testing data directory.")

  args = parser.parse_args()

  if args.datadir:
    DATA_DIR=args.datadir

  sys.argv = sys.argv[:1]
