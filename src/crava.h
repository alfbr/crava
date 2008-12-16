#ifndef CRAVA_H
#define CRAVA_H

#include "fft/include/fftw.h"
#include "definitions.h"

class Model;
class FFTGrid;
class FFTFileGrid;
class Wavelet;
class Simbox;
class RandomGen;
class WellData;
class CKrigingAdmin;
class CovGridSeparated;
class KrigingData3D;
class FaciesProb;
class GridMapping;
class FilterWellLogs;
class Corr;

class Crava 
{
public:
  Crava(Model * model);
  ~Crava();
  int                computePostMeanResidAndFFTCov();
  int                simulate( RandomGen * randomGen );
  int                computeSyntSeismic(FFTGrid * Alpha, FFTGrid * Beta, FFTGrid * Rho);

  FFTGrid          * getPostAlpha()                 { return postAlpha_ ;}
  FFTGrid          * getPostBeta()                  { return postBeta_  ;}
  FFTGrid          * getPostRho()                   { return postRho_   ;}

  int                getWarning(char* wText)  const {if(scaleWarning_>0) sprintf(wText,"%s",scaleWarningText_); return scaleWarning_;}

  void               printEnergyToScreen();
  void               computeFaciesProb(FilterWellLogs *filteredlogs);
  void               filterLogs(Simbox          * timeSimboxConstThick,
                                FilterWellLogs *& filterlogs);

private: 
  int                computeAcousticImpedance(FFTGrid * Alpha, FFTGrid * Rho, char * fileName);
  int                computeShearImpedance(FFTGrid * Beta, FFTGrid * Rho, char * fileName);
  int                computeVpVsRatio(FFTGrid * Alpha, FFTGrid * Beta, char * fileName);
  int                computePoissonRatio(FFTGrid * Alpha, FFTGrid * Beta, char * fileName);
  int                computeLameMu(FFTGrid * Beta, FFTGrid * Rho , char * FileName);
  int                computeLameLambda(FFTGrid * Alpha, FFTGrid * Beta, FFTGrid * Rho, char * fileName);
  int                computeMuRho(FFTGrid * Alpha, FFTGrid * Beta, FFTGrid * Rho, char * fileName);
  int                computeLambdaRho(FFTGrid * Alpha, FFTGrid * Beta, FFTGrid * Rho, char * fileName);
  void               computeDataVariance(void);
  void               setupErrorCorrelation(Model * model);
  void               computeVariances(fftw_real* corrT, Model * model);
  float              getEmpSNRatio(int l)     const {return empSNRatio_[l];}
  float              getTheoSNRatio(int l)    const {return theoSNRatio_[l];}
  float              getSignalVariance(int l) const {return signalVariance_[l];}
  float              getErrorVariance(int l)  const {return errorVariance_[l];}
  float              getDataVariance(int l)   const {return dataVariance_[l];}
  int                checkScale(void);

  //Conventions for writePars:
  // simNum = -1 indicates prediction, otherwise filename ends with n+1.
  // All grids are in normal domain, and on log scale.
  void               writePars(FFTGrid * alpha, FFTGrid * beta, FFTGrid * rho, int simNum); 

  void               fillkW(int k, fftw_complex* kW );
  void               fillInverseAbskWRobust(int k, fftw_complex* invkW );
  void               fillkWNorm(int k, fftw_complex* kWNorm, Wavelet** wavelet);
  FFTGrid          * createFFTGrid();
  FFTGrid          * copyFFTGrid(FFTGrid * fftGridOld);
  FFTFileGrid      * copyFFTGrid(FFTFileGrid * fftGridOld);

  float              computeWDCorrMVar (Wavelet* WD, fftw_real* corrT);
  float              computeWDCorrMVar (Wavelet* WD);

  void               divideDataByScaleWavelet();
  void               multiplyDataByScaleWaveletAndWriteToFile(const char* typeName);
  void               doPostKriging(FFTGrid & postAlpha, FFTGrid & postBeta, FFTGrid & postRho);

  void               writeToFile(char * fileName, FFTGrid * grid, std::string sgriLabel = "NO_LABEL");

  int                fileGrid_;        // is true if is storage is on file 
  const Simbox     * simbox_;          // the simbox
  int                nx_;              // dimensions of the problem
  int                ny_;
  int                nz_;
  int                nxp_;             // padded dimensions
  int                nyp_;
  int                nzp_; 

  Corr             * correlations_;    //

  int                ntheta_;          // number of seismic cubes and number of wavelets
  float              lowCut_;          // lowest frequency that is inverted
  float              highCut_;         // highest frequency that is inverted

  int                nSim_;            // number of simulations
  float            * theta_;           // in radians

  FFTGrid          * meanAlpha_;       // mean values
  FFTGrid          * meanBeta_;
  FFTGrid          * meanRho_;
  FFTGrid          * meanAlpha2_;       // copy of mean values, to be used for facies prob, new method 
  FFTGrid          * meanBeta2_;
  FFTGrid          * meanRho2_;
  FFTGrid          * parSpatialCorr_;   // parameter correlation
  float           ** parPointCov_; 

  Wavelet         ** seisWavelet_;      // wavelet operator that define the forward map.
  FFTGrid         ** seisData_;         // Data
  FFTGrid          * errCorrUnsmooth_;  // Error correlation
  float           ** errThetaCov_;      //
  float              wnc_ ;          // if wnc=0.01 1% of the error wariance is white this has largest effect on
                                     // high frequency components. It makes everything run smoother we
                                     // avoid ill posed problems.
  float           ** A_;             // 

  float            * empSNRatio_;    // signal noise ratio empirical
  float            * theoSNRatio_;   // signal noise ratio from model
  float            * modelVariance_;
  float            * signalVariance_;
  float            * errorVariance_;
  float            * dataVariance_;

  FFTGrid          * postAlpha_;     // posterior values 
  FFTGrid          * postBeta_;
  FFTGrid          * postRho_;

  float            * krigingParams_;
  WellData        ** wells_;
  int                nWells_;
  
  int                scaleWarning_;
  char             * scaleWarningText_;

  int                outputFlag_; //See model.h for bit interpretation.
  
  float              energyTreshold_; //If energy in reflection trace divided by mean energy
                                      //in reflection trace is lower than this, the reflections
                                      //will be interpolated. Default 0, set from model.
  RandomGen        * random_;
  FaciesProb       * fprob_;
  Model            * model_;
};
#endif
