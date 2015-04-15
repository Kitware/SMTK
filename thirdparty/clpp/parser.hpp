// C++ Command line parameters parser.
//
// Copyright (C) Denis Shevchenko, 2010.
// shev.denis @ gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// See http://www.opensource.org/licenses/mit-license.php
//
// http://clp-parser.sourceforge.net/

/////////////////////////////////////////////////////////////////////////////////////////
///
/// \mainpage CLPP
///
/// \section about About
///
/// This document describes <b>'Command line parameters parser'</b> library. Library provides parsing
/// of command line parameters, with callback corresponding functions and, if necessary, checking
/// of parameter's values semantic.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section download Download
///
/// You can download CLPP source code from \htmlonly<b><a href="http://sourceforge.net/projects/clp-parser">SourceForge</a></b>\endhtmlonly.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section license License
///
/// This library is licensed under \htmlonly<b><a href="http://www.opensource.org/licenses/mit-license.php">MIT License</a></b>\endhtmlonly.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section motivation Motivation
/// \htmlonly <p style="text-align: right"><strong><em>"Don't reinvent the wheel"</em></strong></p> \endhtmlonly
///
/// Almost every "console-start" program requires input of some command line parameters, so it handling
/// is very common task. Actually, this handling add up to two tasks:
/// \li check correctness (in all senses of this word) of inputed parameters,
/// \li some reaction(s) on it.
///
/// CLPP library provides general solution for this tasks. It very easy to use.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section hello-world-example "Hello world" example
///
/// Simplest example of CLPP using:
///
/// \code
/// #include <clpp/parser.hpp>
///
/// #include <iostream>
///
/// void help() {
///     std::cout << "My program, version 1.0. Common usage is..." << std::endl;
///     //
/// }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     // Register parameter, without value and without any checking...
///     parser.add_parameter( "-h", "--help", help );
///     try {
///         parser.parse( argc, argv );
///         // If user inputs '-h' or '--help' in command line, function 'help()' will be called.
///     } catch ( const std::exception& exc ) {
///         std::cerr << exc.what() << std::endl;
///     }
///
///     return 0;
/// }
/// \endcode
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section features Features
///
/// \li Header-only (does not require build).
/// \li Register parameters with two (in Unix-style) or single names.
/// \li Calls correspond functions, with or without argument.
/// \li Register unnamed parameters that can be inputed without names.
/// \li Provides common checks of inputed parameters, like duplication, incorrect, etc.
/// \li Checking of value's type, like integer or real (defines by type of registered function's argument).
/// \li Checking of value's semantic, like correct path, etc.
/// \li Define parameter's necessity.
/// \li Define parameter's default value.
/// \li Define another "name-value" separator.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section changes Backward incompatible changes
///
/// Since <b>1.0rc</b> version function <tt>check_type()</tt> has been removed (it no need anymore).
/// See <b>User's guide</b> for more info.
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \section acknowledgements Acknowledgements
///
/// I'm very grateful to \htmlonly<b><a href="http://www2.research.att.com/~bs/">Bjarne Stroustrup</a></b>\endhtmlonly, who invented so beautiful, so powerful and so dangerous language!
///
/// Also I grateful to all authors of \htmlonly<b><a href="http://www.boost.org/">Boost C++ libraries</a></b>\endhtmlonly. Thank you for your work!
///
/// \htmlonly <br/> \endhtmlonly
///
///
///
/// \htmlonly <em>Copyright &copy; Denis Shevchenko, 2010</em> \endhtmlonly
///
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \page dependencies External dependencies
///
/// There is only "one" dependency - \htmlonly<b><a href="http://www.boost.org/">Boost C++ libraries</a></b>\endhtmlonly.
///
/// Strictly speacking, CLPP use following libraries:
/// \li <b>Boost.Any</b>
/// \li <b>Boost.Assign</b>
/// \li <b>Boost.Utility</b>
/// \li <b>Boost.Ptr_container</b>
/// \li <b>Boost.Lexical_cast</b>
/// \li <b>Boost.Algorithm</b>
/// \li <b>Boost.Bind</b>
/// \li <b>Boost.Foreach</b>
/// \li <b>Boost.Asio</b>
/// \li <b>Boost.Smart_ptr</b>
/// \li <b>Boost.Function</b>
/// \li <b>Boost.Filesystem</b>
/// \li <b>Boost.System</b>
///
/// Full list of Boost C++ libraries see \htmlonly<b><a href="http://www.boost.org/doc/libs">there</a></b>\endhtmlonly.
///
/// All used libraries are <b>header-only</b>, except <em>Boost.Filesystem</em> and <em>Boost.System</em>,
/// so you must build these libraries and link it with your program.
///
/// However, if you using C++ professionally, you (in my humble opinion) <em><b>must</b></em> have
/// Boost C++ libraries. So just download full package from \htmlonly<b><a href="http://www.boost.org/users/download/">there</a></b>\endhtmlonly, install it and enjoy!
///
/// Note: If you use Windows - see http://www.boostpro.com/download.
///
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \page user_guide User's guide
///
/// <b>Contents:</b>
///     - \ref introduction
///         - \ref value_or_not_value
///     - \ref necessity
///     - \ref default_val
///         - \ref error_reports
///     - \ref common_usage
///     - \ref preparing
///     - \ref defining_parameter
///     - \ref member_functions
///     - \ref parsing
///         - \ref function_arg_types
///     - \ref advanced_usage
///         - \ref how_to_define_necessity
///         - \ref how_to_define_default_value
///     - \ref how_to_define_type_check
///     - \ref how_to_define_semantic_check
///         - \ref combine_of_settings
///         - \ref another_value_separator
///         - \ref unnamed_params
///
/// \htmlonly <hr/> \endhtmlonly
///
/// \section introduction Introduction
///
/// Factually, using CLPP library add up to only two tasks:
/// \li <b>registration</b> set of parameters, with definition all required checkings and characteristics,
/// \li <b>parsing</b> inputed parameter(s), with defined check(s) and calls of  corresponding functions.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection value_or_not_value Value or not value?
///
/// Command line parameter can be with or without value.
///
/// Example of parameter without value:
/// \code
/// # ./program --help
/// \endcode
/// Parameter <b>'--help'</b> is useful of its own accord, it not need any value.
///
/// Example of parameter with value:
/// \code
/// # ./program --log-dir=/some/path
/// \endcode
/// Parameter <b>'--log-dir'</b>, in contrast, useless without value.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection necessity Necessity
///
/// Command line parameter can be necessary or optionally.
///
/// In examples above parameter <b>'--help'</b> is optionally, because it may missing
/// (only in cases where user want to see help info, he input '--help').
///
/// But parameter <b>'--log-dir'</b> <em>may be</em> necessary, in this case user <em>must</em> input it.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection default_val Default value
///
/// Command line parameter can have default value, in this case not required input it.
/// This option can be useful for parameters with predefined default values.
///
/// In example above parameter <b>'--log-dir'</b> may have default value of path, so user can skip it.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection error_reports Error reports
///
/// All reports about errors begins with <b>[CLPP]</b> prefix, for example:
///
/// \code
/// [CLPP] You inputs 3 parameters, but only 2 registered!
/// \endcode
///
/// \htmlonly <hr/> \endhtmlonly
///
/// \section common_usage Common usage
///
/// This section describes common usage of CLPP.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection preparing Preparing
///
/// Copy 'clpp' folder in some place where your compiler is looking for header files and add:
///
/// \code #include <clpp/parser.hpp> \endcode
///
/// in your program.
///
/// For simplicity, you can also add:
///
/// \code using namespace clpp; \endcode
///
/// Note: In old versions of library used namespace <b>clp_parser</b>, but backward compatibility is maintained.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection defining_parameter Defining command line parameter
///
/// Registration of new parameter included three tasks:
/// \li define parameter's name (short and full names, or single name),
/// \li define function that will be called if corresponding parameter will be inputed,
/// \li define checks and default value for parameter <em>(optionally)</em>.
///
/// Use <b>clpp::command_line_parameters_parser::add_parameter()</b> function for it.
///
/// \code
///
/// void help() { /* some info */ }
/// void config() { /* some config info */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     // Register parameter with two names (in Unix-style) and 'help()' function.
///     parser.add_parameter( "-h", "--help", help );
///     // Register parameter with single name and 'config()' function.
///     parser.add_parameter( "--config", config );
///     // ...
/// }
/// \endcode
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection member_functions Register of functions-members
///
/// You can register not only global functions, but also functions-members.
///
/// \code
/// struct some_parameters_storage {
///     void some_path( const std::string& path ) { /* Some work with path... */ }
///     void some_num( double number ) { /* Some work with number... */ }
/// }
///
/// int main( int argc, char* argv[] ) {
///     some_parameters_storage storage;
///
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-f", "--file", &storage, &some_parameters_storage::some_path );
///     parser.add_parameter( "-n", "--number", &storage, &some_parameters_storage::some_num );
///     // ...
/// }
/// \endcode
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection parsing Parsing
///
/// For parsing use <b>clpp::command_line_parameters_parser::parse()</b> function.
///
/// \code
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     // ...
///     try {
///         parser.parse( argc, argv );
///     } catch ( const std::exception& exc ) {
///         std::cerr << exc.what() << std::endl;
///     }
///     // ...
/// }
/// \endcode
///
/// This function parse all inputed command line parameters,
/// checks correctness and calls corresponding functions.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection function_arg_types Supported types of user's function argument
///
/// When you register parameter with value, you can use follow types of function's argument:
/// \li almost all standard C++-types (see below),
/// \li std::string.
///
/// You CANNOT use <b>const char*</b> and <b>wchar_t</b> argument.
/// But this limitation, in my humble opinion, is not the real problem.
///
/// You can pass argument by value, like this:
/// \code
/// void f( int i ) { /* some work... */ }
/// \endcode
/// or by const reference:
/// \code
/// void f( const std::string& path ) { /* some work... */ }
/// \endcode
///
/// Passing by non-const reference is NOT supported (I think this is completely unnecessary).
///
/// \htmlonly <hr/> \endhtmlonly
///
/// \section advanced_usage Advanced usage
///
/// This section describes advanced usage of CLPP.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection how_to_define_necessity Define parameter's necessity
///
/// For define parameter's necessity use <b>clpp::parameter::necessary()</b> function:
/// \code
///
/// void config( const std::string& path ) { /* some config info */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-c", "--config", config )
///           .necessary();
///     // ...
/// }
/// \endcode
/// After that user <b>must</b> inputs this parameter in command line.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection how_to_define_default_value Define parameter's default value
///
/// For set parameter's default value use <b>clpp::parameter::default_value()</b> function:
/// \code
///
/// void config( const std::string& path ) { /* some config info */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-c", "--config", config )
///           .default_value( "/some/default/path" );
///     // ...
/// }
/// \endcode
/// After that user <b>can skip</b> this parameter in command line.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection how_to_define_type_check Parameter's value type check
///
/// Since <b>1.0rc</b> version value's type checks automatically. If you register callback function
/// \code
/// void some_num( double num ) { /* some work... */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-n", "--number", some_num );
///     // ...
/// }
/// \endcode
/// so <tt>number</tt>'s value <b>must</b> be 'double' type.
///
/// Note: if you want use function with 'float' argument, like this:
///
/// \code
/// void some_num( float num ) { /* some work... */ }
/// \endcode
/// AND use 'default_value()' function, you must indicate exactly 'float' type, like this:
///
/// \code
///     // ...
///     parser.add_parameter( "-n", "--number", some_num )
///           .default_value( 12.56f )
///           ;
///     // ...
/// \endcode
/// or like this:
///
/// \code
///     // ...
///     parser.add_parameter( "-n", "--number", some_num )
///           .default_value( float( 12.56 ) )
///           ;
///     // ...
/// \endcode
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection how_to_define_semantic_check Parameter's value semantic check
///
/// Use <b>clpp::parameter::check_semantic()</b> function.
///
/// \code
/// void log_dir( const std::string& path_to_log ) { /* some work... */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-l", "--log-dir", log_dir )
///           .check_semantic( clpp::path );
///     // ...
/// }
/// \endcode
///
/// In this case, value of <b>'--log-dir'</b> <em>must</em> be valid path in current filesystem.
///
/// Supported value's semantic:
/// \li <b>path</b> (check of path correctness, in current filesystem),
/// \li <b>ipv4</b> (check of IP-address validity, exactly IPv4),
/// \li <b>ipv6</b> (check of IP-address validity, exactly IPv6),
/// \li <b>ip</b> (check of IP-address validity, IPv4 or IPv6).
/// \li <b>email</b> (check of E-mail validity).
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection combine_of_settings Combine of settings
///
/// Of course, you can combine settings for one parameter, like this:
///
/// \code
/// void log_dir( const std::string& path_to_log ) { /* some work... */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-l", "--log-dir", log_dir )
///           .default_value( "/some/default/path" )
///           .check_semantic( clpp::path )
///           ;
///     // ...
/// }
/// \endcode
///
/// <b>Note:</b> You cannot combine <em>contradictory</em> settings. For example, if you
/// write like this:
///
/// \code
/// void log_dir( const std::string& path_to_log ) { /* some work... */ }
///
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-l", "--log-dir", log_dir )
///           .default_value( "/some/default/path" )
///           .check_semantic( clpp::path )
///           .necessary() // Necessary parameter with defined default value?? Hmm...
///           ;
///     // ...
/// }
/// \endcode
///
/// exception will throw.
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection another_value_separator Another 'name-value' separator
///
/// You can define another 'name-value' separator, instead default <b>'='</b>.
///
/// Use <b>clpp::command_line_parameters_parser::set_value_separator()</b> function.
///
/// \code
/// int main( int argc, char* argv[] ) {
///     clpp::command_line_parameters_parser parser;
///     parser.set_value_separator( ':' );
///     // ...
/// }
/// \endcode
///
/// In this case, parameters with value <b>must</b> be input in command line like this:
///
/// \code
/// # ./program --log-dir:/some/path
/// \endcode
///
/// Note: You CANNOT use characters with ASCII-code less than 0x21.
/// They are unsuitable candidates for the separator role. :-)
///
/// \htmlonly <br/> \endhtmlonly
///
/// \subsection unnamed_params Unnamed parameters
///
/// Sometimes you may want to use some parameters without explicitly inputed names.
///
/// For example, if you develop network program with console-defined host and port,
/// you can use it like this:
///
/// \code
/// # ./program --host=127.0.0.1 --port=80
/// \endcode
///
/// but you probably want to use it easier:
///
/// \code
/// # ./program 127.0.0.1 80
/// \endcode
///
/// Use <b>clpp::parameter::order()</b> function.
///
/// This function sets order number for parameter, so in our example <b>'host'</b> has
/// order number 1, and <b>'port'</b> has order number 2. Note that order number begins
/// with 1, because it is not <em>index</em>, but exactly number.
///
/// Of course, order number cannot be negative.
///
/// So you must register these parameters like this:
///
/// \code
/// struct my_server {
///     void host( const std::string& address ) { /* some work... */ }
///     void port( unsigned int some_port ) { /* some work... */ }
/// };
///
/// int main( int argc, char* argv[] ) {
///     my_server server;
///
///     clpp::command_line_parameters_parser parser;
///     parser.add_parameter( "-h", "--host", &server, &my_server::host )
///           .order( 1 )
///           ;
///     parser.add_parameter( "-p", "--port", &server, &my_server::port )
///           .order( 2 )
///           ;
///     // ...
/// }
/// \endcode
///
/// Now you can use it like this:
///
/// \code
/// # ./program 127.0.0.1 80
/// \endcode
///
/// but <b>not</b> vice versa:
///
/// \code
/// # ./program 80 127.0.0.1
/// \endcode
///
/// because in this case <b>my_server::port()</b> function get string-argument, and exception will throw.
///
/// Remember that order numbers must be unique:
///
/// \code
///     // ...
///     parser.add_parameter( "-h", "--host", &server, &my_server::host )
///           .order( 1 )
///           ;
///     parser.add_parameter( "-p", "--port", &server, &my_server::port )
///           .order( 1 ) // You inputed already used order?? Hmm...
///           ;
///     // ...
/// \endcode
///
/// Of course, you can use "ordered" parameters with names, in any combination:
///
/// \code
/// # ./program -h=127.0.0.1 --port=80
/// \endcode
/// \code
/// # ./program 127.0.0.1 -p=80
/// \endcode
///
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef CLPP_PARSER_HPP
#define CLPP_PARSER_HPP

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wunused-private-field"
  #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
  #pragma GCC diagnostic ignored "-Wshadow"
