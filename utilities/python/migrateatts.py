"""
Python script to migrate attributes from one file to a new file
based on an updated set of attribute definitions. Intended mostly
for development use.

Program inputs are:
  - Attribute file containing the source attributes
  - Template file containing the new/updated definitions

Program outputs are:
  -
  - a list of any conflicts detected

Each source attribute will be validated with the new template file
before being copied into the output file. If any conflict is found,
the attribute will not be copied, but instead a log message(s) will
describe the conflict.
"""

import sys
sys.path[0] = ''  # don't know why, but sys.path[0] breaks import smtk
try:
    import smtk
except ImportError:
    print 'Not able to import smtk library. You might need to:'
    print '  - Use the PYTHONPATH variable to point to the smtk python lib'
    print '  - And/or use the LD_LIBRARY_PATH variable to point to the shiboken libraries'
    print
    sys.exit(-1)


def usage():
    '''
    Writes usage message to the console
    '''
    print
    print 'Script to migrate attributes to new version of same template'
    print 'Usage: python %s  attribute_file  template_file  output_file  [log_file]'
    print '  attribute_file:  contains attribute to be migrated'
    print '  template_file:   contains updated definitions'
    print '  output_filename: name of output file to be written'
    print '  log_filename:    (optional) name of message log file'
    print

def to_concrete_item_definition(defn, logger):
    '''
    Needed to downcast itemDefinition
    TODO Update smtk.to_concrete() to inlude these
    '''
    map = {
        smtk.attribute.Item.DOUBLE: smtk.attribute.DoubleItemDefinition.CastTo,
        smtk.attribute.Item.GROUP: smtk.attribute.GroupItemDefinition.CastTo,
        smtk.attribute.Item.INT: smtk.attribute.IntItemDefinition.CastTo,
        smtk.attribute.Item.STRING: smtk.attribute.StringItemDefinition.CastTo,
    }

    f = map.get(defn.type())
    if f:
        return f(defn)
    else:
        logger.addError('Cannot convert %s to concrete definition', str(defn))

    return None


def find_discrete_index(value, defn):
    '''
    Tries to match value to a discrete index in item definition
    Returns None if not found
    '''
    svalue = str(value)
    for i in range(defn.numberOfDiscreteValues()):
        if svalue == defn.discreteEnum(i):
            return i
    return None


def copy_group_item(src_item, dst_item, defn, manager, logger):
    '''
    '''
    if src_item.numberOfGroups() < defn.numberOfRequiredGroups():
        message = '  Cannot copy group item %s' % src_item.name() + \
            'because it has %d groups but definition requires %d.' % \
            (src_item.numberOfGroups(), defn.numberOfRequiredGroups())
        logger.addWarning(message)
        return False

    if defn.numberOfRequiredGroups() > 1 and \
        src_item.numberOfItemsPerGroup() != dst_item.numberOfItemsPerGroup():
        message = '  Cannot copy group item %s' % src_item.name() + \
            'because its number of groups (%d) and items per group (%d)' % \
            (src_item.numberOfGroups(), src_item.numberOfItemsPerGroup()) + \
            ' do not match destination item.'
        logger.addWarning(message)
        return False

    # Copy input content to output
    for i in range(src_item.numberOfGroups()):
        for j in range(src_item.numberOfItemsPerGroup()):
            src_child = src_item.item(i, j)
            dst_child = dst_item.find(src_child.name())
            if dst_child is None:
                continue

            child_defn = defn.itemDefinition(j)
            if not copy_item(src_child, dst_child, child_defn, manager, logger):
                return False

    return True


