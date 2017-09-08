/* Copyright 2007-2009 Brad King, Chuck Stewart
   Distributed under the Boost Software License, Version 1.0.
   (See accompanying file rtvl_license_1_0.txt or copy at
   http://www.boost.org/LICENSE_1_0.txt) */
#ifndef rtvl_weight_smooth_hxx
#define rtvl_weight_smooth_hxx

#include "rtvl_weight.hxx"

template <unsigned int N>
class rtvl_terms;

template <unsigned int N>
class rtvl_weight_smooth : public rtvl_weight<N>
{
public:
  typedef rtvl_weight<N> derived;
  rtvl_weight_smooth(double gs = 1, unsigned int n = 4);

protected:
  double compute_flat(rtvl_terms<N> const& terms) override;
  void compute_flat_d(rtvl_terms<N> const& terms, vnl_vector_fixed<double, N>& dwflat) override;
  double compute_curved(rtvl_terms<N> const& terms) override;
  void compute_curved_d(rtvl_terms<N> const& terms, vnl_vector_fixed<double, N>& dwcurve) override;

private:
  using derived::geodesic_scale;
  double z;
  double zm3p3;
  double zm3p4;
  double wangle;
  double cos_theta_2nm1;
  double (*intpow)(double);
  unsigned int cos_power;
};

#endif
