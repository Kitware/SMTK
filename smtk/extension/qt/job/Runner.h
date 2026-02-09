//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_job_Runner_h
#define smtk_qt_job_Runner_h

#include "smtk/common/Managers.h"
#include "smtk/extension/qt/Exports.h" // For export macro.

#include <QObject>

#include <memory>

namespace smtk
{
namespace qt
{
namespace job
{

///\brief Runner inspects operation results for job specifications; if they
///       are present, it starts the jobs according to the specification.
///
/// It also provides methods to accept and report job status; and to terminate
/// running or scheduled jobs.
///
/// This class depends on Qt for process and filesystem monitoring.
class SMTKQTEXT_EXPORT Runner : public QObject
{
  Q_OBJECT
public:
  smtkSuperclassMacro(QObject);
  smtkTypeMacroBase(smtk::qt::job::Runner);

  Runner(const std::shared_ptr<smtk::common::Managers>& applicationContext);
  ~Runner() override;

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_Runner_h
