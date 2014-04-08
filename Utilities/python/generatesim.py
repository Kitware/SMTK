"""
Python script to generate simulation attributes from input (text) description
"""

app_description = 'Python script to generate simulation attributes' + \
    ' from input (text) descriptions'


import argparse
import json
import sys

YAML_ENABLED = False
try:
    import yaml
    YAML_ENABLED = True
except ImportError:
    pass

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


def generate_model_items(manager, model_description):
    '''
    Constructs model based on input description
    '''
    print 'Generating model items'
    model = manager.refModel()
    for item_description in model_description:
        group = item_description.get('group')
        name = item_description.get('name')
        mask = item_description.get('mask')
        if (group is None) or (name is None) or (mask is None):
            print 'Warning: model item description incomplete' + \
                ' - group %s, name %s, mask %s ' % (group, name, mask) + \
                ' - skipping'
            continue
        new_group = model.createModelGroup(name, group, mask)

    return model


def set_item_value(item, value, index=0):
    '''
    Sets value, casting to type as needed
    '''
    methods = {
        smtk.attribute.Item.DOUBLE: float,
        smtk.attribute.Item.INT: int,
    }
    cast_method = methods.get(item.type())
    if cast_method is not None:
        value = cast_method(value)

    print 'Setting value to', value, type(value)
    item.setValue(index, value)


def set_item(item, item_description, refitem_list):
    '''
    Recursive method to set contents of Item instances
    Returns boolean indicating success
    '''
    enabled = item_description.get('enabled')
    if enabled is not None:
        item.setIsEnabled(enabled)

    if item.type() == smtk.attribute.Item.GROUP:
        success = process_items(item, item_description, refitem_list)
        return success

    if item.type() == smtk.attribute.Item.ATTRIBUTE_REF:
        # RefItem instances get set after all attributes have been created
        refitem_list.append((item, item_description))
        print 'refitem_list', refitem_list
        return True

    expression = item_description.get('expression')
    if expression is not None:
        exp = manager.findAttribute(expression)
        if exp is None:
            print 'Could not find expression named %s' % expression
            return False

        print 'Setting expression to %s' % expression
        success = item.setExpression(exp)
        return success

    discrete_index = item_description.get('discrete_index')
    if discrete_index is not None:
        print 'Setting discrete index to %d' % discrete_index
        success = item.setDiscreteIndex(discrete_index)
        return success

    value = item_description.get('value')
    if value is not None:
        if isinstance(value, (list, tuple)):
            num_values = len(value)
            print 'num_values', num_values
            item.setNumberOfValues(num_values)
            for i in range(num_values):
                set_item_value(item, value[i], i)
        else:
            item.setNumberOfValues(1)
            set_item_value(item, value)


def process_items(parent, parent_description, refitem_list):
    '''
    Traverses all items contained by parent
    Note that parent may be either Attribute or GroupItem
    Returns boolean indicating success
    '''
    success = True

    debug_flag = False
    if hasattr(parent, 'type') and parent.type() == 'NaturalTransport':
        debug_flag = True
        print 'Set debug_flag', debug_flag

    item_list = parent_description.get('items', list())
    for item_description in item_list:
        item_name = item_description.get('name')
        if debug_flag:
            print 'debug item name', item_name
        item = parent.find(item_name)
        if item is None:
            print 'Warning: no item %s for parent %s - skipping' % \
                (item_name, parent.name())
            if isinstance(parent, smtk.attribute.Attribute):
                manager.removeAttribute(name)
                count -= 1
                success = False
            break

        print 'Set item %s' % item.name()
        concrete_item = smtk.attribute.to_concrete(item)
        set_item(concrete_item, item_description, refitem_list)
    return success


def fetch_attribute(manager, att_type, name, att_id):
    '''
    Retrieves or creates attribute as needed
    '''
    att = None   # return value

    # First check that Definition exists
    defn = manager.findDefinition(att_type)
    if defn is None:
        print 'Warning: no attribute definition for %s type' % \
            att_type + ' - skipping'
        return None

    if name is not None:
        att = manager.findAttribute(name)
    else:
        # Check for single attribute instance
        att_list = manager.findAttributes(att_type)
        if len(att_list) == 1:
            #print 'Found single attribute type \"%s\"' % att_type
            att = att_list[0]

    if att is None:
        print 'Creating %s attribute' % att_type
        if att_id is not None:
            # First check that id not already in use
            test_id_att = manager.findAttribute(att_id)
            if test_id_att is not None:
                print 'Cannot generate attribute %s with id %d' % \
                    (name, att_id) + \
                    ' because id is already in use.'
                if name is None:
                    att = manager.createAttribute(att_type)
                else:
                    att = manager.createAttribute(name, att_type)
            else:
                att = manager.createAttribute(name, att_type, att_id)
        elif name is None:
            att = manager.createAttribute(att_type)
        else:
            att = manager.createAttribute(name, att_type)

        if att is None:
            print 'Warning: Manager did not create attribute of type %s -skipping' % \
                att_type
    else:
        print 'Found attribute %s' % name
        if att_id is not None and att_id != att.id():
            print 'Warning: attribute id (%d) does not match input %d' % \
                (att.id(), att_id)
    return att


