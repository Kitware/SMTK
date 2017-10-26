//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_common_Generator_h
#define __smtk_common_Generator_h

#include "smtk/common/CompilerInformation.h"

#include <cassert>
#include <iostream>
#include <set>

namespace smtk
{
namespace common
{

/// Given an input type <Input> and a desired output type <Output>, the
/// following class templates describe a class Generator<Input, Output> for
/// generating an instance of the Output type given the Input type. The output
/// type must have a default constructor; this value will be returned when no
/// conditions for generation have been satisfied. Here is an example of an
/// implementation that accepts std::string and outputs type Foo:
///
/// (GenerateFoo_1.h)
///
/// class GenerateFoo_1 : public GeneratorType<std::string, Foo, GenerateFoo_1>
/// {
/// public:
///   bool valid(const std::string&) const override;
///   Foo operator()(const std::string&) override;
/// };
///
/// (GenerateFoo_1.cxx)
///
/// namespace
/// {
/// static bool registered_Foo_1 = GenerateFoo_1::registerClass();
/// }
///
/// bool GenerateFoo_1::valid(const std::string& input) const
/// {
///   return <Logic to decide if input1 is useable by GenerateFoo_1>
/// }
///
/// Foo GenerateFoo_1::operator()(const std::string& input)
/// {
///   return <a generated Foo>
/// }
///
/// (GenerateFoo.h)
///
/// #ifndef EXPORT
/// extern
/// #endif
/// template class Generator<std::string, Foo>;
///
/// class GenerateFoo : public Generator<std::string, Foo>
/// {
/// public:
///   virtual ~GenerateFoo();
/// };
///
/// (GenerateFoo.cxx)
///
/// template class Generator<std::string, Foo>;
///
/// GenerateFoo::~GenerateFoo()
/// {
/// }
///
/// (Implementation.cxx)
///
/// ...
/// std::string foo_string = <input for GenerateFoo_1>;
/// GenerateFoo generateFoo;
/// Foo foo = generateFoo(foo_string);
/// ...
///
/// NOTE: if different instances of GeneratorType are described across
/// compilation units, care must be taken to (a) explicitly declare
/// Generator<Input, Output> in the compilation unit that contains the class
/// that is derived from Generator<Input, Output>, and (b) extern declare it
/// in subsequent compilation units that contain the derived instances of
/// GeneratorType.

/// Base for all generators. Describes the two methods used for generator
/// selection and object generation.
template <class Input, class Output>
class GeneratorBase
{
public:
  virtual ~GeneratorBase() {}

  /// A discriminating method to determine whether or not the input should be
  /// considered for use as input by a generator type. If this method returns
  /// false, the generator type will not be considered. If this method returns
  /// true, the class will be considered for generation but may still fail (see
  /// the description of the function call operator).
  virtual bool valid(const Input&) const = 0;

  /// Function call operator for generating Output-s. Implementations of this
  /// method may throw std::exception-s, facilitating the use of valid() as a
  /// quick check for input validity without necessarily guaranteeing the
  /// class's success.
  virtual Output operator()(const Input&) = 0;
};

/// Interface generator class. Implements the base methods valid() and the
/// function call operator as a loop over the registered GeneratorType-s. Also
/// contains the static set of generator types.
template <class Input, class Output>
class Generator : public GeneratorBase<Input, Output>
{
  template <class U, class V, class W>
  friend class GeneratorType;

public:
  /// Loop over registered generators and return true if any of the generators'
  /// valid() calls return true; otherwise, return false.
  bool valid(const Input&) const override;

  /// Loop over registered generators and return an Output instance from the
  /// first generator (a) whose valid() call returns true, and (b) that
  /// successfully creates an
  /// instance of Output (without throwing).
  Output operator()(const Input&) override;

protected:
  static std::set<GeneratorBase<Input, Output>*>& generators();
};

template <class Input, class Output>
std::set<GeneratorBase<Input, Output>*>& Generator<Input, Output>::generators()
{
  static std::set<GeneratorBase<Input, Output>*> generators;
  return generators;
}

template <class Input, class Output>
bool Generator<Input, Output>::valid(const Input& input) const
{
  for (auto gen : Generator<Input, Output>::generators())
  {
    if (gen->valid(input))
    {
      return true;
    }
  }
  return false;
}

template <class Input, class Output>
Output Generator<Input, Output>::operator()(const Input& input)
{
  Output output;
  for (auto gen : Generator<Input, Output>::generators())
  {
    if (!gen->valid(input))
    {
      continue;
    }

    try
    {
      output = (*gen)(input);
      break;
    }
    catch (std::exception& e)
    {
#ifndef NDEBUG
      std::cerr << "Exception caught: " << e.what() << std::endl;
#else
      (void)e;
#endif
    }
  }
  return output;
}

/// Base class for specific generator types. Uses CRTP to simplify the process
/// of registration to the interface class.
template <class Input, class Output, class Self>
class GeneratorType : public GeneratorBase<Input, Output>
{
  friend Self;

public:
  static bool registerClass();

private:
  static bool s_registered;
  GeneratorType() {}
};

template <class Input, class Output, class Self>
bool GeneratorType<Input, Output, Self>::registerClass()
{
  static bool registered = false;
  if (!registered)
  {
    Generator<Input, Output>::generators().insert(new Self());
    registered = true;
  }
  return registered;
}

template <class Input, class Output, class Self>
bool GeneratorType<Input, Output, Self>::s_registered =
  GeneratorType<Input, Output, Self>::registerClass();
}
}

#endif
