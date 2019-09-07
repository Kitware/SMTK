//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_SelectionAction_h
#define smtk_view_SelectionAction_h

namespace smtk
{
namespace view
{

/// Descriptors for how lists passed to the selection should modify the selection-map.
enum class SelectionAction
{
  /**\brief Replace all the current selection and filter it.
    *
    * Example use case: normal selection from rendering window.
    */
  FILTERED_REPLACE = 0,
  /**\brief Replace all the current selection and do not filter it.
    *
    * Example use case: selection from model tree and attribute panel.
    */
  UNFILTERED_REPLACE = 1,
  /**\brief Filter the input selection and add it to the current selection.
    *
    * Example use case: addition from rendering window.
    */
  FILTERED_ADD = 2,
  /**\brief Do not filter the input selection, just add it to the current selection.
    *
    * Example use case: addition from operator dialog where a the user is
    * presented with a pre-filtered list.
    */
  UNFILTERED_ADD = 3,
  /**\brief Filter the input selection, then subtract it from the current selection.
    *
    * Example use case: substraction from rendering window.
    */
  FILTERED_SUBTRACT = 4,
  /**\brief Do not filter the input selection, just subtract it from the current selection.
    *
    * Example use case: subtraction from operator dialog where a pre-filtered list is provided.
    */
  UNFILTERED_SUBTRACT = 5,
  /**\brief Use the default SelectionAction provided by the Selection.
    *
    * Use it when SelectionAction should be decided by previous user input
    * that resulted in a mode being set on the Selection.
    *
    * An example is when a user presses a modifier key while in a selection mode
    * to switch between addition, subtraction, or replacement. In this case,
    * the key presses will change the default action on the selection
    * while the mouse clicks that identify entities signal their intent
    * to use the updated default with this enum.
    */
  DEFAULT = 6
};

} // namespace view
} // namespace smtk

#endif