#endif
#include "detail/checkers/all.hpp"
#include "detail/user_functions_caller.hpp"
#include "detail/unnamed_parameters_handler.hpp"
#include "detail/parameter.hpp"
#include "detail/misc.hpp"

#include <boost/noncopyable.hpp>
#include <boost/assign.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

using namespace boost::assign;

/// \class command_line_parameters_parser
/// \brief Parser.
///
/// Presents parsing functionality.
class command_line_parameters_parser : boost::noncopyable {
    typedef detail::common_checker< parameters, detail::parameter_parts_extractor, std::string > checker;
    typedef boost::ptr_vector< checker >    common_checkers;
    typedef common_checkers::iterator       checker_it;
    typedef common_checkers::const_iterator checker_const_it;
public:
    command_line_parameters_parser() :
        name_value_separator( "=" ) {}
public:
    parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , void ( *fn )() ) {
        return create_parameter( short_name, full_name, fn );
    }

  template< typename ArgType >
    parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , void ( *fn )( const ArgType& ) ) {
        return create_parameter( short_name, full_name, fn );
    }

  template< typename ArgType >
    parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , void ( *fn )( ArgType ) ) {
        return create_parameter( short_name, full_name, fn );
    }

    parameter& add_parameter( const std::string& single_name
                              , void ( *fn )() ) {
        return create_parameter( single_name, "", fn );
    }

  template< typename ArgType >
    parameter& add_parameter( const std::string& single_name
                              , void ( *fn )( const ArgType& ) ) {
        return create_parameter( single_name, "", fn );
    }

  template< typename ArgType >
    parameter& add_parameter( const std::string& single_name
                              , void ( *fn )( ArgType ) ) {
        return create_parameter( single_name, "", fn );
    }

    template< typename ObjectType >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )() ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template< typename ObjectType >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )() const ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( const ArgType& ) ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( const ArgType& ) const ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( ArgType ) ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& short_name
                           , const std::string& full_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( ArgType ) const ) {
        return create_parameter( short_name, full_name, obj, fn );
    }

    template< typename ObjectType >
  parameter& add_parameter( const std::string& single_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )() ) {
        return create_parameter( single_name, "", obj, fn );
    }

    template< typename ObjectType >
  parameter& add_parameter( const std::string& single_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )() const ) {
        return create_parameter( single_name, "", obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& single_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( ArgType ) ) {
        return create_parameter( single_name, "", obj, fn );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
  parameter& add_parameter( const std::string& single_name
                           , ObjectType* obj
                           , void ( ObjectType::*fn )( ArgType ) const ) {
        return create_parameter( single_name, "", obj, fn );
    }
private:
    parameters registered_parameters;
private:
    template< typename PtrToFun >
    parameter& create_parameter( const std::string&  short_name
                                , const std::string& full_name
                                , const PtrToFun&    ptr_to_fun ) {
        check_null_ptr( ptr_to_fun, short_name );
        check_names_validity( short_name, full_name );

        registered_parameters += parameter( short_name, full_name, ptr_to_fun );
        return registered_parameters.back();
    }

    template
    <
        typename PtrToObj
        , typename PtrToObjFun
    >
    parameter& create_parameter( const std::string&  short_name
                                , const std::string& full_name
                                , const PtrToObj&    ptr_to_obj
                                , const PtrToObjFun& ptr_to_fun ) {
        check_null_ptr( ptr_to_obj, short_name );
        check_null_ptr( ptr_to_fun, short_name );
        check_names_validity( short_name, full_name );

        registered_parameters += parameter( short_name, full_name, ptr_to_obj, ptr_to_fun );
        return registered_parameters.back();
    }
private:
    template< typename Ptr >
    void check_null_ptr( const Ptr& ptr, const std::string& parameter_name ) const {
        if ( 0 == ptr ) {
            const std::string what_happened =
                "Zero pointer (function/object/method) detected for parameter '" + parameter_name + "'!";
            throw std::invalid_argument( what_happened );
        } else {}
    }

    void check_names_validity( const std::string& short_name, const std::string& full_name ) const {
        check_spaces_existence_in( short_name );
        check_spaces_existence_in( full_name );
        check_equality_of( short_name, full_name );
        check_uniqueness_of( short_name );
        if (!full_name.empty())
          check_uniqueness_of( full_name );
    }

    void check_spaces_existence_in( const std::string& name ) const {
        if ( boost::contains( name, " " ) ) {
            const std::string what_happened =
                    "Invalid parameter's name '" + name + "', it shouldn't contains space(s)!";
            throw std::runtime_error( what_happened );
        } else {}
    }

    void check_equality_of( const std::string& short_name, const std::string& full_name ) const {
      if ( short_name == full_name ) {
            const std::string what_happened = "Equal names of parameter: '"
                                              + short_name + "', '" + full_name + "'!";
            throw std::invalid_argument( what_happened );
        } else {}
    }

    void check_uniqueness_of( const std::string& name ) const {
        parameter_const_it it = std::find( registered_parameters.begin()
                                           , registered_parameters.end()
                                           , name );
        if ( registered_parameters.end() != it ) {
            const std::string what_happened = "Parameter with name '" + name + "' already exists!";
            throw std::invalid_argument( what_happened );
        } else {}
    }
private:
    std::string name_value_separator;
    common_checkers checkers;
    detail::parameter_parts_extractor_p extractor;
public:
    void set_value_separator( char separator ) {
        check_printable_of( separator );
        name_value_separator = separator;
    }
private:
    void check_printable_of( char separator ) const {
        const int ascii_code = separator;
        if ( ascii_code <= 0x20 ) {
            const std::string what_happened = "Symbol (ASCII-code is "
                                              + detail::to_str( separator )
                                              + ") is not suitable for name-value separator!";
            throw std::invalid_argument( what_happened );
        } else {}
    }
public:
    void parse( int argc, char** argv ) {
        detail::str_storage inputed_parameters = obtain_parameters_from( argc, argv );
        if ( there_is_nothing_to_parse( inputed_parameters ) ) {
            return;
        } else {}

        create_parameter_parts_extractor();
        create_parameters_checkers();
        base_checks( inputed_parameters );
        handle_unnamed_parameters( inputed_parameters );
        remaining_checks( inputed_parameters );
        call_corresponding_user_functions( inputed_parameters );
    }
private:
    detail::str_storage obtain_parameters_from( int argc, char** argv ) const {
        detail::str_storage inputed_parameters;
        for ( int i = 1; i < argc; ++i ) {
            inputed_parameters += argv[i];
        }
        return inputed_parameters;
    }

    bool there_is_nothing_to_parse( const detail::str_storage& inputed_parameters ) const {
        return inputed_parameters.empty()
               && no_parameters_with_default_value()
               && no_necessary_parameters();
    }

    bool no_parameters_with_default_value() const {
        parameter_const_it it = std::find_if( registered_parameters.begin()
                                              , registered_parameters.end()
                                              , boost::mem_fn( &parameter::has_default_value ) );
        return registered_parameters.end() == it;
    }

    bool no_necessary_parameters() const {
        parameter_const_it it = std::find_if( registered_parameters.begin()
                                              , registered_parameters.end()
                                              , boost::mem_fn( &parameter::it_is_necessary ) );
        return registered_parameters.end() == it;
    }

    void handle_unnamed_parameters( detail::str_storage& inputed_parameters ) {
        detail::unnamed_parameters_handler handle( registered_parameters
                                                   , *extractor
                                                   , name_value_separator );
        handle( inputed_parameters );
    }

    void create_parameter_parts_extractor() {
        extractor.reset( new detail::parameter_parts_extractor( name_value_separator ) );
    }

    void create_parameters_checkers() {
        using namespace detail;

        checkers.push_back(
                new parameters_existence_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new parameters_redundancy_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new parameters_repetition_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new incorrect_parameters_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new necessary_parameters_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new parameters_values_checker( registered_parameters, *extractor, name_value_separator ) );
        checkers.push_back(
                new values_semantic_checker( registered_parameters, *extractor, name_value_separator ) );
    }

    void base_checks( const detail::str_storage& inputed_parameters ) {
        const size_t base_checkers_quantity = 3;
        for ( checker_const_it it = checkers.begin();
              checkers.begin() + base_checkers_quantity != it;
              ++it ) {
            it->check( inputed_parameters );
        }
        checkers.erase( checkers.begin(), checkers.begin() + base_checkers_quantity );
    }

    void remaining_checks( const detail::str_storage& inputed_parameters ) {
        for ( checker_const_it it = checkers.begin(); checkers.end() != it; ++it ) {
            it->check( inputed_parameters );
        }
    }

    void call_corresponding_user_functions( const detail::str_storage& inputed_parameters ) {
        detail::user_functions_caller caller( registered_parameters, *extractor, name_value_separator );
        caller.call( inputed_parameters );
    }
};

/// For backward compatibility with old versions.
typedef command_line_parameters_parser command_line_parameter_parser;

} // namespace clpp

/// For backward compatibility with old versions.
namespace clp_parser = clpp;

#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#endif // CLPP_PARSER_HPP
