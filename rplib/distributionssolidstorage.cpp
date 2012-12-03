#include "nrlib/trend/trendstorage.hpp"
#include "nrlib/trend/trend.hpp"
#include "nrlib/grid/grid2d.hpp"

#include "rplib/distributionwithtrend.h"
#include "rplib/deltadistributionwithtrend.h"
#include "rplib/distributionssolid.h"
#include "rplib/distributionssolidstorage.h"
#include "rplib/distributionssolidtabulated.h"
#include "rplib/distributionssoliddem.h"
#include "rplib/distributionssolidmix.h"
#include "rplib/distributionwithtrendstorage.h"
#include "rplib/distributionsstoragekit.h"


DistributionsSolidStorage::DistributionsSolidStorage()
{
}

DistributionsSolidStorage::~DistributionsSolidStorage()
{
}

std::vector<DistributionsSolid *>
DistributionsSolidStorage::CreateDistributionsSolidMix(const int                                                       & n_vintages,
                                                       const std::string                                               & path,
                                                       const std::vector<std::string>                                  & trend_cube_parameters,
                                                       const std::vector<std::vector<double> >                         & trend_cube_sampling,
                                                       const std::map<std::string, DistributionsSolidStorage *>        & model_solid_storage,
                                                       const std::vector<std::string>                                  & constituent_label,
                                                       const std::vector<std::vector<DistributionWithTrendStorage *> > & constituent_volume_fraction,
                                                       DEMTools::MixMethod                                               mix_method,
                                                       std::string                                                     & errTxt) const
{

  int n_constituents = static_cast<int>(constituent_label.size());

  std::vector<int> n_vintages_constit(n_constituents);
  for(int i=0; i<n_constituents; i++)
    n_vintages_constit[i] = static_cast<int>(constituent_volume_fraction[i].size());

  std::vector<double> alpha(n_constituents);
  for(int i=0; i<n_constituents; i++) {
    if(constituent_volume_fraction[i][0] != NULL)
      alpha[i] = constituent_volume_fraction[i][0]->GetOneYearCorrelation();
    else
      alpha[i] = 1;
  }

  std::vector<std::vector<DistributionsSolid *> > distr_solid(n_vintages);
  for(int i=0; i<n_vintages; i++)
    distr_solid[i].resize(n_constituents, NULL);

  for (int s = 0; s < n_constituents; s++) {
    std::vector<DistributionsSolid *> distr_solid_all_vintages = ReadSolid(n_vintages,
                                                                           constituent_label[s],
                                                                           path,
                                                                           trend_cube_parameters,
                                                                           trend_cube_sampling,
                                                                           model_solid_storage,
                                                                           errTxt);

    for(int i=0; i<n_vintages; i++) {
      if(i < static_cast<int>(distr_solid_all_vintages.size()))
        distr_solid[i][s] = distr_solid_all_vintages[i];
      else
        distr_solid[i][s] = distr_solid[i-1][s]->Clone();
    }
  }

  std::vector<DistributionsSolid *>                  final_dist_solid(n_vintages, NULL);
  std::vector<std::vector<DistributionWithTrend *> > all_volume_fractions(n_vintages);

  for(int i=0; i<n_vintages; i++)
    all_volume_fractions[i].resize(n_constituents, NULL);

  for(int i=0; i<n_vintages; i++) {

    for (int s=0; s<n_constituents; s++) {

      if(i < n_vintages_constit[s]) {
        if(constituent_volume_fraction[s][i] != NULL)
          all_volume_fractions[i][s] = constituent_volume_fraction[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
      }
      else {
        if(all_volume_fractions[i-1][s] != NULL)
          all_volume_fractions[i][s] = all_volume_fractions[i-1][s]->Clone();
      }
    }

    CheckVolumeConsistency(all_volume_fractions[i], errTxt);

    if (errTxt == "")
      final_dist_solid[i] = new DistributionsSolidMix(distr_solid[i], all_volume_fractions[i], mix_method, alpha);
  }

  return(final_dist_solid);

}

//----------------------------------------------------------------------------------//
TabulatedVelocitySolidStorage::TabulatedVelocitySolidStorage(std::vector<DistributionWithTrendStorage *> vp,
                                                             std::vector<DistributionWithTrendStorage *> vs,
                                                             std::vector<DistributionWithTrendStorage *> density,
                                                             double                                      correlation_vp_vs,
                                                             double                                      correlation_vp_density,
                                                             double                                      correlation_vs_density)
: vp_(vp),
  vs_(vs),
  density_(density),
  correlation_vp_vs_(correlation_vp_vs),
  correlation_vp_density_(correlation_vp_density),
  correlation_vs_density_(correlation_vs_density)
{
}

TabulatedVelocitySolidStorage::~TabulatedVelocitySolidStorage()
{
  if(vp_[0]->GetIsShared() == false)
    delete vp_[0];
  if(vs_[0]->GetIsShared() == false)
    delete vs_[0];
  if(density_[0]->GetIsShared() == false)
    delete density_[0];
}

std::vector<DistributionsSolid *>
TabulatedVelocitySolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                                          const std::string                                         & path,
                                                          const std::vector<std::string>                            & trend_cube_parameters,
                                                          const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                          const std::map<std::string, DistributionsSolidStorage *>  & /*model_solid_storage*/,
                                                          std::string                                               & errTxt) const
{
  std::vector<double> alpha(3);
  alpha[0] = vp_[0]     ->GetOneYearCorrelation();
  alpha[1] = vs_[0]     ->GetOneYearCorrelation();
  alpha[2] = density_[0]->GetOneYearCorrelation();

  int n_vintages_vp      = static_cast<int>(vp_.size());
  int n_vintages_vs      = static_cast<int>(vs_.size());
  int n_vintages_density = static_cast<int>(density_.size());

  std::vector<DistributionsSolid *>    dist_solid(n_vintages, NULL);
  std::vector<DistributionWithTrend *> vp_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> vs_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> density_dist_with_trend(n_vintages, NULL);

  for(int i=0; i<n_vintages; i++) {
    if(i < n_vintages_vp)
      vp_dist_with_trend[i] = vp_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      vp_dist_with_trend[i] = vp_dist_with_trend[i-1]->Clone();

    if(i < n_vintages_vs)
      vs_dist_with_trend[i] = vs_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      vs_dist_with_trend[i] = vs_dist_with_trend[i-1]->Clone();

    if(i < n_vintages_density)
      density_dist_with_trend[i] = density_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      density_dist_with_trend[i] = density_dist_with_trend[i-1]->Clone();

    DistributionsSolid * solid = new DistributionsSolidTabulated(vp_dist_with_trend[i],
                                                                 vs_dist_with_trend[i],
                                                                 density_dist_with_trend[i],
                                                                 correlation_vp_vs_,
                                                                 correlation_vp_density_,
                                                                 correlation_vs_density_,
                                                                 DEMTools::Velocity,
                                                                 alpha);

    dist_solid[i] = solid;


  }

  return(dist_solid);

}

