#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

DescriptivePhrase::DescriptivePhrase()
  : m_type(INVALID_DESCRIPTION), m_subphrasesBuilt(false)
{
}

DescriptivePhrasePtr DescriptivePhrase::setup(DescriptivePhraseType ptype, Ptr parnt)
{
  this->m_parent = parnt;
  this->m_type = ptype;
  this->m_subphrasesBuilt = false;
  return shared_from_this();
}

DescriptivePhrases& DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return this->m_subphrases;
}

/// Return the index of the given phrase in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(DescriptivePhrase* child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
    {
    if (it->get() == child)
      return i;
    }
  return -1;
}

void DescriptivePhrase::buildSubphrases()
{
  if (!this->m_subphrasesBuilt)
    {
    this->m_subphrasesBuilt = true;
    this->buildSubphrasesInternal();
    }
}

  } // model namespace
} // smtk namespace
