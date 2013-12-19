"""
Python script to generate simulation attributes from input (text) description
"""

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
    print 'Not able to import smtk library. You might need to:'
    print '  - Use the PYTHONPATH variable to point to the smtk python lib'
    print '  - And/or use the LD_LIBRARY_PATH variable to point to the shiboken libraries'
    print
    sys.exit(-1)


def generate_model(model_description):
    '''
    Constructs model based on input description
    '''
    #print 'Generating model'
    model = smtk.model.Model()
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


def set_item(item, item_description):
    '''
    '''
    enabled = item_description.get('enabled')
    if enabled is not None:
        item.setIsEnabled(enabled)

    discrete_index = item_description.get('discrete_index')
    if discrete_index is not None:
        item.setDiscreteIndex(discrete_index)

    value = item_description.get('value')
    if value is not None:
        # TODO Handle multiple values
        item.setValue(0, value)




def generate_atts(manager, attributes_description, model):
    '''
    Constructs attributes based on input description
    Returns number of attributes that got created
    '''
    count = 0
    for att_description in attributes_description:
        att_type = att_description.get('att')
        name = att_description.get('name')
        att_id = att_description.get('id')

        if (att_type is None) or (name is None) or (att_id is None):
            print 'Warning: attribute description incomplete' + \
                ' - type %s, name %s, id %s ' % (att_type, name, id) + \
                ' - skipping'
            continue
        defn = manager.findDefinition(att_type)
        if defn is None:
            print 'Warning: no attribute definition for %s type' % \
                att_type + ' - skipping'
            continue

        # Attribute may have been automatically instanced
        att = manager.findAttribute(name)
        if att is None:
            att = manager.createAttribute(name, att_type, att_id)
        if att is None:
            print 'Warning: Manager did not create attribute of type %s -skipping' % \
                att_type
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
                    print 'Associate model item %d to att %s' % (model_item_id, name)
                    att.associateEntity(model_item)

        count += 1

        item_list = att_description.get('items', list())
        for item_description in item_list:
            item_name = item_description.get('name')
            item = att.find(item_name)
            if item is None:
                print 'Warning: no item %s for attribute %s - skipping' % \
                    (item_name, name)
                manager.removeAttribute(name)
                count -= 1
                break

            concrete_item = smtk.attribute.to_concrete(item)
            set_item(concrete_item, item_description)


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
        model = generate_model(model_description)
        manager.setRefModel(model)

    att_description = description.get('attributes')
    if att_description is None:
        print 'Warning: no attributes found in input description'
    else:
        count = generate_atts(manager, att_description, model)
    return count


if __name__ == '__main__':
    description = 'Python script to generate simulation attributes' + \
        ' from input (text) descriptions'
    epilog = 'Note: you must specify EITHER --yaml_filename OR --json_filename'
    parser = argparse.ArgumentParser(description=description, epilog=epilog)
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
        print 'Either install PyYaml or swith to json input file.'
        print
        sys.exit(-2)


    #  Load template file
    logger = smtk.util.Logger()
    print 'Loading template file %s' % args.template_filename
    manager = smtk.attribute.Manager()
    reader = smtk.util.AttributeReader()
    err = reader.read(manager, args.template_filename, logger)
    if err:
        print 'Abort: Could not load attribute file'
        sys.exit(-3)

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
            sim_description = yaml.load(contents)
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