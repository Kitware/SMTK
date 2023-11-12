//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Gallery_h
#define smtk_task_Gallery_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/task/Worklet.h"

#include <map>
#include <string>

namespace smtk
{
namespace task
{

class Manager;

/**\brief Gallery represents a map of tasks worklets.
  *
  * It provides APIs to manage the set of worklets as well as
  * providing the ability to change a worklet's name
  */
class SMTKCORE_EXPORT Gallery
{
public:
  Gallery(Manager* manager);
  ~Gallery() = default;
  /**\brief Adds a Task Worklet to a Gallery
   *
   * If makeUnique is set to false and there is already a Worklet
   * with the same name, the Worklet will not be added and the method will
   * return false.  If makeUnique is true and there is another Worklet with
   * the same name as the one being added, then a new unique name will be created based
   * on the original suffixed by a number and the instance's unique name separator.
   */
  bool add(const Worklet::Ptr& worklet, bool makeUnique = false);

  /**\brief Remove a worklet from the gallery.
   *
   * Returns true if the worklet was removed and false if nothing was removed.
   */
  bool remove(const Worklet::Ptr& worklet);

  /**\brief Find a worklet in the gallery by its name.
   */
  Worklet::Ptr find(const std::string& name) const;

  /**\brief Returns a constant map of all of the worklets in the gallery.
   */
  const std::unordered_map<smtk::string::Token, Worklet::Ptr>& worklets() const
  {
    return m_worklets;
  }

  /**\brief Rename an existing worklet
   *
   * This will fail if the worklet is not part of the gallery or if the
   * new name is in use.
   */
  bool rename(const Worklet::Ptr& worklet, const std::string& newName);

  /**\brief Helper method to create a new name based on a provided /a name
   *
   * If the provided name is not already in use in the gallery it is returned as is
   * Else an unique name is created using the name provided followed by the gallery's
   * unique name separator and an integer.
   */
  std::string createUniqueName(const std::string& name) const;

  /** Set the unique name separator for the gallery
   */
  void setUniqueNameSeparator(const std::string& newSep) { m_uniqueNameSeparator = newSep; }

protected:
  Manager* m_manager;
  std::unordered_map<smtk::string::Token, Worklet::Ptr> m_worklets;
  std::string m_uniqueNameSeparator = "-";
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Gallery_h
