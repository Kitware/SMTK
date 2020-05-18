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
#include <memory>
#include <set>

#define DEBUG_GENERATOR 0

#if !defined(NDEBUG) && DEBUG_GENERATOR
#include <iostream>
#endif

namespace smtk
{
namespace common
{

///@file Generator.h @brief Generator templates.
///
/// Given an input type `<Input>` and a desired output type `<Output>`, the
/// following class templates describe a class `Generator<Input, Output>` for
/// generating an instance of the Output type given the Input type. The output
/// type must have a default constructor; this value will be returned when no
/// conditions for generation have been satisfied. Here is an example of an
/// implementation that accepts std::string and outputs type Foo:
///
/// (RegisterGenerateFoo.h)
/// This class can exist external to smtk, like in a plugin (RGG, for example).
/// ```
/// class RegisterGenerateFoo : public GeneratorType<std::string, Foo, RegisterGenerateFoo>
/// {
/// public:
///   bool valid(const std::string&) const override;
///   Foo operator()(const std::string&) override;
/// };
/// ```
/// (RegisterGenerateFoo.cxx)
/// Note that `valid()` and `operator()` may be defined in the header, and
/// `registerClass()` may be called in another .cxx file, like smtk's `Registrar.cxx` files.
/// ```
/// namespace
/// {
/// static bool registered_Foo_1 = RegisterGenerateFoo::registerClass();
/// }
///
/// bool RegisterGenerateFoo::valid(const std::string& input) const
/// {
///   return <Logic to decide if input1 is useable by RegisterGenerateFoo>
/// }
///
/// Foo RegisterGenerateFoo::operator()(const std::string& input)
/// {
///   return <a generated Foo>
/// }
/// ```
/// (GenerateFoo.h)
/// Note: the macros used here are different and depend on the smtk module, here IOVTK
/// ```
/// #ifndef smtkIOVTK_EXPORTS
/// extern
/// #endif
/// template class SMTKIOVTK_EXPORT Generator<std::string, Foo>;
///
/// using GenerateFoo = Generator<std::string, Foo>;
/// ```
/// (GenerateFoo.cxx)
/// The .cxx must exist to generate the right symbols to link against.
/// ```
/// template class Generator<std::string, Foo>;
/// ```
/// (Implementation.cxx)
/// ```
/// ...
/// std::string foo_string = "<input for GenerateFoo>";
/// GenerateFoo generateFoo;
/// Foo foo = generateFoo(foo_string);
/// ...
/// ```

namespace detail
{
struct NullGeneratorBase
{
};
} // namespace detail

///@brief Base for all generators.
///
/// Describes the two methods used for generator
/// selection and object generation.
template<class Input, class Output, class Base = detail::NullGeneratorBase>
class GeneratorBase : public Base
{
public:
  template<typename... T>
  GeneratorBase(T&&... all)
    : Base(std::forward<T>(all)...)
  {
  }

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

///@brief Interface generator class.
///
/// Implements the base methods valid() and the
/// function call operator as a loop over the registered GeneratorTypes. Also
/// contains the static set of generator types.
template<class Input, class Output, class Base = detail::NullGeneratorBase>
class Generator : public GeneratorBase<Input, Output, Base>
{
  template<class U, class V, class W, class X>
  friend class GeneratorType;

public:
  template<typename... T>
  Generator(T&&... all)
    : GeneratorBase<Input, Output, Base>(std::forward<T>(all)...)
  {
  }

  /// Loop over registered generators and return true if any of the generators'
  /// valid() calls return true; otherwise, return false.
  bool valid(const Input&) const override;

  /// Loop over registered generators and return an \a Output instance from the
  /// first generator (a) whose valid() call returns true, and (b) that
  /// successfully creates an
  /// instance of \a Output (without throwing).
  Output operator()(const Input&) override;

protected:
  /// Even though the set of generators is static, we cannot guarantee its
  /// existence across compilation units due to our plugin-based architecture.
  /// We therefore use a weak pointer to guard ourselves against the unlikely
  /// event of a generator implementation outliving its base generator class.
  static std::weak_ptr<std::set<GeneratorBase<Input, Output, Base>*>> generators();
};

template<class Input, class Output, class Base>
std::weak_ptr<std::set<GeneratorBase<Input, Output, Base>*>>
Generator<Input, Output, Base>::generators()
{
  static std::shared_ptr<std::set<GeneratorBase<Input, Output, Base>*>> generators =
    std::make_shared<std::set<GeneratorBase<Input, Output, Base>*>>(
      std::set<GeneratorBase<Input, Output, Base>*>());
  return generators;
}

template<class Input, class Output, class Base>
bool Generator<Input, Output, Base>::valid(const Input& input) const
{
  auto gens = Generator<Input, Output, Base>::generators().lock();
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

template<class Input, class Output, class Base>
Output Generator<Input, Output, Base>::operator()(const Input& input)
{
  Output output;
  auto gens = Generator<Input, Output, Base>::generators().lock();
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
#if !defined(NDEBUG) && DEBUG_GENERATOR
      std::cerr << "Exception caught: " << e.what() << std::endl;
#else
      (void)e;
#endif
    }
  }
  return output;
}

/// @brief Base class for specific generator types.
///
/// Uses CRTP to simplify the process of registration to the interface class.
template<class Input, class Output, class Self, class Base = detail::NullGeneratorBase>
class GeneratorType : public GeneratorBase<Input, Output, Base>
{
  /// The Registry is needed to scope the lifetime of the generator type to the
  /// library that defines it. This prevents the base generator from attempting
  /// to call an instance of this generator type after the library defining it
  /// has been removed.
  class Registry
  {
    friend GeneratorType;

    Registry(std::weak_ptr<std::set<GeneratorBase<Input, Output, Base>*>> generators)
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
    std::weak_ptr<std::set<GeneratorBase<Input, Output, Base>*>> m_generators;
    Self* m_self;
  };

  friend Self;

public:
  static bool registerClass();

protected:
  template<typename... T>
  GeneratorType(T&&... all)
    : GeneratorBase<Input, Output, Base>(std::forward<T>(all)...)
  {
  }

private:
  static bool s_registered;
};

template<class Input, class Output, class Self, class Base>
bool GeneratorType<Input, Output, Self, Base>::registerClass()
{
  static bool registered = false;
  if (!registered)
  {
    static Registry registry(Generator<Input, Output, Base>::generators());
    registered = true;
  }
  return registered;
}

template<class Input, class Output, class Self, class Base>
bool GeneratorType<Input, Output, Self, Base>::s_registered =
  GeneratorType<Input, Output, Self, Base>::registerClass();
} // namespace common
} // namespace smtk

#endif