def copy_value_item(src_item, dst_item, defn, manager, logger):
    '''
    '''
    if src_item.numberOfRequiredValues() < defn.numberOfRequiredValues():
        message = '  Cannot copy item %s because it does not have enough required values' % \
            src_item.name()
        logger.addWarning(message)
        return False

    # Set number of values in dst_item
    dst_item.setNumberOfValues(src_item.numberOfValues())

    # Traverse each value
    for i in range(dst_item.numberOfValues()):
        # Is value set
        if not src_item.isSet(i):
            dst_item.unset(i)
            continue

        # Is value an expression
        if src_item.isExpression(i):
            if not dst_item.allowsExpressions():
                message = '  Cannot copy value %d for item %s' % (i, src_item.name()) + \
                    ' because expressions are not allowed in the destination item'
                logger.addRecord(smtk.util.Logger.INFO, message)
                return False

            # Find dst expression with same name as src expression
            src_exp = src_item.expression(i)
            exp_name = src_exp.name()
            dst_exp = manager.findAttribute(exp_name)
            if dst_exp is None:
                print 'Cannot find expression %s' % name
                continue

            ok = dst_item.setExpression(i, dst_exp)
            if not ok:
                print 'setExpression returned %s' % ok
            continue

        # Is value a discrete index
        if src_item.isDiscrete():
            if not dst_item.isDiscrete():
                message = '  Cannot copy value %d for item %s' % (i, src_item.name()) + \
                    ' because source *is* discrete but destination is *not*.'
                logger.addRecord(smtk.util.Logger.INFO, message)
                return False
            else:
                dst_item.setDiscreteIndex(i, src_item.discreteIndex(i))
                continue
        elif dst_item.isDiscrete() and not src_item.isDiscrete():
            index = find_discrete_index(src_item.value(i), defn)
            if index is None:
                message = '  Cannot copy value %d for item %s (%s)' % \
                    (i, src_item.name(), src_item.value(i)) + \
                    ' because source is *not* discrete but destination *is*,' + \
                    ' and no corresponding index found.'
                logger.addRecord(smtk.util.Logger.INFO, message)
                return False
            else:
                dst_item.setDiscreteIndex(index)
                message = '  Converted string value %s for item %s' % \
                    (src_item.value(i), src_item.name()) + \
                    ' to discrete index %d.' % index
                logger.addRecord(smtk.util.Logger.INFO, message)
                continue

        # Is value valid
        src_value = src_item.value(i)
        if defn.isValueValid(src_value):
            dst_item.setValue(i, src_value)
        else:
            message = '  Cannot copy value %s for item %s' % (i, src_item.name()) + \
                ' because %s is not valid' % src_item.value(i)
            logger.addRecord(smtk.util.Logger.INFO, message)
            return False

    return True


def copy_item(src_item, dst_item, defn, manager, logger):
    '''
    Recursively copies contents of items from source to destination
    '''
    message = '  Copying item %s' % src_item.name()
    logger.addRecord(smtk.util.Logger.INFO, message)

    # Copy stuff common to all item
    if dst_item.isOptional():
        dst_item.setIsEnabled(src_item.isEnabled())

    # Void type is covered
    if src_item.type() == smtk.attribute.Item.VOID:
        return True

    # Call type-specific functions
    concrete_src_item = smtk.attribute.to_concrete(src_item)
    concrete_dst_item = smtk.attribute.to_concrete(dst_item)
    concrete_defn = to_concrete_item_definition(defn, logger)

    if src_item.type() == smtk.attribute.Item.ATTRIBUTE_REF:
        logger.addWarning('  Sorry - Cannot yet copy AttributeRefItem instances')
        return False

    if src_item.type() == smtk.attribute.Item.GROUP:
        return copy_group_item(concrete_src_item, concrete_dst_item, \
            concrete_defn, manager, logger)

    valitem_list = [
        smtk.attribute.Item.DOUBLE,
        smtk.attribute.Item.INT,
        smtk.attribute.Item.STRING
    ]
    if src_item.type() in valitem_list:
        return copy_value_item(concrete_src_item, concrete_dst_item, \
            concrete_defn, manager, logger)

    else:
        message = '  Sorry - cannot yet copy item type %s' % src_item.type()
        logger.addWarning(message)
        return False
    return False


def copy_attribute(src_att, defn, logger):
    '''
    Copies attribute to new manager
    Returns boolean indicating success status
    '''
    #print 'dst defn', defn
    message = 'Copying attribute type %s id %d' % \
        (src_att.type(), src_att.id())
    logger.addRecord(smtk.util.Logger.INFO, message)

    manager = defn.manager()

    # If defn is in instance view, attribute already created
    dst_att = manager.findAttribute(src_att.name())
    if dst_att is not None and dst_att.id() != src_att.id():
        message = 'Renumbering attribute %s from %d to %d' % \
            (dst_att.name(), dst_att.id(), src_att.id())
        logger.addRecord(smtk.util.Logger.INFO, message)
        manager.removeAttribute(dst_att)
    if dst_att is None:
        dst_att = manager.createAttribute(src_att.name(), defn, src_att.id())

    if src_att.appliesToBoundaryNodes():
        dst_att.setAppliesToBoundaryNodes()
    elif src_att.appliesToInteriorNodes():
        dst_att.setAppliesToInteriorNodes()

    if src_att.isColorSet():
        dst_att.setColor(src_att.color())

    # Copy contents of items
    for i in range(src_att.numberOfItems()):
        src_item = src_att.item(i)
        dst_item = dst_att.find(src_item.name())
        if dst_item is None:
            message = '  Skipping item %s because not in definition' % \
                src_item.name()
            logger.addRecord(smtk.util.Logger.INFO, message)
        else:
            item_defn = defn.itemDefinition(i)
            success = copy_item(src_item, dst_item, item_defn, manager, logger)

        if not success:
            manager.removeAttribute(dst_att)
            return False

    # Copy model associations (only works when using CMB export app)
    for model_ent in src_att.associatedEntitiesSet():
        dst_att.associateEntity(model_ent)

    return True


