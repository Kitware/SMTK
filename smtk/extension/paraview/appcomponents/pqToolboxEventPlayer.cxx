//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqToolboxEventPlayer.h"

#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtOperationTypeView.h"

#include "smtk/string/Token.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QtDebug>

#include "pqEventDispatcher.h"

pqToolboxEventPlayer::pqToolboxEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

bool pqToolboxEventPlayer::playEvent(
  QObject* Object,
  const QString& Command,
  const QString& Arguments,
  bool& Error)
{
  using namespace smtk::string::literals;
  auto* view = qobject_cast<qtOperationTypeView*>(Object);
  if (view)
  {
    QAbstractItemModel* baseModel = view->model();
    qtOperationTypeModel* model = nullptr;
    while (baseModel && !model)
    {
      auto* sfpm = qobject_cast<QSortFilterProxyModel*>(baseModel);
      if (sfpm)
      {
        baseModel = sfpm->sourceModel();
        model = qobject_cast<qtOperationTypeModel*>(baseModel);
      }
      else
      {
        break;
      }
    }
    if (model)
    {
      smtk::string::Token cmd = Command.toStdString();
      if (cmd.id() == "runOperationWithDefaults"_hash || cmd.id() == "editOperationParameters"_hash)
      {
        QRegularExpression eventRegex("\\(([^,]*),([^,]*)\\)");
        auto eventMatch = eventRegex.match(Arguments);
        if (eventMatch.hasMatch())
        {
          // We ignore cap(1) for now since the operation index is
          // a platform-dependent hash code.
          QVariant v = eventMatch.captured(2);
          std::string opTypeName = v.toString().toStdString();
          auto opTypeIndex = model->typeIndexFromTypeName(opTypeName);
          if (opTypeIndex)
          {
            switch (cmd.id())
            {
              default: // fall through
              case "editOperationParameters"_hash:
                model->requestOperationParameterEdits(opTypeIndex);
                break;
              case "runOperationWithDefaults"_hash:
                model->runOperationWithDefaults(opTypeIndex);
                break;
            }
          }
        }
        else
        {
          Error = true;
        }
        return true;
      }
    }
  }
  return this->Superclass::playEvent(Object, Command, Arguments, Error);
}
