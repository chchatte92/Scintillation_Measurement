#include "Constants.h"
#include "Process.h"
int main(){
  printf("%lf, %lf\n", h*1e34,c);
  Process p;
  p.SetFolder("char");
  p.SetCalibConstant(5.02);
  p.ReadNormalization();
  p.ReadCalibration();
  p.ReadBkg();
  p.ReadData();
  return 0;
}
