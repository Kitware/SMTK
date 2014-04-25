"""
Expermental script to translate CMB2 template files to CMB3 format.
i.e., USE THIS SCRIPT AT YOUR OWN RISK!
This script does not completely migrate everything, but it should
help reduce the manual workload.

This script parses the CMB2 template in python, and creates the
corresponding attdefs in current (CMB3) format.

Only generates Definitions, does NOT generate sections.

There are two basic patterns from input to output:
    Pattern #1:
        <TopLevel>
            <Templates>
                <Instance>              --> Definition
                    <InformationValue>  --> ItemDefinition

    Pattern #2:
        <TopLevel>
            <InformationValue>  --> Definition & ItemDefinition

This code presumes that the input template is valid.

Also hard-codes one ExpressionType definition, for connecting to elements
specifying "FunctionId" as their <ValueType>.
"""

import sys
import xml.etree.ElementTree as ET

app_description = 'Python script to parse CMB2.0 template file' + \
    ' and generate eqivalent defintions in CMB3.0 format.'

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

import smtk



# Use one AttDef as the global/default expression, initialized in main()
global_exp_def = None


def create_item(elem, name, categories, indent=''):
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
    elif valuetype == 'FunctionId':
        item = smtk.attribute.DoubleItemDefinition.New(name)
        # Assign global expression type
        item.setExpressionDefinition(global_exp_def)
    else:
        print '%sCode for type %s not implemented -- skipping' % (indent, valuetype)
        return None

    # Handle <Advanced>
    advanced_elem = elem.find('Advanced')
    if advanced_elem is not None:
        item.setAdvanceLevel(1)

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
        elif ctype == 'Discrete':
            enum_elem_list = constraint_elem.findall('Enum')
            for enum_elem in enum_elem_list:
                value = enum_elem.attrib.get('Value')
                name = enum_elem.attrib.get('Name')
                item.addDiscreteValue(cast(value), name)
            default_index = constraint_elem.attrib.get('DefaultIndex')
            if default_index is not None:
                item.setDefaultDiscreteIndex(int(default_index))

    # Handle <DefaultValue>
    default_elem = elem.find('DefaultValue')
    if default_elem is not None:
        default_value = default_elem.attrib.get('Value')
        item.setDefaultValue(cast(default_value))

    # Handle <Group>, which maps to Category
    group_elem = elem.find('Group')
    if group_elem is not None:
        category = group_elem.attrib.get('Name')
        item.addCategory(category)
        categories.add(category)

    # Handle <Option>
    option_elem = elem.find('Option')
    if option_elem is not None:
        item.setIsOptional(True)
        default_string = option_elem.attrib.get('Default', 'OFF')
        default_value = True if default_string == 'ON' else False
        item.setIsEnabledByDefault(default_value)

    # Handle <Units>
    units_attrib = elem.attrib.get('Units')
    if units_attrib is not None:
        item.setUnits(units_attrib)

    return item


def create_definition(manager, elem, indent=''):
    '''
    Instantiates Definition from element
    '''
    tagname = elem.attrib.get('TagName')
    name = elem.attrib.get('Name')
    uiname = elem.attrib.get('UIName')
    print '%s<%s> name %s, tagname %s, UIName \"%s\"' % \
        (elem.tag, indent, name, tagname, uiname)

    att_type = tagname
    if att_type is None:
        att_type = name
    print '%sCreate definition for <%s> %s' % (elem.tag, indent, tagname)
    defn = manager.createDefinition(att_type)
    if defn is None:
        print 'ERROR - manager did not create definition \"%s\"' % att_type
        return None

    if uiname is not None:
        defn.setLabel(uiname)

    return defn


def process_multivalue_elem(elem, group_item, category, categories, indent=''):
    '''
    Processes <MultiValue> elements
    '''
    child_indent = '  ' + indent
    input_elem_list = elem.findall('InputValue')
    for input_elem in input_elem_list:
        name = 'Value%s' % input_elem.attrib.get('Index')
        input_item = create_item(input_elem, name, categories, child_indent)
        if input_item is not None:
            # Assign category to first item (only)
            if category is not None:
                input_item.addCategory(category)
                category = None
            group_item.addItemDefinition(input_item)