//----------------------------------------------------------------------------------//
TabulatedModulusSolidStorage::TabulatedModulusSolidStorage(std::vector<DistributionWithTrendStorage *> bulk_modulus,
                                                           std::vector<DistributionWithTrendStorage *> shear_modulus,
                                                           std::vector<DistributionWithTrendStorage *> density,
                                                           double                                      correlation_bulk_shear,
                                                           double                                      correlation_bulk_density,
                                                           double                                      correlation_shear_density)
: bulk_modulus_(bulk_modulus),
  shear_modulus_(shear_modulus),
  density_(density),
  correlation_bulk_shear_(correlation_bulk_shear),
  correlation_bulk_density_(correlation_bulk_density),
  correlation_shear_density_(correlation_shear_density)
{
}

TabulatedModulusSolidStorage::~TabulatedModulusSolidStorage()
{
  if(bulk_modulus_[0]->GetIsShared() == false)
    delete bulk_modulus_[0];
  if(shear_modulus_[0]->GetIsShared() == false)
    delete shear_modulus_[0];
  if(density_[0]->GetIsShared()== false)
    delete density_[0];
}

std::vector<DistributionsSolid *>
TabulatedModulusSolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                                         const std::string                                         & path,
                                                         const std::vector<std::string>                            & trend_cube_parameters,
                                                         const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                                         const std::map<std::string, DistributionsSolidStorage *>  & /*model_solid_storage*/,
                                                         std::string                                               & errTxt) const
{
  std::vector<double> alpha(3);
  alpha[0] = bulk_modulus_[0] ->GetOneYearCorrelation();
  alpha[1] = shear_modulus_[0]->GetOneYearCorrelation();
  alpha[2] = density_[0]      ->GetOneYearCorrelation();

  int n_vintages_bulk    = static_cast<int>(bulk_modulus_.size());
  int n_vintages_shear   = static_cast<int>(shear_modulus_.size());
  int n_vintages_density = static_cast<int>(density_.size());

  std::vector<DistributionsSolid *>    dist_solid(n_vintages, NULL);
  std::vector<DistributionWithTrend *> bulk_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> shear_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> density_dist_with_trend(n_vintages, NULL);

  for(int i=0; i<n_vintages; i++) {
    if(i < n_vintages_bulk)
      bulk_dist_with_trend[i] = bulk_modulus_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      bulk_dist_with_trend[i] = bulk_dist_with_trend[i-1]->Clone();

    if(i < n_vintages_shear)
      shear_dist_with_trend[i] = shear_modulus_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      shear_dist_with_trend[i] = shear_dist_with_trend[i-1]->Clone();

    if(i < n_vintages_density)
      density_dist_with_trend[i] = density_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
    else
      density_dist_with_trend[i] = density_dist_with_trend[i-1]->Clone();

    double lower_mega = 1.0e+5; //Finn grenser fra modelsettings
    double upper_mega = 1.0e+8;
    double test_bulk  = bulk_dist_with_trend[0]->ReSample(0,0);
    double test_shear = shear_dist_with_trend[0]->ReSample(0,0);
    if(test_bulk < lower_mega || test_bulk > upper_mega)
      errTxt += "Bulk modulus need to be given in megaPa\n";
    if(test_shear < lower_mega || test_shear > upper_mega)
      errTxt += "Shear modulus need to be given in megaPa\n";


    DistributionsSolid * solid = new DistributionsSolidTabulated(bulk_dist_with_trend[i],
                                                                 shear_dist_with_trend[i],
                                                                 density_dist_with_trend[i],
                                                                 correlation_bulk_shear_,
                                                                 correlation_bulk_density_,
                                                                 correlation_shear_density_,
                                                                 DEMTools::Modulus,
                                                                 alpha);

    dist_solid[i] = solid;
  }

  return(dist_solid);
}

