#ifndef RPLIB_DISTRIBUTIONS_DRYROCK_TABULATED_H
#define RPLIB_DISTRIBUTIONS_DRYROCK_TABULATED_H

#include "rplib/distributionsdryrock.h"
#include "rplib/demmodelling.h"

class DryRock;
class DistributionWithTrend;
class Tabulated;


// Abstract class for holding all t = 0 distribution functions for solid parameters.
// One derived class for each solid model, the latter specified in a parallel, derived DryRock class.
// The class must be able to produce an object of the specific DryRock class.
class DistributionsDryRockTabulated : public DistributionsDryRock {
public:

  //NB: Class is not completed
  DistributionsDryRockTabulated(const DistributionWithTrend * elastic1,
                                const DistributionWithTrend * elastic2,
                                const DistributionWithTrend * density,
                                double                        corr_elastic1_elastic2,
                                double                        corr_elastic1_density,
                                double                        corr_elastic2_density,
                                DEMTools::TabulatedMethod     method,
                                std::vector<double>         & alpha);

  DistributionsDryRockTabulated(const DistributionsDryRockTabulated & dist);

  virtual ~DistributionsDryRockTabulated();

  // DryRock is an abstract class, hence pointer must be used here. Allocated memory (using new) MUST be deleted by caller.

  virtual DistributionsDryRock  * Clone() const;

  virtual DryRock               * GenerateSample(const std::vector<double> & trend_params) const;

  virtual bool                    HasDistribution() const;

  virtual std::vector<bool>       HasTrend() const;

protected:
  virtual DryRock *               UpdateSample(double                      corr_param,
                                               bool                        param_is_time,
                                               const std::vector<double> & trend,
                                               const DryRock             * sample) const;

private:
  DryRock                       * GetSample(const std::vector<double> & u, const std::vector<double> & trend_params) const;

  const DistributionWithTrend   * elastic1_;
  const DistributionWithTrend   * elastic2_;
  const DistributionWithTrend   * density_;
  double                          corr_elastic1_elastic2_;
  double                          corr_elastic1_density_;
  double                          corr_elastic2_density_;
  Tabulated                     * tabulated_;
  DEMTools::TabulatedMethod       tabulated_method_;
};

#endif
