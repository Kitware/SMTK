# =========================================================================
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
# =========================================================================

""" cpp_to_pybind11.py:

Reads a C++ header file and creates a C++ header file with pybind11 bindings for
the objects defined in the input file. cpp_to_pybind11.py uses pygccxml to parse
the input C++, and the generated C++ file contains functions used for generating
bindings. These generated functions must be called within a PYBIND11_PLUGIN code
block that is compiled into a python module. cpp_to_pybind11.py can be used as a
standalone binding generator for a single header file, or
generate_pybind11_bindings.py can be used to generate an entire module from a
directory of C++ header files.
"""

import os
import sys
from typing import List

# Find out the file location within the sources tree
this_module_dir_path = os.path.abspath(
    os.path.dirname(sys.modules[__name__].__file__))
# Add pygccxml package to Python path
sys.path.append(os.path.join(this_module_dir_path, '..', '..'))

from pygccxml import parser  # nopep8
from pygccxml import declarations  # nopep8
from pygccxml import utils  # nopep8

operator_map = {'+':  '__add__',
                '-':  '__sub__',
                '*':  '__mul__',
                '/':  '__div__',
                '()': '__call__',
                '%':  '__mod__',
                '<':  '__lt__',
                '<=': '__le__',
                '>':  '__gt__',
                '>=': '__ge__',
                '==': '__eq__',
                '!=': '__ne__',
                '=':  'deepcopy'}


def get_scope(namespace):
    """
    Returns a list of namespaces from global to the input namespace
    """
    scope = []
    current_namespace = namespace.parent
    while current_namespace != namespace.top_parent:
        scope.append(current_namespace.name)
        current_namespace = current_namespace.parent

    scope = scope[::-1]
    return scope


def full_class_name(class_):
    """
    Returns the absolute name of a class, with all nesting namespaces included
    """
    return '::'.join(get_scope(class_) + [class_.name])


def mangled_name(obj):
    """
    Returns a string that is unique to the object and can be used as a variable
    name
    """
    return '_'.join(get_scope(obj) + [obj.name]) \
        .replace('<', '_').replace('>', '_').replace('::', '_')


def get_parent(class_):
    """
    Returns the parent of a class, if one exists
    """
    base_classes = class_.recursive_bases
    parent = None
    if base_classes and len(base_classes) > 0:
        parent = base_classes[0].related_class
        inherits = base_classes[0].declaration_path
        enable_shared_decl = 'enable_shared_from_this<%s>' \
            % full_class_name(class_)
        if inherits == ['::', 'std', enable_shared_decl]:
            parent = None
    return parent


def bind_class_name(class_):
    """
    Returns the pybind11 class name used to declare the class
    """
    parent = get_parent(class_)
    parent_name = ""
    if parent:
        parent_name = ", " + full_class_name(parent)
    base_classes = class_.recursive_bases
    if base_classes or len(base_classes) > 0:
        for base in base_classes:
            for decl in base.declaration_path:
                if decl.find('enable_shared_from_this') != -1:
                    return "PySharedPtrClass< %s%s >" % \
                        (full_class_name(class_), parent_name)
    return "py::class_< %s%s >" % (full_class_name(class_), parent_name)


parsed_classes = set()


def file_from_class_name(class_name):
    """
    Given a class name, returns the header file in which it is defined.
    NOTE: this function assumes the convention of namespaces being directories
          and headers named after the class they define.
    """
    file_name = class_name.split(" ")[0].replace("::", "/") + ".h"
    file_name = file_name.replace("Ptr", "")
    if file_name.startswith("/"):
        return file_name[1:]
    return file_name


