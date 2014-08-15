#include <boost/function.hpp>
#include <boost/bind.hpp>

struct favorite_int {
  int operator() () { return 42; }
};

struct conditional_int {
  int operator() (float x) { return 2 * static_cast<int>(x); }
};

int main(int, char**)
{
  boost::function<int()> f;
  f = favorite_int();
  boost::function<int()> g;
  g = boost::bind<float>(conditional_int(), 21.0);
  return (f() == 42 && g() == 42 ) ? 0 : 1;
}
