#pragma once
#include "config.h"
class CutOffs{
 public:
   const double GetLWL(){return m_CutOFFWL[0];};
   const double GetUWL(){return m_CutOFFWL[1];};
 protected:
  const double *m_CutOFFWL = Config::CutoffWavelengths;
  int m_selectionMaskStartIndex = 1150;
  int m_selectionMaskEndIndex = 1300;
  double m_selectionMaskMeanCutLower = -1e6;
  double m_selectionMaskMeanCutUpper = 8.5e8;

};
