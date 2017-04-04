"""
Python script to load smtk resource file and write out with options.
Requires smtk module
"""

import argparse
import os
import sys

import smtk

if __name__ == '__main__':
    description = 'Load resource file and write with options'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('input_file', help='Input resource file')
    parser.add_argument('output_file', help='Output resource file')
    parser.add_argument('-o', '--option', default='write',
                        help='linked files option: \"expand\", \"write\", or \"skip\"' +
                        ' (default is \"expand\")')
    args = parser.parse_args()

    resource_set = smtk.common.ResourceSet()
    reader = smtk.io.ResourceSetReader()
    logger = smtk.io.Logger()

    # Read input file
    hasErrors = reader.readFile(args.input_file, resource_set, logger)
    if hasErrors:
        print 'ERROR reading input file', args.input_file
        print logger.convertToString()
        sys.exit(-2)

    # Convert option to smtk.io.ResourceSetWriter::LinkedFilesOption
    linked_files_option = smtk.io.ResourceSetWriter.EXPAND_LINKED_FILES
    if args.option == 'skip':
        linked_files_option = smtk.io.ResourceSetWriter.SKIP_LINKED_FILES
    elif args.option == 'write':
        linked_files_option = smtk.io.ResourceSetWriter.WRITE_LINKED_FILES
    elif args.option != 'expand':
        print 'Unsupported option \"%s\"; using \"expand\" instead' % args.option

    writer = smtk.io.ResourceSetWriter()
    hasErr = writer.writeFile(
        args.output_file, resource_set, logger, linked_files_option)
    if hasErrors:
        print 'ERROR writing output file', output_file
        print logger.convertToString()
        sys.exit(-3)

    sys.exit(0)
