#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

Tessellation::Tessellation()
{
}

int Tessellation::addCoords(double* a)
{
  std::vector<double>::size_type ipt = this->m_coords.size();
  for (int i = 0; i < 3; ++i)
    {
    this->m_coords.push_back(a[i]);
    }
  return static_cast<int>(ipt / 3);
}

Tessellation& Tessellation::addCoords(double x, double y, double z)
{
  this->m_coords.push_back(x);
  this->m_coords.push_back(y);
  this->m_coords.push_back(z);
  return *this;
}

Tessellation& Tessellation::addPoint(double* a)
{
  int ai = this->addCoords(a);
  // Uncomment the below to use the modified three.js viewer.
  // Leave commented to make the VTK sources work.
  /*
  std::vector<int> pconn;
  pconn.push_back(512); // Extension of three.js file format for "Vertex" glyph
  pconn.push_back(ai);
  this->m_conn.push_back(ai);
  */
  return *this;
}

Tessellation& Tessellation::addLine(double* a, double* b)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(double* a, double* b, double* c)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  int ci = this->addCoords(c);
  this->m_conn.push_back(0); // A triangle in three.js format.
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::addPoint(int ai)
{
  this->m_conn.push_back(512);
  this->m_conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::addLine(int ai, int bi)
{
  this->m_conn.push_back(2);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(int ai, int bi, int ci)
{
  this->m_conn.push_back(0);
  this->m_conn.push_back(ai);
  this->m_conn.push_back(bi);
  this->m_conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::reset()
{
  this->m_conn.clear();
  this->m_coords.clear();
  return *this;
}

  } // model namespace
} // smtk namespace
