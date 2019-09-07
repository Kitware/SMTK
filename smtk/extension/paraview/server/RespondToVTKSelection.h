//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_RespondToVTKSelection_h
#define smtk_view_RespondToVTKSelection_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/server/Exports.h" // For export macro
#include "smtk/operation/XMLOperation.h"

#include "smtk/view/Selection.h"

class vtkSelection;
class vtkMultiBlockDataSet;

namespace smtk
{
namespace view
{

/**\brief A base class for operations that should be invoked when
  *       the VTK selection changes.
  *
  * Instances of subclasses of this operation are automatically populated
  * by the current selection object inside ParaView by the vtkSMTKEncodeSelection
  * class created as selection occurs.
  *
  * The selection translator behavior will invoke subclass instances
  * that have been registered to the VTKSelectionResponderGroup until
  * one returns success.
  */
class SMTKPVSERVEREXT_EXPORT RespondToVTKSelection : public smtk::operation::XMLOperation
{
public:
  using Result = smtk::operation::Operation::Result;
  smtkTypeMacro(RespondToVTKSelection);
  smtkCreateMacro(RespondToVTKSelection);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  virtual ~RespondToVTKSelection();

  /**\brief Set/get the multiblock data holding per-component tessellation info.
    *
    * This is set by the vtkSMTKWrapper before the operation is invoked.
    */
  bool setVTKData(vtkMultiBlockDataSet* seln);
  vtkMultiBlockDataSet* vtkData() const { return m_vtkData; }

  /**\brief Set/get the VTK data object holding the user-requested selection parameters.
    *
    * This is set by the vtkSMTKWrapper before the operation is invoked.
    */
  bool setVTKSelection(vtkSelection* seln);
  ::vtkSelection* vtkSelection() const { return m_vtkSelection; }

  /**\brief Set/get the SMTK selection that is the destination/target of the translated VTK selection.
    *
    * This is set by the vtkSMTKWrapper before the operation is invoked.
    */
  bool setSMTKSelection(const smtk::view::SelectionPtr& seln);
  smtk::view::SelectionPtr smtkSelection() const;

  /**\brief Set/get the name used as the "source" from which the VTK selection originated.
    *
    * This defaults to "paraview" but will usually be overwritten by whatever the
    * vtkSMTKWrapper is configured to use.
    */
  bool setSMTKSelectionSource(const std::string& sourceName);
  std::string smtkSelectionSource() const { return m_smtkSelectionSource; }

  /**\brief Set/get the SMTK selection bits to be set for each item of the VTK selection.
    *
    * This defaults to 1 but will usually be overwritten by whatever the
    * vtkSMTKWrapper is configured to use.
    * Note that a value of 0 is not accepted; at least one bit must be set.
    */
  bool setSMTKSelectionValue(int value);
  int smtkSelectionValue() const { return m_smtkSelectionValue; }

  /**\brief Set/get the selection modifier indicated by the user.
    *
    * The default 0 is to replace the current selection.
    * Other values are: addition (1), subtraction (2), and
    * toggle (3). See ParaView's pqView.h header for values.
    */
  bool setModifier(int modifier);
  int modifier() const { return m_modifier; }

  /// A convenience to return SMTK's equivalent of modifier()'s value.
  smtk::view::SelectionAction modifierAction() const;

  /**\brief Set/get the selection block-mode.
    *
    * When true, the user is selecting blocks, which indicates
    * that SMTK should select persistent objects (usu. components)
    * rather than subsets.
    *
    * The value returned by setSelectingBlocks() indicates whether
    * the value was changed rather than the new or old value of
    * the internal variable itself; call selectingBlocks() to
    * discover the internal variable value.
    */
  bool setSelectingBlocks(bool shouldSelectBlocks);
  bool selectingBlocks() const { return m_selectingBlocks; }

protected:
  RespondToVTKSelection();

  /**\brief A convenience method that subclasses may use internally
    *       to handle VTK block selections.
    *
    * This will modify the SMTK selection to match the VTK selection
    * should any selected blocks correspond to SMTK components.
    *
    * Note that the selection may be filtered.
    */
  bool transcribeBlockSelection();

  /// By default, only handle block selections by calling transcribeBlockSelection().
  Result operateInternal() override;
  /// Fail or succeed quietly.
  void generateSummary(Operation::Result&) override{};

  ::vtkSelection* m_vtkSelection;
  vtkMultiBlockDataSet* m_vtkData;
  smtk::view::WeakSelectionPtr m_smtkSelection;
  std::string m_smtkSelectionSource;
  int m_smtkSelectionValue;
  int m_modifier;
  bool m_selectingBlocks;

private:
  const char* xmlDescription() const override;
};

} //namespace view
} // namespace smtk

#endif
#endif // smtk_view_RespondToVTKSelection_h
