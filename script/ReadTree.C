#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <vector>
#include "../include/Constants.h"
int main() {

  TFile file("../Output/C2F6Transparency.root", "READ");

  //=========================
  // Spectra tree
  //=========================
  TTree* spectra = nullptr;
  file.GetObject("Spectra", spectra);

  int runID;
  int nBkg;
  double normFactor;

  std::vector<double>* dataWL = nullptr;
  std::vector<double>* signal = nullptr;
  std::vector<double>* bkg1 = nullptr;
  std::vector<double>* bkg2 = nullptr;

  spectra->SetBranchAddress("RunID", &runID);
  spectra->SetBranchAddress("NBkg", &nBkg);
  spectra->SetBranchAddress("NormFactor", &normFactor);
  spectra->SetBranchAddress("DataWL", &dataWL);
  spectra->SetBranchAddress("Signal", &signal);
  spectra->SetBranchAddress("Bkg1", &bkg1);
  spectra->SetBranchAddress("Bkg2", &bkg2);

  //=========================
  // Calibration tree
  //=========================
  TTree* calib = nullptr;
  file.GetObject("Calib", calib);

  std::vector<double>* calibWL = nullptr;
  std::vector<double>* calibVec = nullptr;

  calib->SetBranchAddress("CalibWL", &calibWL);
  calib->SetBranchAddress("Calib", &calibVec);

  // Read calibration (usually one entry)
  calib->GetEntry(0);

  std::cout << "Calibration points: "
	    << calibWL->size() << std::endl;

  //=========================
  // Read spectra
  //=========================
  Long64_t nEntries = spectra->GetEntries();

  for (Long64_t i = 0; i < nEntries; ++i) {
    std::cout<<Constants::h<<std::endl;
    spectra->GetEntry(i);

    std::cout << "Run " << runID
	      << "  Norm = " << normFactor
	      << std::endl;

    for (size_t j = 0; j < dataWL->size(); ++j) {
      
      std::cout
	<< dataWL->at(j) << "  "
	<< signal->at(j);

      if (nBkg > 0)
	std::cout << "  " << bkg1->at(j);

      if (nBkg > 1)
	std::cout << "  " << bkg2->at(j);

      std::cout << '\n';
      
      //Yield 
    }
  }

  file.Close();
  return 0;
}