//----------------------------------------------------------------------------------//
ReussSolidStorage::ReussSolidStorage(std::vector<std::string>                                  constituent_label,
                                     std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction)
{
}

ReussSolidStorage::~ReussSolidStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] && constituent_volume_fraction_[0][i]->GetIsShared() == false)
      delete constituent_volume_fraction_[0][i];
  }
}

std::vector<DistributionsSolid *>
ReussSolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                              const std::string                                         & path,
                                              const std::vector<std::string>                            & trend_cube_parameters,
                                              const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                              const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                              std::string                                               & errTxt) const
{
  std::vector<DistributionsSolid *> solid = CreateDistributionsSolidMix(n_vintages,
                                                                        path,
                                                                        trend_cube_parameters,
                                                                        trend_cube_sampling,
                                                                        model_solid_storage,
                                                                        constituent_label_,
                                                                        constituent_volume_fraction_,
                                                                        DEMTools::Reuss,
                                                                        errTxt);
  return(solid);
}

//----------------------------------------------------------------------------------//
VoigtSolidStorage::VoigtSolidStorage(std::vector<std::string >                                 constituent_label,
                                     std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction)
{
}

VoigtSolidStorage::~VoigtSolidStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] && constituent_volume_fraction_[0][i]->GetIsShared() == false)
      delete constituent_volume_fraction_[0][i];
  }
}

