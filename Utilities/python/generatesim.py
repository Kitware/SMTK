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


def generate_atts(manager, att_description, model):
	'''
	Constructs attributes based on input description
	Returns number of attributes that got created
	'''
	count = 0
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

	att_description = description.get('attributes')
	if att_description is None:
		print 'Warning: no attributes found in input description'
	else:
		count = generate_atts(manager, description, model)
		print 'Number of attributes created: %d' % count
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
    print 'Number of attributes created: %d' % count

    # TODO
    # Write output
