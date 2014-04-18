"""
Python script to list contents of template file as indented list.

At some point, might get expanded to also list out attributes.


For future reference, to run from template directory
  LD_LIBRARY_PATH=/media/ssd/sim/cmb_core/build/CMBSuperBuild/install/lib \
  /media/ssd/sim/cmb_core/build/CMBSuperBuild/install/bin/pvpython \
  /media/ssd/sim/cmb_core/git/SMTK/Utilities/python/listdefs.py \
  -t GEOTACS_Template_3.0.sbt
"""


app_description = 'Python script to list definitions'

import argparse
import sys

try:
    import smtk
except ImportError:
    print
    print app_description
    print 'Not able to import smtk library. You might need to:'
    print '  - Use the PYTHONPATH variable to point to the smtk python lib'
    print '  - And/or use the LD_LIBRARY_PATH variable to point to the shiboken libraries'
    print
    sys.exit(-1)


def get_base_definitions(defn, def_list):
  '''Recursively builds list of base definitions

  '''
  base_def = defn.baseDefinition()
  if base_def is None:
    return

  def_list.append(base_def.type())
  return get_base_definitions(base_def, def_list)



def list_definitions(manager, options={}):
  '''Lists definitions in indented-text format

  '''
  base_list = manager.findBaseDefinitions()
  #print base_list

  def_list = list()
  for defn in base_list:
    #print defn.type()
    derived_list = manager.findAllDerivedDefinitions(defn, True)
    #print 'derived_list', derived_list
    #print '%s count %d' % (defn.type(), len(derived_list))
    def_list.extend(derived_list)

  #print def_list
  def_list.sort(key=lambda d: d.type())

  for defn in def_list:
    base_list = list()
    get_base_definitions(defn, base_list)
    if base_list:
      base_string = ' : '.join(base_list)
      print '%s : %s' % (defn.type(), base_string)
    else:
      print defn.type()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=app_description)
    parser.add_argument('-t', '--template_filename', required=True)
    args = parser.parse_args()

    #  Load template file
    logger = smtk.util.Logger()
    print 'Loading template file %s' % args.template_filename
    manager = smtk.attribute.Manager()
    reader = smtk.util.AttributeReader()
    err = reader.read(manager, args.template_filename, logger)
    if err:
        print 'Abort: Could not load template file'
        print logger.convertToString()
        sys.exit(-2)

    # List the output
    list_definitions(manager)
