#include <iostream>
#include <string.h>

#include "lib/utils.h"
#include "lib/global_def.h"
#include "src/definitions.h"

#include "fft/include/fftw.h"
#include "fft/include/rfftw.h"
#include "fft/include/fftw-int.h"
#include "fft/include/f77_func.h"

#include "nrlib/iotools/logkit.hpp"

//------------------------------------------------------------
void
Utils::writeHeader(const std::string     & text, 
                   LogKit::MessageLevels   logLevel)
{
  int width = 100; // Total width of header
  std::string ruler(width,'*');
  std::string stars("*****");
  LogKit::LogFormatted(logLevel,"\n"+ruler+"\n");
  int starLength  = int(stars.length());
  int textLength  = int(text.length());
  int blankLength = width - textLength - 2*starLength;
  std::string blanks(blankLength/2,' ');
  std::string center;
  if(blankLength % 2)
    center = stars + blanks + text + blanks + " " + stars;
  else
    center = stars + blanks + text + blanks +stars;
  LogKit::LogFormatted(logLevel,center+"\n");
  LogKit::LogFormatted(logLevel,ruler+"\n");
}

//------------------------------------------------------------
void    
Utils::writeTitler(const char * text)
{
  const int length = static_cast<int>(strlen(text));
  LogKit::LogFormatted(LogKit::LOW,"\n%s\n",text);
  for (int i = 0 ; i < length ; i++)
    LogKit::LogFormatted(LogKit::LOW,"-");
  LogKit::LogFormatted(LogKit::LOW,"\n");
}  

//------------------------------------------------------------
void
Utils::copyVector(const int * from,
                  int       * to,
                  int         ndim)
{
  for (int i = 0 ; i < ndim ; i++)
    to[i] = from[i];
}

//------------------------------------------------------------
void
Utils::copyVector(const float * from,
                  float       * to,
                  int           ndim)
{
  for (int i = 0 ; i < ndim ; i++)
    to[i] = from[i];
}

//------------------------------------------------------------
void
Utils::copyMatrix(const float ** from,
                  float       ** to,
                  int            ndim1,
                  int            ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++)
    for (int j = 0 ; j < ndim2 ; j++)
      to[i][j] = from[i][j];
}

//------------------------------------------------------------
void    
Utils::writeVector(float * vector,
                   int     ndim)
{
  for (int i = 0 ; i < ndim ; i++) {
    LogKit::LogFormatted(LogKit::LOW,"%10.6f ",vector[i]);
  }
  LogKit::LogFormatted(LogKit::LOW,"\n");
}

//------------------------------------------------------------
void    
Utils::writeVector(double * vector,
                   int      ndim)
{
  for (int i = 0 ; i < ndim ; i++) {
    LogKit::LogFormatted(LogKit::LOW,"%10.6f ",vector[i]);
  }
  LogKit::LogFormatted(LogKit::LOW,"\n");
}

//------------------------------------------------------------
void    
Utils::writeMatrix(float ** matrix,
                   int      ndim1,
                   int      ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++) {
    for (int j = 0 ; j < ndim2 ; j++) {
      LogKit::LogFormatted(LogKit::LOW,"%10.6f ",matrix[i][j]);
    }
    LogKit::LogFormatted(LogKit::LOW,"\n");
  }
}

//------------------------------------------------------------
void    
Utils::writeMatrix(double ** matrix,
                   int       ndim1,
                   int       ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++) {
    for (int j = 0 ; j < ndim2 ; j++) {
      LogKit::LogFormatted(LogKit::LOW,"%10.6f ",matrix[i][j]);
    }
    LogKit::LogFormatted(LogKit::LOW,"\n");
  }
}

//------------------------------------------------------------
void
Utils::fft(fftw_real* rAmp,fftw_complex* cAmp,int nt)
{
  rfftwnd_plan p1 = rfftwnd_create_plan(1, &nt, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE | FFTW_IN_PLACE);
  rfftwnd_one_real_to_complex(p1, rAmp, cAmp);
  fftwnd_destroy_plan(p1);
}

//------------------------------------------------------------
void           
Utils::fftInv(fftw_complex* cAmp,fftw_real* rAmp,int nt)
{
  rfftwnd_plan p2 = rfftwnd_create_plan(1, &nt, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE | FFTW_IN_PLACE);
  rfftwnd_one_complex_to_real(p2, cAmp, rAmp);
  fftwnd_destroy_plan(p2);
  double sf = 1.0/double(nt);
  for(int i=0;i<nt;i++)
    rAmp[i]*=fftw_real(sf);
}
//-----------------------------------------------------------
int 
Utils::findEnd(std::string & seek, int start, std::string & find)
{
  int i    = start;
  int end  = seek.length();
  int ok   = find.length();
  int flag = 0;

  while(flag < ok && i < end)
  {
    if(seek[i] == find[flag])
      flag++;
    else
      flag = 0;
    i++;
  }
  if(flag == ok)
    return(i-1);
  else
    return(-1);
}
//-------------------------------------------------------------
void
Utils::readUntilStop(int pos, std::string & in, std::string & out, std::string read)
{ 
  /*
    reads a string from position pos+1 
    until the terminating char given in stop 
    returns the string excluding the stop
  */
  
  int stop = in.find(read,pos);
  if (stop < 0)
    stop = in.length();
  out = "";
  int i=0;
  while (i < stop-pos-1)
  {
    out.push_back(in[pos+1+i]);
    i++;
  }
}
