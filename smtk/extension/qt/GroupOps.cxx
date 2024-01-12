//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/GroupOps.h"

#include "smtk/extension/qt/Exports.h"

#include "smtk/operation/GroupOps.h"

#include "smtk/attribute/VoidItem.h"

#include "smtk/common/CompilerInformation.h"

#include <QMessageBox>
#include <QString>

namespace smtk
{
namespace extension
{

/// A function to resolve issues when deleting persistent objects.
///
/// Pass this function to smtk::operation::deleteObjects(); it will
/// prompt users if some subset of the input objects cannot be
/// deleted.
bool qtDeleterDisposition(smtk::operation::DispositionBatch& batch)
{
  QMessageBox msgBox;
  msgBox.setInformativeText(
    QString("<b>Proceed to delete %1 object(s)?</b><br><br>").arg(batch.numberOfObjects));
  QString msg;
  if (batch.numberOfBlockedOperations > 0)
  {
    msg += QString("Some objects have dependents; delete dependents?<br><br>");
    //.arg(batch.numberOfBlockedAssociations);
  }
  if (!batch.noMatch.empty())
  {
    msg += QString("%1 object(s) have no matching deleter. Skip them?<br><br>")
             .arg(batch.noMatch.size());
  }
  if (!batch.cannotAssociate.empty())
  {
    msg += QString("%1 object(s) could not be associated to a deleter. Skip them?<br><br>")
             .arg(batch.cannotAssociate.size());
  }
  msgBox.setText(msg);
  auto buttons = QMessageBox::Yes | QMessageBox::No;
  msgBox.setStandardButtons(buttons);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Yes && batch.numberOfBlockedOperations > 0)
  {
    // Look for any operations with a "delete dependents" option and enable it.
    for (const auto& entry : batch.ops)
    {
      if (
        !entry.second->ableToOperate() && entry.second->parameters()->findVoid("delete dependents"))
      {
        entry.second->parameters()->findVoid("delete dependents")->setIsEnabled(true);
      }
    }
  }
  return ret == QMessageBox::Yes;
}

} // namespace extension
} // namespace smtk
