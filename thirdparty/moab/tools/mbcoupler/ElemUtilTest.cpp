#include "TestUtil.hpp"
#include "ElemUtil.hpp"
#include <iostream>

using namespace moab;

void test_hex_nat_coords();

int main()
{
  int rval = 0;
  rval += RUN_TEST(test_hex_nat_coords);
  return rval;
}

const CartVect cube_corners[8] = { CartVect( 0, 0, 0 ),
                                     CartVect( 1, 0, 0 ),
                                     CartVect( 1, 1, 0 ),
                                     CartVect( 0, 1, 0 ),
                                     CartVect( 0, 0, 1 ),
                                     CartVect( 1, 0, 1 ),
                                     CartVect( 1, 1, 1 ),
                                     CartVect( 0, 1, 1 ) };
                                    

const CartVect hex_corners[8] = { CartVect( 1.0e0, 0.0e0, 0.0e0 ),
                                    CartVect( 1.0e0, 1.0e0, 0.3e0 ),
                                    CartVect( 0.0e0, 2.0e0, 0.6e0 ),
                                    CartVect( 0.2e0, 1.1e0, 0.4e0 ),
                                    CartVect( 1.5e0, 0.3e0, 1.0e0 ),
                                    CartVect( 1.5e0, 1.3e0, 1.0e0 ),
                                    CartVect( 0.5e0, 2.3e0, 1.0e0 ),
                                    CartVect( 0.7e0, 1.4e0, 1.0e0 ) };

/** shape function for trilinear hex */
CartVect hex_map( const CartVect& xi, const CartVect* corners )
{
  CartVect x(0.0);
  x += (1 - xi[0]) * (1 - xi[1]) * (1 - xi[2]) * corners[0];
  x += (1 + xi[0]) * (1 - xi[1]) * (1 - xi[2]) * corners[1];
  x += (1 + xi[0]) * (1 + xi[1]) * (1 - xi[2]) * corners[2];
  x += (1 - xi[0]) * (1 + xi[1]) * (1 - xi[2]) * corners[3];
  x += (1 - xi[0]) * (1 - xi[1]) * (1 + xi[2]) * corners[4];
  x += (1 + xi[0]) * (1 - xi[1]) * (1 + xi[2]) * corners[5];
  x += (1 + xi[0]) * (1 + xi[1]) * (1 + xi[2]) * corners[6];
  x += (1 - xi[0]) * (1 + xi[1]) * (1 + xi[2]) * corners[7];
  return x *= 0.125;
}

static void hex_bounding_box( const CartVect* corners, CartVect& min, CartVect& max  )
{
  min = max = corners[0];
  for (unsigned i = 1; i < 8; ++i)
    for (unsigned d = 0; d < 3; ++d)
      if (corners[i][d] < min[d])
        min[d] = corners[i][d];
      else if (corners[i][d] > max[d])
        max[d] = corners[i][d];
}

static bool in_range( const CartVect& xi )
  { return fabs(xi[0]) <= 1 
        && fabs(xi[1]) <= 1 
        && fabs(xi[2]) <= 1; 
  }        

void test_hex_nat_coords()
{
  CartVect xi, result_xi;
  bool valid;
  // rename EPS to EPS1 because of conflict with definition of EPS in types.h
  // types.h is now included in the header.
  const double EPS1 = 1e-6;
  
    // first test with cube because it's easier to debug failures
  std::vector<CartVect> cube_corners2;
  std::copy(cube_corners, cube_corners+8, std::back_inserter(cube_corners2));
  Element::LinearHex hex(cube_corners2);
  for (xi[0] = -1; xi[0] <= 1; xi[0] += 0.2) {
    for (xi[1] = -1; xi[1] <= 1; xi[1] += 0.2) {
      for (xi[2] = -1; xi[2] <= 1; xi[2] += 0.2) {
        const CartVect pt = hex_map(xi, cube_corners);
        result_xi = hex.ievaluate(pt, EPS1/10);
        double dum = EPS1/10;
        valid = hex.inside_nat_space(result_xi, dum);
        CHECK(valid);
        CHECK_REAL_EQUAL( xi[0], result_xi[0], EPS1 );
        CHECK_REAL_EQUAL( xi[1], result_xi[1], EPS1 );
        CHECK_REAL_EQUAL( xi[2], result_xi[2], EPS1 );
      }
    }
  }
  
    // now test with distorted hex
  std::vector<CartVect> hex_corners2;
  std::copy(hex_corners, hex_corners+8, std::back_inserter(hex_corners2));
  Element::LinearHex hex2(hex_corners2);
  for (xi[0] = -1; xi[0] <= 1; xi[0] += 0.2) {
    for (xi[1] = -1; xi[1] <= 1; xi[1] += 0.2) {
      for (xi[2] = -1; xi[2] <= 1; xi[2] += 0.2) {
        const CartVect pt = hex_map(xi, hex_corners);
        result_xi = hex2.ievaluate(pt, EPS1/10);
        double dum = EPS1/10;
        valid = hex2.inside_nat_space(result_xi, dum);
        CHECK(valid);
        CHECK_REAL_EQUAL( xi[0], result_xi[0], EPS1 );
        CHECK_REAL_EQUAL( xi[1], result_xi[1], EPS1 );
        CHECK_REAL_EQUAL( xi[2], result_xi[2], EPS1 );
      }
    }
  }
  
    // test points outside of element
  CartVect x, min, max;
  hex_bounding_box( cube_corners, min, max );
  for (x[0] = -1; x[0] <= 2; x[0] += 0.4) {
    for (x[1] = -1; x[1] <= 2; x[1] += 0.4) {
      for (x[2] = -1; x[2] <= 2; x[2] += 0.4) {
        bool in_box = x[0] >= min[0] && x[0] <= max[0] 
                   && x[1] >= min[1] && x[1] <= max[1]
                   && x[2] >= min[2] && x[2] <= max[2];
        if (in_box)
          continue;
        result_xi = hex.ievaluate(x, EPS1/10);
        double dum = EPS1/10;
        valid = hex.inside_nat_space(result_xi, dum);
        
//std::cout << (valid ? 'y' : 'n');
        CHECK(!valid || !in_range(result_xi));
      }
    }
  }
//std::cout << std::endl;

  hex_bounding_box( hex_corners, min, max );
  for (x[0] = -1; x[0] <= 3; x[0] += 0.5) {
    for (x[1] = -2; x[1] <= 4; x[1] += 0.5) {
      for (x[2] = -1; x[2] <= 2; x[2] += 0.4) {
        bool in_box = x[0] >= min[0] && x[0] <= max[0] 
                   && x[1] >= min[1] && x[1] <= max[1]
                   && x[2] >= min[2] && x[2] <= max[2];
        if (in_box)
          continue;
        try {
          result_xi = hex2.ievaluate(x, EPS1/10);
        }
        catch (Element::Map::EvaluationError err) {
          valid = false;
        }
//std::cout << (valid ? 'y' : 'n');
        CHECK(!valid || !in_range(result_xi));
      }
    }
  }
//std::cout << std::endl;
}
  