std::vector<DistributionsSolid *>
VoigtSolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                              const std::string                                         & path,
                                              const std::vector<std::string>                            & trend_cube_parameters,
                                              const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                              const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                              std::string                                               & errTxt) const
{
  std::vector<DistributionsSolid *> solid = CreateDistributionsSolidMix(n_vintages,
                                                                        path,
                                                                        trend_cube_parameters,
                                                                        trend_cube_sampling,
                                                                        model_solid_storage,
                                                                        constituent_label_,
                                                                        constituent_volume_fraction_,
                                                                        DEMTools::Voigt,
                                                                        errTxt);
  return(solid);
}

//----------------------------------------------------------------------------------//
HillSolidStorage::HillSolidStorage(std::vector<std::string>                                  constituent_label,
                                   std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction)
{
}

HillSolidStorage::~HillSolidStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] && constituent_volume_fraction_[0][i]->GetIsShared() == false)
      delete constituent_volume_fraction_[0][i];
  }
}

std::vector<DistributionsSolid *>
HillSolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                             const std::string                                         & path,
                                             const std::vector<std::string>                            & trend_cube_parameters,
                                             const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                             const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                             std::string                                               & errTxt) const
{
  std::vector<DistributionsSolid *> solid = CreateDistributionsSolidMix(n_vintages,
                                                                        path,
                                                                        trend_cube_parameters,
                                                                        trend_cube_sampling,
                                                                        model_solid_storage,
                                                                        constituent_label_,
                                                                        constituent_volume_fraction_,
                                                                        DEMTools::Hill,
                                                                        errTxt);
  return(solid);
}

//----------------------------------------------------------------------------------//

DEMSolidStorage::DEMSolidStorage(std::string                                               host_label,
                                 std::vector<DistributionWithTrendStorage *>               host_volume_fraction,
                                 std::vector<std::string>                                  inclusion_label,
                                 std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_volume_fraction,
                                 std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_aspect_ratio)
: host_label_(host_label),
  host_volume_fraction_(host_volume_fraction),
  inclusion_label_(inclusion_label),
  inclusion_volume_fraction_(inclusion_volume_fraction),
  inclusion_aspect_ratio_(inclusion_aspect_ratio)
{
}

DEMSolidStorage::~DEMSolidStorage()
{
  if(host_volume_fraction_[0]->GetIsShared() == false)
    delete host_volume_fraction_[0];

  for(int i=0; i<static_cast<int>(inclusion_volume_fraction_[0].size()); i++) {
    if(inclusion_volume_fraction_[0][i] && inclusion_volume_fraction_[0][i]->GetIsShared() == false)
      delete inclusion_volume_fraction_[0][i];
  }

  for(int i=0; i<static_cast<int>(inclusion_aspect_ratio_[0].size()); i++) {
    if(inclusion_aspect_ratio_[0][i] && inclusion_aspect_ratio_[0][i]->GetIsShared() == false)
      delete inclusion_aspect_ratio_[0][i];
  }
}