def get_includes(global_ns, filename, project_source_directory):
    """
    Given the global namespace, the name of an input header file and the root
    directory of a project, returns a set containing the header files within
    within the project that must be included to bind the objects in the input
    file.
    """
    includes = set()

    for class_ in global_ns.classes(allow_empty=True):
        if class_.location.file_name != os.path.abspath(filename) or\
                class_ in parsed_classes:
            continue
        top_namespace = full_class_name(class_).split("::")[0]
        parent = get_parent(class_)
        if parent:
            file_name = file_from_class_name(full_class_name(parent))
            if file_name.split("/")[0] == top_namespace:
                includes.add(file_name)
        for constructor in class_.constructors():
            if constructor.parent.name != class_.name:
                continue
            if constructor.access_type != 'public':
                continue
            for arg in constructor.arguments:
                file_name = file_from_class_name(arg.decl_type.decl_string)
                if file_name.split("/")[0] == top_namespace:
                    includes.add(file_name)
        for operator in class_.operators(allow_empty=True):
            if operator.parent.name != class_.name:
                continue
            if operator.access_type != 'public':
                continue
            for arg in operator.arguments:
                file_name = file_from_class_name(arg.decl_type.decl_string)
                if file_name.split("/")[0] == top_namespace:
                    includes.add(file_name)
            file_name = file_from_class_name(operator.return_type.decl_string)
            if file_name.split("/")[0] == top_namespace:
                includes.add(file_name)
        for member in class_.public_members:
            if member.parent.name != class_.name:
                continue
            if member.__class__.__name__ != "member_function_t":
                continue
            for arg in member.arguments:
                file_name = file_from_class_name(arg.decl_type.decl_string)
                if file_name.split("/")[0] == top_namespace:
                    includes.add(file_name)
            file_name = file_from_class_name(member.return_type.decl_string)
            if file_name.split("/")[0] == top_namespace:
                includes.add(file_name)
        for variable in class_.variables(allow_empty=True):
            if variable.parent.name != class_.name:
                continue
            if variable.access_type != "public":
                continue
            file_name = file_from_class_name(variable.decl_string)
            if file_name.split("/")[0] == top_namespace:
                includes.add(file_name)

    for fn_ in global_ns.free_functions(allow_empty=True):
        if fn_.location.file_name != os.path.abspath(filename):
            continue
        top_namespace = full_class_name(fn_).split("::")[0]
        for arg in fn_.arguments:
            file_name = file_from_class_name(arg.decl_type.decl_string)
            if file_name.split("/")[0] == top_namespace:
                includes.add(file_name)
        file_name = file_from_class_name(fn_.return_type.decl_string)
        if file_name.split("/")[0] == top_namespace:
            includes.add(file_name)

    def flatten(list_, separator):
        s = ""
        for x in list_:
            s = s + x + separator
        return s

    incs = list(includes)
    for inc in incs:
        split = inc.split("/")
        type_caster_file = flatten(
            split[:-1], "/") + "pybind11/Pybind" + split[-1].replace(".h", "") + "TypeCaster.h"
        includes.add(type_caster_file)

    incs = list(includes)
    for inc in incs:
        if not os.path.isfile(os.path.abspath(project_source_directory) + "/" +
                              inc):
            includes.remove(inc)

    this_file = os.path.relpath(os.path.abspath(filename),
                                os.path.commonprefix(
        [os.path.abspath(project_source_directory),
         os.path.abspath(filename)]))

    if this_file in includes:
        includes.remove(this_file)

    return sorted(includes)


