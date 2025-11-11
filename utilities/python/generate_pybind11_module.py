# =========================================================================
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
# =========================================================================

""" generate_pybind11_module.py:

Reads a directory containing C++ header files and creates the necessary C++
files to define the pybind11 wrappings for a python module.
"""

import os
import sys

from cpp_to_pybind11 import *
from toposort import toposort_flatten

if __name__ == '__main__':
    import argparse

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-I', '--include-dirs',
                            action='append',
                            help='Add an include directory to the parser',
                            default=[])

    arg_parser.add_argument('-i', '--input-directory',
                            help='<Required> Input directory',
                            required=True)

    arg_parser.add_argument('-m', '--module',
                            help='<Required> Module name',
                            required=True)

    arg_parser.add_argument('-o', '--output-directory',
                            help='Output directory',
                            required=True)

    arg_parser.add_argument('-p', '--prefix',
                            help='File name prefix',
                            default='Pybind')

    arg_parser.add_argument('-s', '--project-source-dir',
                            help='Project source directory',
                            default=".")

    arg_parser.add_argument('-v', '--verbose',
                            help='Print out generated wrapping code',
                            action='store_true')

    args = arg_parser.parse_args()

    if not os.path.exists(args.input_directory):
        raise IOError("No directory \"%s\"" % args.input_directory)

    if len(args.include_dirs) == 0:
        args.include_dirs.append(args.input_directory)

    def stream_with_line_breaks(stream):
        def write(string):
            stream.write(string.replace('>>', '> >'))
            stream.write('\n')

        def write_verbose(string):
            write(string)
            print(string.replace('>>', '> >'))
        if args.verbose:
            return write_verbose
        else:
            return write

    wrapped_objects = {}
    header_files = [os.path.join(args.input_directory, f)
                    for f in os.listdir(args.input_directory)
                    if os.path.splitext(f)[1] == ".h"]
    output_files = []

    for header_file in header_files:
        output_file = os.path.join(args.output_directory,
                                   args.prefix + os.path.basename(header_file))
        output_files.append(output_file)
        if not os.path.exists(os.path.dirname(output_file)):
            try:
                os.makedirs(os.path.dirname(output_file))
            except OSError as exc:  # Guard against race condition
                if exc.errno != errno.EEXIST:
                    raise
        with open(output_file, 'w') as f:
            stream = stream_with_line_breaks(f)
            wrapped_objects.update(
                parse_file(header_file, args.project_source_dir,
                           args.include_dirs, "", stream))

    filename = os.path.join(args.output_directory, args.prefix +
                            os.path.basename(args.input_directory).capitalize() + '.cxx')

    with open(filename, 'w') as f:
        stream = stream_with_line_breaks(f)

        # output file preamble
        stream("""//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
""")
        stream("#include <pybind11/pybind11.h>")
        stream("#include <utility>")
        stream("")
        stream("namespace py = pybind11;")
        stream("")
        stream("template <typename T, typename... Args>")
        stream(
            "using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;")
        stream("")
        for output_file in output_files:
            stream("#include \"%s\"" % os.path.basename(output_file))
        stream("")
        stream('PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);')
        stream("")
        stream("PYBIND11_MODULE(%s, m)" % args.module)
        stream("{")
        stream("  m.doc() = \"<description of %s>\";" % args.module)

        modules = set()
        for obj in wrapped_objects:
            parent = "m"
            for ns in get_scope(obj):
                if ns not in modules:
                    stream("  py::module %s = %s.def_submodule(\"%s\", \"<description>\");" % (
                        ns, parent, ns))
                    modules.add(ns)
                parent = ns
        stream("")
        stream(
            "  // The order of these function calls is important! It was determined by")
        stream(
            "  // comparing the dependencies of each of the wrapped objects.")

        topological_mapping = {}
        for obj in wrapped_objects:
            dependencies = set()
            if type(obj).__name__.find('class_t') != -1:
                for base_class in obj.recursive_bases:
                    parent = base_class.related_class
                    inherits = base_class.declaration_path
                    enable_shared = 'enable_shared_from_this<%s>' \
                        % full_class_name(obj)
                    if inherits != ['::', 'std', enable_shared]:
                        dependencies.add(parent)
            topological_mapping[obj] = dependencies

        sorted_objs = toposort_flatten(topological_mapping)

        for obj in sorted_objs:
            if obj not in wrapped_objects:
                continue
            scope = get_scope(obj)
            signature = wrapped_objects[obj]
            module = scope[-1] if len(scope) > 0 else "m"
            if type(obj).__name__.find('class_t') != -1:
                parent = get_parent(obj)
                stream("  %s %s = %s(%s);" % (bind_class_name(obj),
                                              mangled_name(obj), signature,
                                              module))
            else:
                stream("  %s(%s);" % (signature, module))
        stream("}")
