//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/PythonInterpreter.h"
#include "smtk/common/CompilerInformation.h"
#include "smtk/common/smtkPythonPath.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif
SMTK_THIRDPARTY_POST_INCLUDE

namespace
{
std::string python_path_list = std::string(smtkPythonPath);
}

namespace smtk
{
namespace common
{
PythonInterpreter PythonInterpreter::m_instance;

PythonInterpreter& PythonInterpreter::instance()
{
  return PythonInterpreter::m_instance;
}

PythonInterpreter::PythonInterpreter()
{
  this->initialize();
}

PythonInterpreter::~PythonInterpreter()
{
  this->finalize();
}

bool PythonInterpreter::isInitialized() const
{
  return Py_IsInitialized() != 0;
}

void PythonInterpreter::initialize()
{
  if (!this->isInitialized())
  {
    pybind11::initialize_interpreter();

    // Add the contents of our python path list to the path
    pybind11::module sys = pybind11::module::import("sys");
    regex re(",");
    sregex_token_iterator it(python_path_list.begin(), python_path_list.end(), re, -1), last;
    for (; it != last; ++it)
    {
      sys.attr("path").attr("append")(it->str().c_str());
    }
  }
}

void PythonInterpreter::finalize()
{
  if (this->isInitialized())
  {
    pybind11::finalize_interpreter();
  }
}
}
}
