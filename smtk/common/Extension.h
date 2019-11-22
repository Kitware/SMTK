//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Extension_h
#define smtk_common_Extension_h
/*!\file */

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/common/CompilerInformation.h"

#ifdef SMTK_MSVC
// Ignore symbol exposure warnings for STL classes.
#pragma warning(disable : 4251)
#endif

#include <functional>

/**\brief Provide a way to force registration of operator extensions at load time.
  *
  * Invoke this macro in the implementation file of your Extension subclass.
  * Then, including "smtk/AutoInit.h" and invoking smtkAutoInit() in an
  * object file that is directly compiled into an executable target will force
  * registration of the extension even when dynamically linked to the extension
  * with a lazy link-loader.
  *
  * The macro takes 3 arguments:
  *
  * + a macro used to export a global function that will register the class;
  * + a **unique** name under which the extension should be exported (make this a
  *   valid variable name as it will be used as such);
  * + the **fully-qualified** name of your class (including all namespaces it is in).
  *
  * As an example, consider a subclass named AuxGeomFileTestExtension. Inside
  * the `AuxGeomFileTestExtension.cxx` file, we place: <pre>
  *   smtkDeclareExtension(
  *     SMTKVTKEXTENSIONS_EXPORT,
  *     aux_geom_file_test,
  *     AuxGeomFileTestExtension);
  * </pre>
  * Then, inside an executable, we can place: <pre>
  *   \#include "smtk/AutoInit.h"
  *   smtkAutoInit(aux_geom_file_test_extension);
  * </pre>
  * and be guaranteed that the extension will be registered.
  * Note that when you call the `smtkAutoInit()` macro, you should add `_extension` to the
  * name you passed to `smtkDeclareExtension()`.
  */
#define smtkDeclareExtension(exportmacro, name, cls)                                               \
  void exportmacro smtk_##name##_extension_AutoInit_Construct()                                    \
  {                                                                                                \
    smtk::common::Extension::registerExtension(#name,                                              \
      []() { return std::dynamic_pointer_cast<smtk::common::Extension>(cls::create()); },          \
      /* Never register class-static extension constructor as one-shot */ false);                  \
  }                                                                                                \
  void exportmacro smtk_##name##_extension_AutoInit_Destruct()                                     \
  {                                                                                                \
    smtk::common::Extension::unregisterExtension(#name);                                           \
  }

namespace smtk
{
namespace common
{

/**\brief Allow extension of operator functionality in separate libraries.
  *
  * Extension subclasses allow an operator to present additional functionality
  * when additional libraries are available beyond those required by the one containing
  * the operator itself.
  *
  * An example from ModelBuilder would be extending the auxiliary geometry operator
  * to test loading files using VTK without the base operator depending on VTK.
  * The base operator would call Extension::find() to discover whether any
  * extensions were available for its particular instance. If so, then it could
  * call methods on the extension to have it test files for compatibility.
  * In this way, the base operator can be available even when VTK is not, but provide
  * better feedback when VTK is present.
  *
  * Extensions are registered with a unique name. If you wish to find all extensions
  * (or the first extension) of a given type, use the visit() method rather than findAs().
  *
  * Extensions may be one-shot (i.e., unregistered after they are used) or permanent
  * (i.e., unregistered only when the executable exits or dynamic library is unloaded).
  * Registrations using the smtkDeclareExtension() macro are always permanent
  */
class SMTKCORE_EXPORT Extension : smtkEnableSharedPtr(Extension)
{
public:
  smtkTypeMacroBase(Extension);
  virtual ~Extension();

  /**\brief Register an extension with a unique name.
    *
    * Any operator that asks for this name will be given the result of calling the passed function.
    * If \a oneShot is true (the default), then the extension will be unregistered the first
    * time it is used.
    *
    * Note that the find method is templated, so the function may produce an instance of an
    * extension which is immediately discarded because it cannot be cast to the type requested
    * by the called of find().
    */
  static bool registerExtension(
    const std::string& name, std::function<Extension::Ptr(void)> ctor, bool oneShot = true);

  /// Remove an extension from the registry.
  static bool unregisterExtension(const std::string& name);

  /**\brief Call the given function on each registered extension.
    *
    * The \a visitor function is passed the name of the extension and an instance of it.
    * The \a visitor must return a pair of booleans: the first indicates whether or
    * not the instance was used (so that one-shot extensions are unregistered properly).
    * The second boolean indicates whether visitAll() should terminate early without
    * invoking \a visitor on any more extensions.
    */
  static void visitAll(
    std::function<std::pair<bool, bool>(const std::string&, Extension::Ptr)> visitor);

  /**\brief Iterate over all the extensions that are subclasses of the given type
    *
    * \sa visitAll for more information on how \a visitor is used.
    */
  template <typename T>
  static void visit(std::function<std::pair<bool, bool>(const std::string&, T)> visitor)
  {
    T result;
    Extension::visitAll([&result, visitor](const std::string& name, Extension::Ptr entry) {
      result = smtk::dynamic_pointer_cast<typename T::element_type>(entry);
      if (!result)
      {
        return std::make_pair(false, false);
      }
      return visitor(name, result);
    });
  }

  /// Find an extension given a specific name.
  static Extension::Ptr find(const std::string& name, bool removeOneShot = true);

  /// Find the first extension with a given \a name and type.
  template <typename T>
  static typename T::Ptr findAs(const std::string& name)
  {
    typename T::Ptr result = std::dynamic_pointer_cast<T>(Extension::find(name, false));
    return result;
  }

protected:
  Extension();
};
}
}

#endif