def migrate(src_manager, dst_manager):
    '''
    Copies attributes from src to dst
    Returns Logger of messages
    '''
    logger = smtk.util.Logger()

    # Set refModel if src_manager has one
    if src_manager.refModel() is not None:
        dst_manager.setRefModel(src_manager.refModel())

    # Traverse all source attributes to build table sorted by type
    src_count = 0
    src_dict = dict()
    next_id = src_manager.nextId()
    for i in range(next_id):
        att = src_manager.findAttribute(i)
        if att is None:
            continue

        att_list = src_dict.get(att.type())
        if att_list is None:
            att_list = list()
            src_dict[att.type()] = att_list
        att_list.append(att)
        src_count += 1
    print 'Number of input attributes: %d' % src_count

    # Make list of att types, and put expressions first
    att_type_list = src_dict.keys()
    exp_type = 'PolyLinearFunction'
    exp_positions = [i for i,x in enumerate(att_type_list) if x == exp_type]
    if exp_positions:
        i = exp_positions[0]
        del att_type_list[i]
        att_type_list.insert(0, exp_type)
    #print 'att_types', att_type_list

    # Traverse contents of table in att_type_list order
    dst_count = 0
    for att_type in att_type_list:
        att_list = src_dict.get(att_type)

        # Check for definition in dst_manager
        defn = dst_manager.findDefinition(att_type)
        if defn is None:
            message = 'Unable to migrate attributes of type %s' % att_type + \
                ', because no definition found in destination template.' + \
                ' (Qty %d)' % len(att_list)
            logger.addRecord(smtk.util.Logger.INFO, message)
            continue
        elif defn.isAbstract():
            message = 'Unable to migrate attributes of type %s' % att_type + \
                ', because definition is abstract.' + \
                ' (Qty %d)' % len(att_list)
            logger.addRecord(smtk.util.Logger.INFO, message)
            continue

        for att in att_list:
            if copy_attribute(att, defn, logger):
                dst_count += 1

    print 'Number of attributes copied: %d' % dst_count

    # Break encapsulation to set manager's next id
    dst_manager.m_nextAttributeId = src_manager.nextId()

    return logger


if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage()
        sys.exit(-2)

    io_logger = smtk.util.Logger()

    # Load source attributes
    attribute_file = sys.argv[1]
    print 'Loading attribute file %s' % attribute_file
    attribute_reader = smtk.util.AttributeReader()
    src_manager = smtk.attribute.Manager()
    io_logger.reset()
    err = attribute_reader.read(src_manager, attribute_file, io_logger)
    if err:
        print 'WARNING - CANNOT READ ATTRIBUTE FILE %s' % attribute_file
        print io_logger.convertToString()

    # Load template file as the destination attribute
    template_file = sys.argv[2]
    print 'Loading template file %s' % template_file
    template_reader = smtk.util.AttributeReader()
    dst_manager = smtk.attribute.Manager()
    io_logger.reset()
    err = template_reader.read(dst_manager, template_file, io_logger)
    if err:
        print 'ERROR - CANNOT READ TEMPLATE FILE %s' % template_file
        print io_logger.convertToString()
        sys.exit(-3)

    # TODO Do we need to check that template has no attributes?
    # Or delete any existing attributes?

    # Traverse input and migrate attributes
    migrate_logger = smtk.util.Logger()
    migrate_logger = migrate(src_manager, dst_manager)

    # Write output file
    output_filename = sys.argv[3]
    writer = smtk.util.AttributeWriter()
    err = writer.write(dst_manager, output_filename, io_logger)
    if err:
        print 'ERROR - CANNOT WRITE OUTPUT FILE %s' % output_filename
        print io_logger.convertToString()
        sys.exit(-4)
    print 'Wrote %s' % output_filename

    log = migrate_logger.convertToString()
    # Write message to file
    if len(sys.argv) > 4:
        log_filename = sys.argv[4]
        with open(log_filename, 'w') as f:
            f.write(log)
            print 'Wrote log file %s' % log_filename
    # Write messages to console
    else:
        print 'Log:'
        print log
