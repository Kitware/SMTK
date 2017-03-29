"""
Generates format table syntax for CMB 3.0 exporters (as of April 2014)
from CMB 2.0 format file input.
"""

import sys
import xml.etree.ElementTree as ET

# ---------------------------------------------------------------------


def process_file_element(scope, elem):
    file_type = elem.attrib.get('Type')
    print 'File type', file_type
    if file_type in ['Veg', 'Ray']:
        print 'Nonstandard format - skipping'
        return

    format_strings_elem = elem.find('FormatStrings')
    for child in format_strings_elem.findall('InformationValue'):
        tagname = child.attrib.get('TagName')
        value = child.attrib.get('Value')
        # print tagname, ':', value
        multivalue = child.attrib.get('MultiValue')

        var_start = value.find('#')
        opcode = value[:var_start].strip()

        varstring = '(none)'
        comment_start = value.find('!')
        comment_text = ''
        if comment_start > 0:
            comment = value[comment_start:]
            comment_string = ', comment=\'%s\'' % comment
            varstring = value[var_start:comment_start].strip()
        else:
            varstring = value[var_start:].strip()

        subitems_sting = ''

        # print tagname, opcode, varstring, comment
        # print '%-20s: %s \"%s\"  %s' % (tagname, opcode, varstring, comment)
        fmt = '    fmt.(\'%s\', \'%s\'%s%s),' % \
            (tagname, opcode, comment_string, subitems_sting)
        print fmt
        scope.output.write(fmt)
        scope.output.write('\n')


# ---------------------------------------------------------------------
if __name__ == '__main__':
    import argparse
    print

    # Get command-line arguments
    app_description = 'Python script to parse CMB2.0 format file' + \
        ' and generate equivalent format table(s) for CMB3.0 export script.'
    parser = argparse.ArgumentParser(description=app_description)
    parser.add_argument('-i', '--input_format_file', required=True)
    parser.add_argument('-o', '--output_file', default='output.txt')
    args = parser.parse_args()

    # Define scope object
    Scope = type('Scope', (object,), dict())
    scope = Scope()

    # Read input
    input_path = args.input_format_file
    tree = ET.parse(input_path)
    root = tree.getroot()
    print 'Root element', root.tag
    if root.tag != 'ADHOutputFormat':
        print 'WARNING: This file does NOT appear to be a format file'

    # Initialize output
    complete = False
    with open(args.output_file, 'w') as scope.output:
        for child in root:
            # print 'elem', child.tag
            if child.tag == 'File':
                process_file_element(scope, child)
        complete = True

    if not complete:
        print 'Script did not successfully complete - output may be missing or invalid'
    else:
        print 'Wrote', args.output_file