def process_information_elem(elem, defn, categories, indent=''):

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
        if uiname is not None:
            item.setLabel(uiname)

        category = None
        group_elem = elem.find('Group')
        if group_elem is not None:
            category = group_elem.attrib.get('Name')
        process_multivalue_elem(mvalue_elem, item, category, categories, child_indent)
        defn.addItemDefinition(item)
        return

    item = create_item(elem, tagname, categories, child_indent)
    if item is None:
        return

    # Set Label
    if uiname is not None:
        item.setLabel(uiname)

    # Check for <BoundaryConditionGroup Name="Nodal"
    bcgroup_elem = elem.find('BoundaryConditionGroup')
    if bcgroup_elem is not None:
        name = bcgroup_elem.attrib.get('Name')
        if name == 'Nodal':
            defn.setIsNodal(True)

    defn.addItemDefinition(item)


def process_instance_elem(manager, elem, view, categories, indent=''):

    '''
    Processes <Instance> element
    '''
    child_indent = '  ' + indent
    defn = create_definition(manager, elem, child_indent)
    if defn is None:
        return

    for child in elem.findall('InformationValue'):
        process_information_elem(child, defn, categories, child_indent)
    view.addDefinition(defn)


def process_templates_elem(manager, elem, view, categories, indent=''):
    '''
    Processes <Templates> element
    '''
    child_indent = '  ' + indent
    for child in elem.findall('Instance'):
        if child.tag == 'Instance':
            process_instance_elem(manager, child, view, categories, child_indent)

    for child in elem.findall('InformationValue'):
        defn = create_definition(manager, child)
        if defn is None:
            continue

        process_information_elem(elem, defn, categories, child_indent)
        view.addDefinition(defn)


def process_toplevel_elem(manager, elem, indent=''):
    '''
    Process <TopLevel> elements
    '''
    categories = set()
    child_indent = '  ' + indent
    skip_types = set(['Functions', 'Domain'])
    section_type = elem.attrib.get('Section')
    if section_type in skip_types:
        print '%s(skipping)' % child_indent
        return

    # Create AttributeView because we can add definitions to them
    print 'SECTION', section_type
    view = smtk.view.Attribute.New(section_type)
    manager.rootView().addSubView(view)

    for child in elem.findall('Templates'):
        process_templates_elem(manager, child, view, categories, child_indent)

    for child in elem.findall('InformationValue'):
        # Create defn then create item
        defn = create_definition(manager, child, child_indent)
        if defn is not None:
            view.addDefinition(defn)
            process_information_elem(child, defn, categories, child_indent)

    # Add placeholder analysis type so we can add categories
    # Otherwise smtk will refuse to read the file
    manager.defineAnalysis('PlaceholderAnalysis', list(categories))
    manager.updateCategories()


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
    import argparse

    print

    # Get command line args
    parser = argparse.ArgumentParser(description=app_description)
    parser.add_argument('-i', '--input_template_file', required=True)
    parser.add_argument('-o', '--output_template_file', default='output.sbt')
    args = parser.parse_args()

    # Instantiate attribute manager for outpot
    manager = smtk.attribute.Manager()

    # Generate default expression types
    sim_exp = manager.createDefinition('SimExpression')
    sim_exp.setIsAbstract(True)
    interp_exp = manager.createDefinition('SimInterpolation', sim_exp)
    interp_exp.setIsAbstract(True)
    poly_exp = manager.createDefinition('PolyLinearFunction', interp_exp)
    poly_exp.setLabel('PolyLinear Function')

    group_item = smtk.attribute.GroupItemDefinition.New('ValuePairs')
    group_item.setLabel('Value Pairs')
    x_item = smtk.attribute.DoubleItemDefinition.New('X')
    group_item.addItemDefinition(x_item)
    value_item = smtk.attribute.DoubleItemDefinition.New('Value')
    group_item.addItemDefinition(value_item)
    poly_exp.addItemDefinition(group_item)

    name_item = smtk.attribute.StringItemDefinition.New('Sim1DLinearExp')
    poly_exp.addItemDefinition(name_item)
    global_exp_def = poly_exp

    #input_path = '/media/ssd/sim/cmb_core/git/CMB/Source/Applications/ModelBuilder/SimBuilder/XML/GEOTACS_Template.sbt'
    input_path = args.input_template_file
    tree = ET.parse(input_path)
    root = tree.getroot()
    print 'root', root.tag
    for child in root:
        print 'elem', child.tag
        if child.tag == 'TopLevelGroup':
            process_toplevel_group(manager, child)

    # Write manager to template file
    output_path = args.output_template_file
    writer = smtk.util.AttributeWriter()
    logger = smtk.util.Logger()
    hasErr = writer.write(manager, output_path, logger)
    print 'Write %s hasErr? %s' % (output_path, hasErr)
    if hasErr:
        print logger.convertToString()
