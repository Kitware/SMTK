"""
Python script to list contents of template file as indented list.

At some point, might get expanded to also list out attributes.


For future reference, to run from template directory
  LD_LIBRARY_PATH=/media/ssd/sim/cmb_core/build/CMBSuperBuild/install/lib \
  /media/ssd/sim/cmb_core/build/CMBSuperBuild/install/bin/pvpython \
  /media/ssd/sim/cmb_core/git/SMTK/Utilities/python/listdefs.py \
  -t GEOTACS_Template_3.0.sbt
"""


# Global vars
app_description = 'Python script to list definitions'
default_spaces_per_indent = 2

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


# ---------------------------------------------------------------------------
def get_base_definitions(defn, def_list):
  '''Recursively builds list of base definitions

  '''
  base_def = defn.baseDefinition()
  if base_def is None:
    return

  def_list.append(base_def.type())
  return get_base_definitions(base_def, def_list)


# ---------------------------------------------------------------------------
def list_items(parent, level, options):
  ''' Lists items contained by parent

  The level input sets the indentation
  Conditional children are prefixed with "*"
  '''
  this_indent = options.indent * level
  if hasattr(parent, 'itemDefinition'):
    # Parent is Definition or GroupItemDefinition
    n = parent.numberOfItemDefinitions()
    for i in range(n):
      item = parent.itemDefinition(i)
      concrete_item = smtk.to_concrete(item)
      type_string = smtk.attribute.Item.type2String(item.type())
      print '%s%s \"%s\"' % (this_indent, type_string, item.name())
      list_items(concrete_item, level+1, options)
  elif hasattr(parent, 'childrenItemDefinitions'):
    # Parent is ValueItemDefinition
    item_dict = parent.childrenItemDefinitions()
    item_list = item_dict.items()
    if item_list is None:
      return

    item_list.sort()
    for name, item in item_list:
      concrete_item = smtk.to_concrete(item)
      type_string = smtk.attribute.Item.type2String(item.type())
      print '%s*%s \"%s\"' % (this_indent, type_string, name)
      list_items(concrete_item, level+1, options)


# ---------------------------------------------------------------------------
def list_definitions(manager, options):
  '''Lists definitions in indented-text format

  '''
  base_list = manager.findBaseDefinitions()
  #print base_list

  def_list = list()
  for defn in base_list:
    derived_list = manager.findAllDerivedDefinitions(defn, True)
    def_list.extend(derived_list)

  def_list.sort(key=lambda d: d.type())
  for defn in def_list:
    base_list = list()
    get_base_definitions(defn, base_list)
    if base_list:
      base_string = ' : '.join(base_list)
      print '%s : %s' % (defn.type(), base_string)
    else:
      print defn.type()

    list_items(defn, 1, options)


# ---------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=app_description)
    parser.add_argument('-t', '--template_filename', required=True)
    parser.add_argument('-s', '--spaces_per_indent', type=int, \
      default=default_spaces_per_indent)
    args = parser.parse_args()

    # Construct options object
    options = type('Options', (object,), {})
    options.indent = ' ' * args.spaces_per_indent

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
    list_definitions(manager, options)
