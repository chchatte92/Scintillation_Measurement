#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <map>
#include <ranges>

#include "TVectorD.h"
#include "TGraph.h"
#include "TFile.h"
#include "TTree.h"

#include "Constants.h"
#include "CutOffs.h"
#include "config.h"

using namespace Constants;

struct BackgroundData{
  int nFiles = 0;
  std::vector<double> wl;
  std::vector<double> bg1;
  std::vector<double> bg2;
};

class Process: public CutOffs{
 public:
  Process();
  ~Process();
  //void SetFolder(const char* base ){m_baseFolder = base;};
  //void SetOutName(const char* out){m_resultsName = out;};
  //void SetNFiles(int nF) {m_nFiles =nF;};
  //void SetMaxFiles(int maxF){m_maxFiles= maxF;};
  //void SetComputeError(bool cError){m_computeError = cError;};
  //void SetCalibConstant(double calibConst){m_calibConstant = calibConst;};
  void DataProcess();
  void ReadNormalization();
  void ReadCalibration();
  void BookPlots(int);
  void ReadData();
  void ReadBkg();
 private:
  //ordinary varaibles 
  int m_nFiles;
  int m_maxFiles;
  int m_graphCounter = 0;
  int m_runID;
  int m_nBkg = 0;

  double m_calibConstant;
  double m_normFactor = 1.0;
  
  const char * m_baseFolder;
  const char * m_dataPath;
  const char * m_resultsName;
  
  bool m_computeError;
  
  
  //Vectors for Tree variables et al.
  std::vector<double> m_wl;
  std::vector<double> m_en;
  std::vector<double> m_calib;
  std::vector<double> m_dataWL;
  std::vector<double> m_dataReadings;
  std::vector<double> m_bgWL;
  std::vector<double> m_bgReadings;
  std::vector<double> m_treeSignal;
  std::vector<double> m_treeBkg1;
  std::vector<double> m_treeBkg2;
  std::vector<double> m_lightYield;
  
  //smart Pointers to trees and graphs and TFiles
  std::unique_ptr<TFile> m_file;
  std::unique_ptr<TGraph> m_gr_calib_Wl;
  std::unique_ptr<TGraph> m_gr_calib_En;
  std::unique_ptr<TGraph> m_gr_data_Wl;
  std::unique_ptr<TTree> m_tree;
  std::unique_ptr<TTree> m_tree2;
  
  //Mappings
  std::map<std::string,BackgroundData> m_backgrounds;
  std::map <int,double> m_normalization;

};