std::vector<DistributionsSolid *>
DEMSolidStorage::GenerateDistributionsSolid(const int                                                 & n_vintages,
                                            const std::string                                         & path,
                                            const std::vector<std::string>                            & trend_cube_parameters,
                                            const std::vector<std::vector<double> >                   & trend_cube_sampling,
                                            const std::map<std::string, DistributionsSolidStorage *>  & model_solid_storage,
                                            std::string                                               & errTxt) const
{
  // Remember: Host info is included first in constituent vectors
  int n_inclusions = static_cast<int>(inclusion_volume_fraction_.size());
  int n_constituents = n_inclusions + 1;

  std::vector<std::vector<DistributionWithTrendStorage *> > volume_fractions(n_constituents);
  volume_fractions[0] = host_volume_fraction_;

  for(int i=0; i<n_inclusions; i++)
    volume_fractions[i+1] = inclusion_volume_fraction_[i];

  std::vector<int> n_vintages_aspect(n_constituents);
  for(int i=0; i<n_inclusions; i++)
    n_vintages_aspect[i] = static_cast<int>(inclusion_aspect_ratio_[i].size());

  std::vector<int> n_vintages_volume(n_constituents);
  for(int i=0; i<n_constituents; i++)
    n_vintages_volume[i] = static_cast<int>(volume_fractions[i].size());

  // Order in alpha: aspect_ratios, host_volume_fraction, inclusion_volume_fractions
  std::vector<double> alpha(n_inclusions + n_constituents);

  for(int i=0; i<n_inclusions; i++)
    alpha[i] = inclusion_aspect_ratio_[i][0]->GetOneYearCorrelation();

  for(int i=0; i<n_constituents; i++) {
    if(volume_fractions[0][i] != NULL)
      alpha[i+n_inclusions] = volume_fractions[i][0]->GetOneYearCorrelation();
    else
      alpha[i+n_inclusions] = 1;
  }

  //Read host label
  std::vector<DistributionsSolid *> final_distr_solid (n_vintages);
  std::vector<DistributionsSolid *> distr_solid;

  distr_solid = ReadSolid(n_vintages,
                          host_label_,
                          path,
                          trend_cube_parameters,
                          trend_cube_sampling,
                          model_solid_storage,
                          errTxt);

  for(int i=0; i<n_vintages; i++) {
    if(i < static_cast<int>(distr_solid.size()))
      final_distr_solid[i] = distr_solid[i];
    else
      final_distr_solid[i] = final_distr_solid[i-1]->Clone();
  }

  //Read inclusion label
  std::vector<std::vector<DistributionsSolid *> > final_distr_solid_inc(n_vintages);
  for(int i=0; i<n_vintages; i++)
    final_distr_solid_inc[i].resize(n_inclusions, NULL);

  for (int s = 0; s < n_inclusions; s++) {
    std::vector<DistributionsSolid *> distr_solid_all_vintages = ReadSolid(n_vintages,
                                                                           inclusion_label_[s],
                                                                           path,
                                                                           trend_cube_parameters,
                                                                           trend_cube_sampling,
                                                                           model_solid_storage,
                                                                           errTxt);

    for(int i=0; i<n_vintages; i++) {
      if(i < static_cast<int>(distr_solid_all_vintages.size()))
        final_distr_solid_inc[i][s] = distr_solid_all_vintages[i];
      else
        final_distr_solid_inc[i][s] = final_distr_solid_inc[i-1][s]->Clone();
    }
  }

  std::vector<DistributionsSolid *>                  final_dist_solid(n_vintages, NULL);
  std::vector<std::vector<DistributionWithTrend *> > all_volume_fractions(n_vintages);
  std::vector<std::vector<DistributionWithTrend *> > all_aspect_ratios(n_vintages);

  for(int i=0; i<n_vintages; i++) {
    all_volume_fractions[i].resize(n_constituents, NULL);
    all_aspect_ratios[i].resize(n_inclusions, NULL);
  }


  for(int i=0; i<n_vintages; i++) {

    for (int s = 0; s < n_inclusions; s++) {

      if(i < n_vintages_aspect[s]) {
        if(inclusion_aspect_ratio_[s][i] != NULL)
          all_aspect_ratios[i][s] = inclusion_aspect_ratio_[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
      }
      else
        all_aspect_ratios[i][s] = all_aspect_ratios[i-1][s]->Clone();
    }

    for (int s = 0; s < n_constituents; s++) {

      if(i < n_vintages_volume[s]) {
        if(volume_fractions[s][i] != NULL)
          all_volume_fractions[i][s] = volume_fractions[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, errTxt);
      }
      else
        all_volume_fractions[i][s] = all_volume_fractions[i-1][s]->Clone();
    }

    CheckVolumeConsistency(all_volume_fractions[i], errTxt);

  }

  for(int i=0; i<n_vintages; i++) {
    if (errTxt == "")
      final_dist_solid[i] = new DistributionsSolidDEM(final_distr_solid[i],
                                                      final_distr_solid_inc[i],
                                                      all_aspect_ratios[i],
                                                      all_volume_fractions[i],
                                                      alpha);
  }

  return(final_dist_solid);

}
