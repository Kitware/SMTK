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
#include <memory>
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
  /// method may throw exceptions, facilitating the use of valid() as a
  /// quick check for input validity without necessarily guaranteeing the
  /// class's success.
  virtual Output operator()(const Input&) = 0;
};

/// Interface generator class. Implements the base methods valid() and the
/// function call operator as a loop over the registered GeneratorTypes. Also
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
  /// Even though the set of generators is static, we cannot guarantee its
  /// existence across compilation units due to our plugin-based architecture.
  /// We therefore use a weak pointer to guard ourselves against the unlikely
  /// event of a generator implementation outliving its base generator class.
  static std::weak_ptr<std::set<GeneratorBase<Input, Output>*> > generators();
};

template <class Input, class Output>
std::weak_ptr<std::set<GeneratorBase<Input, Output>*> > Generator<Input, Output>::generators()
{
  static std::shared_ptr<std::set<GeneratorBase<Input, Output>*> > generators =
    std::make_shared<std::set<GeneratorBase<Input, Output>*> >(
      std::set<GeneratorBase<Input, Output>*>());
  return generators;
}

template <class Input, class Output>
bool Generator<Input, Output>::valid(const Input& input) const
{
  auto gens = Generator<Input, Output>::generators().lock();
  if (gens == nullptr)
  {
    return false;
  }
  for (auto gen : *gens)
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
  auto gens = Generator<Input, Output>::generators().lock();
  if (gens == nullptr)
  {
    return output;
  }
  for (auto gen : *gens)
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
  /// The Registry is needed to scope the lifetime of the generator type to the
  /// library that defines it. This prevents the base generator from attempting
  /// to call an instance of this generator type after the library defining it
  /// has been removed.
  class Registry
  {
    friend GeneratorType;

    Registry(std::weak_ptr<std::set<GeneratorBase<Input, Output>*> > generators)
      : m_generators(generators)
      , m_self(new Self())
    {
      if (auto gens = m_generators.lock())
      {
        gens->insert(m_self);
      }
    }
    ~Registry()
    {
      if (auto gens = m_generators.lock())
      {
        gens->erase(m_self);
      }
      delete m_self;
    }
    std::weak_ptr<std::set<GeneratorBase<Input, Output>*> > m_generators;
    Self* m_self;
  };

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
    static Registry registry(Generator<Input, Output>::generators());
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