def parse_file(filename, project_source_directory, include_directories: List[str],
               declaration_names, stream):
    """
    Entry point for parsing a file
    """
    # Find out the xml generator (gccxml or castxml)
    generator_path, generator_name = utils.find_xml_generator()

    # Configure the xml generator
    config = parser.xml_generator_configuration_t(
        start_with_declarations=declaration_names.split(" "),
        include_paths=include_directories,
        xml_generator_path=generator_path,
        xml_generator=generator_name,
        cflags='-std=c++11 -Wc++11-extensions')

    # Parse source file
    decls = parser.parse([os.path.abspath(filename)], config,
                         compilation_mode=parser.COMPILATION_MODE.ALL_AT_ONCE)

    # grab global namespace
    try:
        global_ns = declarations.get_global_namespace(decls)
    except RuntimeError:
        return []

    # output file preamble
    fileguard = "pybind_" + \
        os.path.relpath(os.path.abspath(filename),
                        os.path.commonprefix(
            [os.path.abspath(project_source_directory),
             os.path.abspath(filename)])).replace('.', '_').replace('/', '_').replace('-', '_')

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
    stream("#ifndef %s" % fileguard)
    stream("#define %s" % fileguard)
    stream("")
    stream("#include <pybind11/pybind11.h>")
    stream("")
    stream("#include \"%s\"" % os.path.relpath(os.path.abspath(filename),
                                               os.path.commonprefix(
        [os.path.abspath(project_source_directory),
         os.path.abspath(filename)])))
    stream("")
    includes = get_includes(global_ns, filename, project_source_directory)
    for include in includes:
        stream("#include \"%s\"" % include)
    if includes:
        stream("")
    stream("namespace py = pybind11;")
    stream("")

    wrapped_objects = {}

    for enum_ in global_ns.enumerations(allow_empty=True):
        if enum_.location.file_name != os.path.abspath(filename):
            continue
        if enum_.parent and type(enum_.parent).__name__.find('class_t') != -1:
            continue
        wrapped_objects[enum_] = parse_free_enumeration(enum_, stream)
        stream("")

    for class_ in global_ns.classes(allow_empty=True):
        if class_.location.file_name != os.path.abspath(filename) or\
                class_ in parsed_classes:
            continue
        if class_.parent and type(class_.parent).__name__.find('class_t') != -1:
            continue
        wrapped_objects[class_] = parse_class(class_, stream)
        stream("")

    all_functions = set()
    overloaded_functions = set()
    for fn_ in global_ns.free_functions(allow_empty=True):
        if fn_.location.file_name != os.path.abspath(filename):
            continue
        if fn_.name in all_functions:
            overloaded_functions.add(fn_.name)
        else:
            all_functions.add(fn_.name)

    for fn_ in global_ns.free_functions(allow_empty=True):
        if fn_.location.file_name != os.path.abspath(filename):
            continue
        wrapped_objects[fn_] = parse_free_function(
            fn_, fn_.name in overloaded_functions, stream)
        stream("")

    stream("#endif")
    parsed_classes.clear()

    return wrapped_objects


def parse_free_enumeration(enum, stream):
    """
    Write bindings for a free enumeration
    """
    init_function_name = "pybind11_init_" + mangled_name(enum)
    stream("inline void %s(py::module &m)" % init_function_name)
    stream("{")
    full_enum_name = full_class_name(enum)
    stream("  py::enum_<%s>(m, \"%s\")" %
           (full_enum_name, enum.name))
    for (name, num) in enum.values:
        stream("    .value(\"%s\", %s::%s)" %
               (name, full_enum_name, name))
    stream("    .export_values();")
    stream("}")
    return init_function_name


def parse_free_function(func, overloaded, stream):
    """
    Write bindings for a free function
    """
    init_function_name = "pybind11_init_" + \
        (func.mangled if overloaded else mangled_name(func))
    stream("inline void %s(py::module &m)" % init_function_name)
    stream("{")
    if overloaded:
        stream("  m.def(\"%s\", (%s (*)(%s)) &%s, \"\", %s);" %
               (func.name, func.return_type, ', '.join([arg.decl_type.decl_string
                                                        for arg in func.arguments]),
                '::'.join(get_scope(func) + [func.name]),
                ', '.join(["py::arg(\"%s\") = %s" % (arg.name, arg.default_value)
                           if arg.default_value is not None
                           else "py::arg(\"%s\")" % (arg.name)
                           for arg in func.arguments])))
    else:
        stream("  m.def(\"%s\", &%s, \"\", %s);" %
               (func.name, '::'.join(get_scope(func) + [func.name]),
                ', '.join(["py::arg(\"%s\") = %s" % (arg.name, arg.default_value)
                           if arg.default_value is not None
                           else "py::arg(\"%s\")" % (arg.name)
                           for arg in func.arguments])))
    stream("}")
    return init_function_name


