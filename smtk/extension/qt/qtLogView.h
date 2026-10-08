//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_qtLogView_h
#define smtk_qt_qtLogView_h

#include "smtk/extension/qt/Exports.h"

#include <QDialog>

#include <filesystem>
#include <fstream>

class QDialogButtonBox;
class QPlainTextEdit;
class QTimer;

namespace smtk
{
namespace qt
{

/**\brief A dialog to show the contents of a log file as it is being written.
  *
  */
class SMTKQTEXT_EXPORT qtLogView : public QDialog
{
  Q_OBJECT
public:
  qtLogView(const std::filesystem::path& logPath, QWidget* parent = nullptr);
  ~qtLogView() override = default;

  QSize sizeHint() const override;

public Q_SLOTS:
  void readMore();
  void done();

protected:
  std::filesystem::path m_path;
  std::streamoff m_lastRead{ 0 };
  std::ifstream m_file;
  QPlainTextEdit* m_contents;
  QDialogButtonBox* m_buttonBox{ nullptr };
  QTimer* m_updateTimer{ nullptr };
};

} // namespace qt
} // namespace smtk

#endif // smtk_qt_qtLogView_h
