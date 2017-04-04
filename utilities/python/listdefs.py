"""
Python script to list contents of template file as indented list.

At some point, might get expanded to also list out attributes.


For future reference, to run from template directory
  LD_LIBRARY_PATH=/media/ssd/sim/cmb_core/build/CMBSuperBuild/install/lib \
  /media/ssd/sim/cmb_core/build/CMBSuperBuild/install/bin/pvpython \
  /media/ssd/sim/cmb_core/git/SMTK/utilities/python/listdefs.py \
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


def get_base_definitions(defn, def_list):
    '''Recursively builds list of base definitions

    '''
    base_def = defn.baseDefinition()
    if base_def is None:
        return

    quoted = '\"%s\"' % base_def.type()
    def_list.append(quoted)
    return get_base_definitions(base_def, def_list)


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
            list_items(concrete_item, level + 1, options)
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
            list_items(concrete_item, level + 1, options)


def list_definition(system, defn, level, options):
    '''Lists one definition in indented-text format

    '''
    this_indent = options.indent * level
    sublevel = level + 1
    base_list = list()
    get_base_definitions(defn, base_list)

    if base_list:
        base_string = ' : '.join(base_list)
        print '%sAttDef \"%s\" : %s' % (this_indent, defn.type(), base_string)
    else:
        print '%sAttDef \"%s\"' % (this_indent, defn.type())

    list_items(defn, sublevel, options)


def list_by_view(system, view, level, options):
    '''Lists definitions in indented-text format, organized by view

    Works recursively
    '''
    this_indent = options.indent * level
    sublevel = level + 1

    print '%sVIEW \"%s\"' % (this_indent, view.title())

    # Group view
    if hasattr(view, 'numberOfSubViews'):
        n = view.numberOfSubViews()
        for i in range(n):
            subview = view.subView(i)
            concrete_subview = smtk.to_concrete(subview)
            list_by_view(system, concrete_subview, sublevel, options)

    # Attribute view
    elif hasattr(view, 'numberOfDefinitions'):
        n = view.numberOfDefinitions()
        for i in range(n):
            defn = view.definition(i)
            list_definition(system, defn, sublevel, options)

    # Instanced view
    elif hasattr(view, 'numberOfInstances'):
        n = view.numberOfInstances()
        for i in range(n):
            att = view.instance(i)
            defn = att.definition()
            list_definition(system, defn, sublevel, options)


def list_definitions(system, options):
    '''Lists all definitions in indented-text format

    '''
    base_list = system.findBaseDefinitions()
    # print base_list

    def_list = list()
    for defn in base_list:
        derived_list = system.findAllDerivedDefinitions(defn, True)
        def_list.extend(derived_list)

    def_list.sort(key=lambda d: d.type())
    for defn in def_list:
        """
        base_list = list()
        get_base_definitions(defn, base_list)
        if base_list:
          base_string = ' : '.join(base_list)
          print '%s : %s' % (defn.type(), base_string)
        else:
          print defn.type()

        list_items(defn, 1, options)
        """
        list_definition(system, defn, 1, options)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=app_description)
    parser.add_argument('-t', '--template_filename', required=True)
    parser.add_argument('-s', '--spaces_per_indent', type=int,
                        default=default_spaces_per_indent)
    parser.add_argument('-v', '--list_by_view', action='store_true')
    args = parser.parse_args()

    # Construct options object
    options = type('Options', (object,), {})
    options.indent = ' ' * args.spaces_per_indent
    options.list_by_view = args.list_by_view

    #  Load template file
    logger = smtk.util.Logger()
    print 'Loading template file %s' % args.template_filename
    system = smtk.attribute.System()
    reader = smtk.util.AttributeReader()
    err = reader.read(system, args.template_filename, logger)
    if err:
        print 'Abort: Could not load template file'
        print logger.convertToString()
        sys.exit(-2)

    # List the output
    if options.list_by_view:
        root_view = system.rootView()
        list_by_view(system, root_view, 0, options)
    else:
        list_definitions(system, options)