def generate_atts(manager, attributes_description, refitem_list):
    '''
    Constructs attributes based on input description
    Returns number of attributes that got created
    '''
    print 'Generating attributes'
    count = 0
    model = manager.refModel()
    for att_description in attributes_description:
        att_type = att_description.get('att')
        name = att_description.get('name')
        att_id = att_description.get('id')

        """
        if (att_type is None) or (name is None):
            print 'Warning: attribute description incomplete' + \
                ' - type %s, name %s, id %s ' % (att_type, name, att_id) + \
                ' - skipping'
            continue
        """

        # Attribute may have been automatically instanced
        att = fetch_attribute(manager, att_type, name, att_id)
        if att is None:
            continue

        # Only support 1 associated model entity
        if model is not None:
            model_item_id = att_description.get('model_item')
            if model_item_id is not None:
                model_item = model.getModelItem(model_item_id)
                if model_item is None:
                    print 'Warning: Did not find model item %d for attribute type %s - skipping' % \
                        (model_item_id, name)
                else:
                    print 'Associate model item %d to att %s' % (model_item_id, att.name())
                    att.associateEntity(model_item)

        count += 1
        process_items(att, att_description, refitem_list)

    return count


def generate_sim(manager, description):
    '''
    Generates smtk attribute manager
    Returns number of attributes that got created
    '''
    count = 0
    model = None
    model_description = description.get('model')
    if model_description is not None:
        generate_model_items(manager, model_description)

    att_description = description.get('attributes')
    if att_description is None:
        print 'Warning: no attributes found in input description'
    else:
        refitem_list = list()
        count = generate_atts(manager, att_description, refitem_list)

        # Process RefItem instances after all attributes created:
        #print 'refitem_list', refitem_list
        for item, description in refitem_list:
            attname = description.get('attributeref')
            if attname is None:
                print 'Warning: no attributeref specified for', item.name()
                continue

            att = manager.findAttribute(attname)
            refitem = smtk.attribute.to_concrete(item)
            print 'Setting RefItem %s to %s' % (refitem.name(), attname)
            refitem.setValue(att)

    return count


if __name__ == '__main__':
    epilog = 'Note: you must specify EITHER --yaml_filename OR --json_filename'
    parser = argparse.ArgumentParser(description=app_description, epilog=epilog)
    parser.add_argument('-t', '--template_filename', required=True)
    parser.add_argument('-s', '--sim_filename')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-y', '--yaml_filename', help='Input description file')
    group.add_argument('-j', '--json_filename', help='Input description file')
    args = parser.parse_args()

    # Check that yaml module loaded if input has yaml file
    if args.yaml_filename is not None and not YAML_ENABLED:
        print
        print 'Sorry, cannot run because python yaml module was not found.'
        print 'Either install PyYaml or set PYTHONPATH to include PyYaml.'
        print 'Note that PyYaml is available in the SMTK ThirdParty folder.'
        print 'e.g. \"export PYTHONPATH=/path-to-SMTK/ThirdParty/PyYaml\"'
        print
        sys.exit(-2)


    #  Load template file
    logger = smtk.util.Logger()
    print 'Loading template file %s' % args.template_filename
    manager = smtk.attribute.Manager()
    reader = smtk.util.AttributeReader()
    err = reader.read(manager, args.template_filename, logger)
    if err:
        print 'Abort: Could not load template file'
        print logger.convertToString()
        sys.exit(-3)
    model = smtk.model.Model.New()
    manager.setRefModel(model)

    # Load json input
    sim_description = None
    if args.json_filename is not None:
        print 'Loading description file %s' % args.json_filename
        done = False
        with open(json_filename, 'r') as json_file:
            contents = json_file.read()
            done = True
        if not done:
            print 'Abort: Unable to load json description file'
            sys.exit(-4)
        try:
            sim_description = json.loads(contents)
        except Exception:
            print 'Abort: Unable to parse json description file.'
            sys.exit(-4)
    elif args.yaml_filename is not None:
        print 'Loading description file', args.yaml_filename
        done = False
        with open(args.yaml_filename, 'r') as yaml_file:
            contents = yaml_file.read()
            if contents.find('\t') >= 0:
                print 'WARNING: yaml file contains tab char - not valid'
                print 'Will replace with 4 spaces'
                contents = contents.replace('\t', '    ')
            done = True
        if not done:
            print 'Abort: Unable to load yaml description file'
            sys.exit(-4)
        try:
            sim_description = yaml.safe_load(contents)
        except:
            print 'Abort: Unable to parse yaml description file'
            sys.exit(-4)

    # Generate the attributes
    count = generate_sim(manager, sim_description)
    print 'Number of attributes created or updated: %d' % count

    # Write output
    if args.sim_filename is None:
        print '(No output file specified)'
    else:
        print 'Writing output file %s' % args.sim_filename
        writer = smtk.util.AttributeWriter()
        logger.reset()
        err = writer.write(manager, args.sim_filename, logger)
        if err:
            print 'Error writing output file'

    print 'Done'
