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

DescriptivePhrases DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return this->m_subphrases;
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
