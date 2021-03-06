/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#ifndef CRAVA_H
#define CRAVA_H

#include "fftw.h"
#include "definitions.h"
#include "libs/nrlib/flens/nrlib_flens.hpp"

class ModelGeneral;
class ModelAVOStatic;
class ModelAVODynamic;
class FFTGrid;
class FFTFileGrid;
class Wavelet;
class Wavelet1D;
class Simbox;
class RandomGen;
class WellData;
class CKrigingAdmin;
class CovGridSeparated;
class KrigingData3D;
class FaciesProb;
class GridMapping;
class ModelSettings;
class SpatialWellFilter;
class SeismicParametersHolder;


class Crava
{
public:
  Crava(ModelSettings           * modelSettings,
        ModelGeneral            * modelGeneral,
        ModelAVOStatic          * modelAVOstatic,
        ModelAVODynamic         * modelAVOdynamic,
        SeismicParametersHolder & seismicParameters);

  ~Crava();

  int                    computePostMeanResidAndFFTCov(ModelGeneral * modelGeneral, SeismicParametersHolder & seismicParameters);
  int                    computeSyntSeismicOld(FFTGrid * Alpha, FFTGrid * Beta, FFTGrid * Rho);

  FFTGrid              * getPostAlpha() { return postAlpha_ ;}
  FFTGrid              * getPostBeta()  { return postBeta_  ;}
  FFTGrid              * getPostRho()   { return postRho_   ;}

  int                    getWarning(std::string & wText)  const {if(scaleWarning_>0) wText=scaleWarningText_; return scaleWarning_;}

  int                    getRelative();

  void                   computeG(NRLib::Matrix & G) const;
  NRLib::Matrix          getPriorVar0() const;
  NRLib::Matrix          getPostVar0() const;
  NRLib::SymmetricMatrix getSymmetricPriorVar0() const;
  NRLib::SymmetricMatrix getSymmetricPostVar0() const;
  void                   newPosteriorCovPointwise(NRLib::Matrix                 & sigmanew,
                                                  NRLib::Matrix                 & G,
                                                  NRLib::Vector                 & scales,
                                                  NRLib::Matrix                 & sigmamdnew) const;
  NRLib::Matrix          computeFilter(NRLib::SymmetricMatrix & priorCov,
                                       NRLib::SymmetricMatrix & posteriorCov) const;

private:
  void                   computeDataVariance(void);
  void                   setupErrorCorrelation(const std::vector<Grid2D *> & noiseScale);

  void                   computeVariances(fftw_real     * corrT,
                                          ModelSettings * modelSettings);

  void                   writeBWPredicted(void);
  float                  getEmpSNRatio(int l)     const { return empSNRatio_[l]     ;}
  float                  getTheoSNRatio(int l)    const { return theoSNRatio_[l]    ;}
  float                  getSignalVariance(int l) const { return signalVariance_[l] ;}
  float                  getErrorVariance(int l)  const { return errorVariance_[l]  ;}
  float                  getDataVariance(int l)   const { return dataVariance_[l]   ;}
  int                simulate(SeismicParametersHolder & seismicParameters, RandomGen * randomGen );
  int                computePostMeanResidAndFFTCov(ModelGeneral * modelGeneral);
  void               printEnergyToScreen();
  void               computeSyntSeismic(FFTGrid * alpha, FFTGrid * beta, FFTGrid * rho);
  void               computeFaciesProb(SpatialWellFilter       * filteredlogs,
                                       bool                      useFilter,
                                       SeismicParametersHolder & seismicParameters);
  void               computeFaciesProbFromRockPhysicsModel(SpatialWellFilter *filteredlogs, bool useFilter);
  //void               filterLogs(Simbox * timeSimboxConstThick, FilterWellLogs *& filterlogs);
  void               doPredictionKriging(SeismicParametersHolder & seismicParameters);
  void               computeElasticImpedanceTimeCovariance(fftw_real * eiCovT,
                                                           float     * corrT,
                                                           float     * A) const;

  void               computeReflectionCoefficientTimeCovariance(fftw_real * refCovT,
                                                                float     * corrT,
                                                                float     * A ) const ;

  int                    checkScale(void);

  void                   fillkW(int k, fftw_complex* kW, Wavelet** seisWavelet);
  void                   fillInverseAbskWRobust(int k, fftw_complex* invkW ,Wavelet1D** seisWaveletForNorm);
  void                   fillkWNorm(int k, fftw_complex* kWNorm, Wavelet1D** wavelet);

  void                   fillkW_flens(int k, NRLib::ComplexVector & kW, Wavelet** seisWavelet);
  void                   fillInverseAbskWRobust_flens(int k, NRLib::ComplexVector & invkW, Wavelet1D** seisWaveletForNorm);
  void                   fillkWNorm_flens(int k, NRLib::ComplexVector & kWNorm, Wavelet1D** wavelet);

