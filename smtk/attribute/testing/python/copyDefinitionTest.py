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
"""
Test smtk.attribute.Resource.copyDefinition() method

Uses copyDefinitionTest.sbt in the SMTKTestData repo.
"""

import logging
import os
import sys

try:
    import smtk
    from smtk import attribute
    from smtk import io
except ImportError:
    print()
    print('Not able to import smtk library. You might need to:')
    print('  - Use the PYTHONPATH variable to point to the smtk python lib')
    print()
    sys.exit(-1)


logging.basicConfig(level=logging.DEBUG)

SBT_FILENAME = 'copyDefinitionTest.sbt'
SBI_FILENAME = 'copyDefinitionTest.sbi'

if __name__ == '__main__':
    # First (and) only argument is the path to the smtk data directory
    if len(sys.argv) < 2:
        print()
        print('Test smtk.attribute.Resource.copyDefinition()')
        print('Usage: python %s path-to-SMTKTestData')
        print()
        sys.exit(-1)

    logging.debug('LD_LIBRARY_PATH = %s' % os.environ.get('LD_LIBRARY_PATH'))
    logging.debug('PYTHONPATH = %s' % os.environ.get('PYTHONPATH'))

    # Load attribute file into resource
    smtk_test_data = sys.argv[1]
    att_folder = os.path.join(
        smtk_test_data, 'attribute', 'attribute_collection')
    att_path = os.path.join(att_folder, SBT_FILENAME)
    logging.info('Reading %s' % att_path)
    input_resource = smtk.attribute.Resource.create()
    reader = smtk.io.AttributeReader()
    logger = smtk.io.Logger()
    err = reader.read(input_resource, att_path, logger)
    if err:
        logging.error("Unable to load template file")
        logging.error(logger.convertToString())
        sys.exit(-2)

    err_count = 0

    # Instantiate 2nd resource
    test_resource = smtk.attribute.Resource.create()

    # Copy SecondConcrete definition, which should copy alot of stuff
    source_def = input_resource.findDefinition('SecondConcrete')
    test_resource.copyDefinition(source_def, 0)
    expected_types = [
        'SecondConcrete', 'AnotherAbstractBase', 'CommonBase',
        'PolyLinearFunction'
    ]
    for def_type in expected_types:
        defn = test_resource.findDefinition(def_type)
        if defn is None:
            logging.error('Expected %s definition, found None' % def_type)
            err_count += 1

    not_expected_types = [
        'FirstConcrete'
    ]
    for def_type in not_expected_types:
        defn = test_resource.findDefinition(def_type)
        if defn is not None:
            logging.error('Not expected %s definition found' % def_type)
            err_count += 1

    # Add explicit test for conditional children
    defn = test_resource.findDefinition('SecondConcrete')
    assert (defn is not None)

    # First selection list
    i = defn.findItemPosition('SelectionList')
    assert (i >= 0)
    select_item_def = defn.itemDefinition(i)
    assert (select_item_def.name() == 'SelectionList')
    assert (select_item_def.isDiscrete())
    assert (not select_item_def.hasDefault())

    # Second selection list (with conditional children)
    i = defn.findItemPosition('ConditionalSelectionList')
    assert (i >= 0)
    cond_item_def = defn.itemDefinition(i)
    assert (cond_item_def is not None)
    assert (cond_item_def.hasDefault())

    list_one = cond_item_def.conditionalItems('One')
    assert (len(list_one) == 1)

    list_two = cond_item_def.conditionalItems('Two')
    assert (len(list_two) == 3)

    # Note there is ALOT more that could & should be verified here
    logging.debug('Writing resource')

    # Write data out FYI
    writer = smtk.io.AttributeWriter()
    err = writer.write(test_resource, SBI_FILENAME, logger)
    if err:
        logging.error("Unable to write output file")
        sys.exit(-3)
    logging.info('Wrote %s' % SBI_FILENAME)

    # Check error count
    if err_count > 0:
        sys.exit(err_count)

    sys.exit(0)
