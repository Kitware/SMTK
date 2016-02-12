#ifndef __smtk_extension_cumulus_job_h
#define __smtk_extension_cumulus_job_h

#include <QtCore/QString>
#include <QtCore/QMetaType>

namespace cumulus
{

class Job
{
public:
  Job();
  Job(const QString &id, const QString &name, const QString &status);
  Job(const Job &job);

  ~Job();
  QString id() const { return this->m_id; };
  QString name() const { return this->m_name; };
  QString status() const { return this->m_status; };

private:
  QString m_id;
  QString m_name;
  QString m_status;

};

}; // end namespace

Q_DECLARE_METATYPE(cumulus::Job)

#endif