  void                   computeAdjustmentFactor(fftw_complex                  * relativeWeights,
                                                 Wavelet1D                     * wLocal,
                                                 double                          scaleF,
                                                 Wavelet                       * wGlobal,
                                                 const SeismicParametersHolder & seismicParameters,
                                                 float                         * A,
                                                 float                           errorVar);

  FFTGrid              * createFFTGrid();
  FFTGrid              * copyFFTGrid(FFTGrid * fftGridOld);
  FFTFileGrid          * copyFFTGrid(FFTFileGrid * fftGridOld);

  float                  computeWDCorrMVar (Wavelet1D* WD, fftw_real* corrT);

  void                   divideDataByScaleWavelet(const SeismicParametersHolder & seismicParameters);
  void                   multiplyDataByScaleWaveletAndWriteToFile(const std::string & typeName);
  void                   doPostKriging(SeismicParametersHolder & seismicParameters, FFTGrid & postAlpha, FFTGrid & postBeta, FFTGrid & postRho);

  void                   correctAlphaBetaRho(ModelSettings * modelSettings);

  FFTGrid *              computeSeismicImpedance(FFTGrid * alpha,
                                                 FFTGrid * beta,
                                                 FFTGrid * rho,
                                                 int angle);

  std::complex<double>   SetComplexNumber(const fftw_complex & c);

  void                   SetComplexVector(NRLib::ComplexVector & V,
                                          fftw_complex         * v);

  void                   getNextErrorVariance(fftw_complex **& errVar,
                                              fftw_complex   * errMult1,
                                              fftw_complex   * errMult2,
                                              fftw_complex   * errMult3,
                                              int              ntheta,
                                              float            wnc,
                                              double        ** errThetaCov,
                                              bool             invert_frequency) const;

  bool               fileGrid_;         // is true if is storage is on file
  const Simbox     * simbox_;           // the simbox
  int                nx_;               // dimensions of the problem
  int                ny_;
  int                nz_;
  int                nxp_;              // padded dimensions
  int                nyp_;
  int                nzp_;

  int                ntheta_;           // number of seismic cubes and number of wavelets
  float              lowCut_;           // lowest frequency that is inverted
  float              highCut_;          // highest frequency that is inverted

  int                nSim_;             // number of simulations
  float            * thetaDeg_;         // in degrees

  FFTGrid          * meanAlpha_;        // mean values
  FFTGrid          * meanBeta_;
  FFTGrid          * meanRho_;
  FFTGrid          * meanAlpha2_;       // copy of mean values, to be used for facies prob, new method
  FFTGrid          * meanBeta2_;
  FFTGrid          * meanRho2_;

  Wavelet         ** seisWavelet_;      // wavelet operator that define the forward map.
  FFTGrid         ** seisData_;         // Data
  double          ** errThetaCov_;      //
  float              wnc_ ;             // if wnc=0.01 1% of the error wariance is white this has largest effect on
                                        // high frequency components. It makes everything run smoother we
                                        // avoid ill posed problems.
  float           ** A_;                // coefficients in Aki-Richards 3 term reflection coefficients

  float            * empSNRatio_;       // signal noise ratio empirical
  float            * theoSNRatio_;      // signal noise ratio from model
  float            * modelVariance_;
  float            * signalVariance_;
  float            * errorVariance_;
  float            * dataVariance_;

  NRLib::Matrix      priorVar0_;
  NRLib::Matrix      postVar0_;
  std::vector<float> postCovAlpha00_;        // Posterior covariance in (i,j) = (0,0)
  std::vector<float> postCovBeta00_;
  std::vector<float> postCovRho00_;

  FFTGrid          * postAlpha_;        // posterior values
  FFTGrid          * postBeta_;
  FFTGrid          * postRho_;
  FFTGrid          * errCorr_;

  int                     krigingParameter_;
  std::vector<WellData *> wells_;
  int                     nWells_;

  int                scaleWarning_;
  std::string        scaleWarningText_;

  int                outputGridsSeismic_; // See modelsettings.h for bit interpretation.
  int                outputGridsElastic_;
  bool               writePrediction_;  // Write prediction grids?
  bool               doing4DInversion_;

  float              energyTreshold_;   // If energy in reflection trace divided by mean energy
                                        // in reflection trace is lower than this, the reflections
                                        // will be interpolated. Default 0, set from model.
  RandomGen        * random_;
  FaciesProb       * fprob_;

  ModelSettings    * modelSettings_;
  ModelGeneral     * modelGeneral_;
  ModelAVOStatic   * modelAVOstatic_;
  ModelAVODynamic  * modelAVOdynamic_;


  NRLib::Grid2D<double **> * sigmamdnew_;
};

#endif
