#include "rplib/fluidmix.h"

#include "nrlib/exception/exception.hpp"

#include <numeric>

FluidMix::FluidMix(const std::vector<Fluid*>      & fluid,
                   const std::vector<double>      & volume_fraction,
                   const std::vector<double>      & u,
                   DEMTools::MixMethod              mix_method)
: Fluid()
{
  u_ = u; // u contains independent samples used in quantiles of volume_fraction. Use RMISSING for fraction to be calculated from other fractions

  // Deep copy of fluid:
  fluid_.resize(fluid.size());
  for (size_t i = 0; i < fluid.size(); ++i) {
    fluid_[i] = fluid[i]->Clone();
  }

  volume_fraction_ = volume_fraction;
  mix_method_      = mix_method;

  k_ = rho_ = 0;

  if (fluid.size() != volume_fraction.size())
    throw NRLib::Exception("Invalid arguments:Number of properties are different from number of volume fractions.");
  else if (std::accumulate(volume_fraction.begin(), volume_fraction.end(), 0.0) > 1.0) //NBNB fjellvoll possible to give warning to user and then rescale
    throw NRLib::Exception("Invalid arguments:Sum of volume fractions > 1.0");
  else {
    std::vector<double> k(fluid.size()), rho(fluid.size());
    for (size_t i = 0; i < fluid.size(); i++)
      fluid[i]->GetElasticParams(k[i], rho[i]);

    switch (mix_method_) {
      case DEMTools::Hill :
        k_    = DEMTools::CalcEffectiveElasticModuliUsingHill(k, volume_fraction);
        break;
      case DEMTools::Reuss :
        k_    = DEMTools::CalcEffectiveElasticModuliUsingReuss(k, volume_fraction);     // homogeneous
        break;
      case DEMTools::Voigt :
        k_    = DEMTools::CalcEffectiveElasticModuliUsingVoigt(k, volume_fraction);
        break;
      default :
        throw NRLib::Exception("Invalid fluid mixing algorithm specified.");
    }
    rho_  = DEMTools::CalcEffectiveDensity(rho, volume_fraction);
  }
}

FluidMix::~FluidMix()
{
  for (size_t i = 0; i < fluid_.size(); ++i)
    delete fluid_[i];
}

Fluid *
FluidMix::Clone() const
{
  // Provide base class variables.
  FluidMix * s = new FluidMix(*this);

  // Provide variables specific to FluidMix.
  s->volume_fraction_ = this->volume_fraction_;
  s->mix_method_      = this->mix_method_;

  size_t n_fluids = this->fluid_.size();
  s->fluid_.resize(n_fluids);
  for (size_t i = 0; i < n_fluids; ++i)
    s->fluid_[i] = this->fluid_[i]->Clone();

  return s;
}

FluidMix& FluidMix::operator=(const FluidMix& rhs)
{
  if (this != &rhs) {
    Fluid::operator=(rhs);

    volume_fraction_ = rhs.volume_fraction_;
    mix_method_      = rhs.mix_method_;

    size_t n_fluids_old = fluid_.size();
    for (size_t i = 0; i < n_fluids_old; ++i)
      delete fluid_[i];

    size_t n_fluids = rhs.fluid_.size();
    fluid_.resize(n_fluids);
    for (size_t i = 0; i < n_fluids; ++i)
      fluid_[i] = rhs.fluid_[i]->Clone();
  }
  return *this;
}


