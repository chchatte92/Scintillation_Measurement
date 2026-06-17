#ifndef CUTOFFS_H
#define CUTOFF_H
#include "config.h"
class CutOffs{
 public:
   const double GetLWL(){return m_LowerWL;};
   const double GetUWL(){return m_UpperWL;};
   const double GetCOF(){return m_CtOFFWL;};
   //void SetLWL();
   //void SetUWL();
   //void SetCOF();
 protected:
  const double m_CtOFFWL = Config::CutoffWavelength;
  const double m_LowerWL = 200;
  const double m_UpperWL = 800;
  int m_selectionMaskStartIndex = 1150;
  int m_selectionMaskEndIndex = 1300;
  double m_selectionMaskMeanCutLower = -1e6;
  double m_selectionMaskMeanCutUpper = 8.5e8;

};
#endif
