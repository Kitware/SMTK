"""
Script to convert CMB2 template to CMB3 format

This script parses the CMB2 template in python, and creates the
corresponding attdefs in current (CMB3) format.

Note: this code presumes that the input template is valid
"""

import xml.etree.ElementTree as ET
import smtk


def create_item(elem, name, indent=''):
    '''
    Creates new item based on <ValueType>, <DefaultValue>, <Constraint>,
    <Advanced>
    '''
    cast = None
    valuetype_elem = elem.find('ValueType')
    if valuetype_elem is None:
        print 'ERROR - ValueType is None'
        return
    valuetype = valuetype_elem.attrib.get('Name')
    if valuetype == 'double':
        item = smtk.attribute.DoubleItemDefinition.New(name)
        cast = float
    elif valuetype == 'int':
        item = smtk.attribute.IntItemDefinition.New(name)
        cast = int
    elif valuetype == 'text':
        item = smtk.attribute.StringItemDefinition.New(name)
        cast = str
    elif valuetype == 'void':
        item = smtk.attribute.VoidItemDefinition.New(name)
    else:
        print '%sCode for type %s not implemented -- skipping' % (indent, valuetype)
        return None

    # Handle <Advanced>
    advanced_elem = elem.find('Advanced')
    if advanced_elem is not None:
        item.setAdvanceLevel(1)

    # Handle <DefaultValue>
    default_elem = elem.find('DefaultValue')
    if default_elem is not None:
        default_value = default_elem.attrib.get('Value')
        item.setDefaultValue(cast(default_value))

    # Handle <Constraint>
    constraint_elem_list = elem.findall('Constraint')
    for constraint_elem in constraint_elem_list:
        ctype = constraint_elem.attrib.get('Type')
        inclusive = constraint_elem.attrib.get('Inclusive', 'No')
        is_inclusive = True if inclusive == 'Yes' else False
        value = constraint_elem.attrib.get('Value')
        if ctype == 'Minimum':
            item.setMinRange(cast(value), is_inclusive)
        elif ctype == 'Maximum':
            item.setMaxRange(cast(value), is_inclusive)

    return item

def process_multivalue_elem(elem, group_item, indent=''):
    '''
    Processes <MultiValue> elements
    '''
    child_indent = '  ' + indent
    input_elem_list = elem.findall('InputValue')
    for input_elem in input_elem_list:
        name = 'Value%s' % input_elem.attrib.get('Index')
        input_item = create_item(input_elem, name, child_indent)
        if input_item is not None:
            group_item.addItemDefinition(input_item)


def process_information_elem(elem, defn, indent=''):

    '''
    Processes <InformationValue> element
    '''
    child_indent = '  ' + indent
    tagname = elem.attrib.get('TagName')
    uiname = elem.attrib.get('UIName')
    print '%sInfoValue tagname %s, UIName \"%s\"' % (indent, tagname, uiname)

    # Check for group type
    mvalue_elem = elem.find('MultiValue')
    if mvalue_elem is not None:
        print '%sProcessing <MultiValue> element' % indent
        item = smtk.attribute.GroupItemDefinition.New(tagname)
        # TODO SET CATEGORY
        process_multivalue_elem(mvalue_elem, item, child_indent)
        defn.addItemDefinition(item)
        return

    item = create_item(elem, tagname, child_indent)
    if item is None:
        return

    # Set Label
    if uiname is not None:
        item.setLabel(uiname)

    defn.addItemDefinition(item)


def process_instance_elem(manager, elem, indent=''):

    '''
    Process <Instance> element
    '''
    child_indent = '  ' + indent
    tagname = elem.attrib.get('TagName')
    name = elem.attrib.get('Name')
    uiname = elem.attrib.get('UIName')
    print '%sInstance name %s, tagname %s, UIName \"%s\"' % (indent, name, tagname, uiname)

    att_type = tagname
    if att_type is None:
        att_type = name
    print '%sCreate definition %s' % (indent, att_type)
    defn = manager.createDefinition(tagname)
    if defn is None:
        print 'ERROR - manager did not create definition \"%s\"' % tagname
        return

    if uiname is not None:
        defn.setLabel(uiname)

    for child in elem:
        if child.tag == 'InformationValue':
            process_information_elem(child, defn, child_indent)


def process_templates_elem(manager, elem, indent=''):
    '''
    Process <Templates> element
    '''
    child_indent = '  ' + indent
    for child in elem:
        if child.tag == 'Instance':
            process_instance_elem(manager, child, child_indent)


def process_toplevel_elem(manager, elem, indent=''):
    '''
    Process <TopLevel> elements
    '''
    child_indent = '  ' + indent
    skip_types = set(['Functions', 'Domain'])
    section_type = elem.attrib.get('Section')
    if section_type in skip_types:
        print '%s(skipping)' % child_indent
        return

    for child in elem:
        if child.tag == 'Templates':
            process_templates_elem(manager, child, child_indent)
        #if child.tag == 'InformationValue':
        #    process_information_elem(child, defn, child_indent)


def process_toplevel_group(manager, elem, indent=''):
    '''
    Process <TopLevelGroup> element
    '''
    child_indent = '  ' + indent
    for child in elem:
        print indent, child.tag, child.attrib.get('Section')
        #if child.tag == 'TopLevel' and child.attrib.get('Section') == 'Materials':
        process_toplevel_elem(manager, child, child_indent)


if __name__ == '__main__':
    import sys

    # TODO Get args for input & output

    # Instantiate attribute manager for outpot
    manager = smtk.attribute.Manager()

    input_path = '/media/ssd/sim/cmb_core/git/CMB/Source/Applications/ModelBuilder/SimBuilder/XML/GEOTACS_Template.sbt'
    tree = ET.parse(input_path)
    root = tree.getroot()
    print 'root', root.tag
    for child in root:
        print 'elem', child.tag
        if child.tag == 'TopLevelGroup':
            process_toplevel_group(manager, child)

    # Write manager to template file
    output_path = 'output.sbt'
    writer = smtk.util.AttributeWriter()
    logger = smtk.util.Logger()
    hasErr = writer.write(manager, output_path, logger)
    print 'Write %s hasErr? %s' % (output_path, hasErr)
    if hasErr:
        print logger.convertToString()