def has_static(class_):
    for member in class_.public_members:
        if member.parent.name != class_.name:
            continue
        if member.__class__.__name__ != "member_function_t":
            continue
        if member.has_static:
            return True

    for variable in class_.variables(allow_empty=True):
        if variable.parent.name != class_.name:
            continue
        if variable.access_type == "public" \
                and variable.type_qualifiers.has_static:
            return True

    return False


def parse_class(class_, stream, top_level=True):
    """
    Write bindings for a class
    """
    full_class_name_ = full_class_name(class_)
    base_classes = class_.recursive_bases

    init_function_name = "pybind11_init_" + mangled_name(class_)

    if top_level:
        stream("inline %s %s(py::module &m)" % (bind_class_name(class_),
                                                init_function_name))
        stream("{")
        stream("  %s instance(m, \"%s\");" %
               (bind_class_name(class_),
                class_.name.replace('<', '_').replace('>', '_').replace('::', '_')))
        stream("  instance")
    else:
        stream("  %s(instance, \"%s\")" %
               (bind_class_name(class_),
                class_.name.replace('<', '_').replace('>', '_').replace('::', '_')))

    if not class_.is_abstract:
        for constructor in class_.constructors():
            if constructor.parent.name != class_.name:
                continue
            if constructor.access_type != 'public':
                continue
            stream("    .def(py::init<%s>())" %
                   ', '.join([arg.decl_type.decl_string
                              for arg in constructor.arguments]))

    for operator in class_.operators(allow_empty=True):
        if operator.parent.name != class_.name:
            continue
        if operator.access_type != 'public':
            continue
        const = ' const' if operator.has_const else ''
        if operator.symbol == '[]':
            stream("    .def(\"__setitem__\", (%s (%s::*)(%s)) &%s::operator%s)"
                   % (operator.return_type, full_class_name_,
                      ', '.join([arg.decl_type.decl_string
                                 for arg in operator.arguments]),
                      full_class_name_, operator.symbol))
            stream("    .def(\"__getitem__\", (%s (%s::*)(%s)%s) &%s::operator%s)"
                   % (operator.return_type, full_class_name_,
                      ', '.join([arg.decl_type.decl_string
                                 for arg in operator.arguments]),
                      const, full_class_name_, operator.symbol))
        elif operator.symbol in operator_map:
            stream("    .def(\"%s\", (%s (%s::*)(%s)%s) &%s::operator%s)" %
                   (operator_map[operator.symbol], operator.return_type,
                    full_class_name_,
                    ', '.join([arg.decl_type.decl_string
                               for arg in operator.arguments]),
                    const, full_class_name_, operator.symbol))
        else:
            print("WARNING: no built-in type for operator", operator.name)

    all_methods = set()
    overloaded_methods = set()
    for member in class_.public_members:
        if member.parent.name != class_.name:
            continue
        if member.__class__.__name__ != "member_function_t":
            continue
        if member.name in all_methods:
            overloaded_methods.add(member.name)
        else:
            all_methods.add(member.name)

    # this code block separates method pairs that look like "XXX()" and
    # "setXXXX()" and flags them to be used as set-s and get-s to define a
    # property.
    use_properties = False
    property_set_methods = set()
    property_get_methods = set()
    if use_properties:
        def uncapitalize(s):
            return s[:1].lower() + s[1:] if s else ''
        property_set_methods = set(
            [m for m in class_.public_members if m.name[:3] == "set" and
             m.name not in overloaded_methods and
             uncapitalize(m.name[3:]) not in overloaded_methods and
             uncapitalize(m.name[3:]) in all_methods])
        property_get_methods = set(
            [m for m in class_.public_members if
             'set%s' % m.name.capitalize() not in overloaded_methods and
             m.name not in overloaded_methods and
             'set%s' % m.name.capitalize() in all_methods])

        unique_properties = set()
        for prop in property_set_methods:
            static = '_static' if prop.has_static else ''
            set_name = prop.name
            get_name = uncapitalize(prop.name[3:])
            if get_name not in unique_properties:
                stream("    .def_property%s(\"%s\", &%s::%s, &%s::%s)" %
                       (static, get_name, full_class_name_, get_name,
                        full_class_name_, set_name))
                unique_properties.add(get_name)

    for member in class_.public_members:
        if member.parent.name != class_.name:
            continue
        if member.__class__.__name__ != "member_function_t":
            continue
        static = '_static' if member.has_static else ''
        const = ' const' if member.has_const else ''
        args_ = (', ' + ', '.join(["py::arg(\"%s\") = %s" % (arg.name, arg.default_value)
                                   if arg.default_value is not None else "py::arg(\"%s\")"
                                   % (arg.name) for arg in member.arguments])) \
            if len(member.arguments) != 0 else ''
        if member in property_set_methods or member in property_get_methods:
            continue
        if member.name in overloaded_methods:
            stream("    .def%s(\"%s\", (%s (%s*)(%s)%s) &%s::%s%s)" %
                   (static, member.name, member.return_type,
                    ("" if member.has_static else full_class_name_ + "::"),
                       ', '.join([arg.decl_type.decl_string
                                  for arg in member.arguments]),
                       const, full_class_name_, member.name, args_))
        else:
            stream("    .def%s(\"%s\", &%s::%s%s)" %
                   (static, member.name, full_class_name_, member.name, args_))

    for variable in class_.variables(allow_empty=True):
        if variable.parent.name != class_.name:
            continue
        if variable.access_type == "public":
            static = '_static' if variable.type_qualifiers.has_static else ''
            if declarations.is_const(variable):
                stream("    .def_readonly%s(\"%s\", &%s::%s)" %
                       (static, variable.name, full_class_name_, variable.name))
            else:
                stream("    .def_readwrite%s(\"%s\", &%s::%s)" %
                       (static, variable.name, full_class_name_, variable.name))

    stream("    ;")

    for enum in class_.enumerations(allow_empty=True):
        stream("  py::enum_<%s::%s>(%s, \"%s\")" %
               (full_class_name_, enum.name, "instance", enum.name))
        for (name, num) in enum.values:
            stream("    .value(\"%s\", %s::%s::%s)" %
                   (name, full_class_name_, enum.name, name))
        stream("    .export_values();")

    for decl in class_.declarations:
        if type(decl).__name__.find('class_t') != -1:
            parse_class(decl, stream, False)

    parsed_classes.add(class_)

    if top_level:
        stream("  return instance;")
        stream("}")
        return init_function_name
    else:
        return


if __name__ == '__main__':
    import argparse

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-I', '--include-dirs',
                            help='Add an include directory to the parser',
                            action='append',
                            default=[])

    arg_parser.add_argument('-d', '--declaration-name',
                            help='names of C++ classes and functions',
                            default="")

    arg_parser.add_argument('-i', '--input',
                            help='<Required> Input C++ header file',
                            required=True)

    arg_parser.add_argument('-o', '--output',
                            help='Output C++ header file',
                            required=True)

    arg_parser.add_argument('-s', '--project-source-dir',
                            help='Project source directory',
                            default=".")

    arg_parser.add_argument('-v', '--verbose',
                            help='Print out generated wrapping code',
                            action='store_true')

    args = arg_parser.parse_args()

    if len(args.include_dirs) == 0:
        args.include_dirs.append(os.path.getdirname(args.input))

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

    with open(args.output, 'w') as f:
        stream = stream_with_line_breaks(f)
        wrapped_objects.update(
            parse_file(args.input, args.project_source_dir, args.include_dirs,
                       args.declaration_name, stream))

    print(wrapped_objects)
