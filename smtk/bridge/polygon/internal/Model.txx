#ifndef __smtk_bridge_polygon_internal_model_txx
#define __smtk_bridge_polygon_internal_model_txx

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

template<typename T>
Point model::projectPoint(T coordBegin, T coordEnd)
{
  double xyz[3] = {0, 0, 0};
  int i = 0;
  // Translate to origin
  for (T c = coordBegin; c != coordEnd && i < 3; ++i, ++c)
    {
    xyz[i] = *c - this->m_origin[i];
    }
  // Assume any unspecified coordinates are 0 and finish translating to origin
  for (; i < 3; ++i)
    {
    xyz[i] = - this->m_origin[i];
    }
  // Project translated point to x and y axes
  double px = 0, py = 0;
  for (i = 0; i < 3; ++i)
    {
    px += xyz[i] * this->m_xAxis[i];
    py += xyz[i] * this->m_yAxis[i];
    }
  // Scale point and round to integer
  Point result(px * this->m_scale, py * this->m_scale);
  return result;
}

template<typename T>
void model::liftPoint(const Point& ix, T coordBegin)
{
  T coord = coordBegin;
  for (int i = 0; i < 3; ++i, ++coord)
    {
    *coord =
      this->m_origin[i] +
      ix.x() * this->m_iAxis[i] +
      ix.y() * this->m_jAxis[i];
    }
}

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk
#endif // __smtk_bridge_polygon_internal_model_txx
