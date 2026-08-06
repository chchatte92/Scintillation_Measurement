#include "Process.h"

Process::Process(){
  printf("Process Called\n");
  
  m_file = std::make_unique<TFile>(
				   Config::RootOutputFile,
				   "RECREATE"
				   );
  if(Config::BookStatus!=0){
    m_file->mkdir("Calibration");
    m_file->mkdir("Data");
  }
  m_file->cd();
  m_tree =std::make_unique<TTree>(
		     "Spectra",
		     "Spectra"
		     );
  m_tree->SetDirectory(nullptr);
  m_tree->Branch("RunID",&m_runID);
  m_tree->Branch("NBkg",&m_nBkg);
  m_tree->Branch("NormFactor",&m_normFactor);
  m_tree->Branch("DataWL",&m_dataWL);
  m_tree->Branch("Signal",&m_treeSignal);
  m_tree->Branch("Bkg1",&m_treeBkg1);
  m_tree->Branch("Bkg2",&m_treeBkg2);

  ///Calibration
  m_tree2 = std::make_unique<TTree>(
		     "Calib",
		     "Calib"
		     );
  m_tree2->SetDirectory(nullptr);
  m_tree2->Branch("CalibWL",&m_wl);
  m_tree2->Branch("Calib",&m_calib);
  ReadNormalization();
}

Process::~Process(){
  if(m_file){
    m_file->cd();
    m_tree->Write();
    m_tree2->Write();
    m_file->Write();
    
  }
  printf("Process Over\n");
}

void Process::ReadNormalization(){
  std::ifstream file(Config::NormalizationFile);
  if(!file.is_open()){
    std::cerr
      << "Cannot open "
      << Config::NormalizationFile
      << std::endl;
    return;
  }
  std::string RunID;
  double norm;
  while(file>>RunID>>norm){
    int run =std::stoi(RunID.substr(3));
    m_normalization[run] = norm;
  }
  file.close();
}

void Process::ReadCalibration(){
  std::ifstream file(Config::CalibrationFile);

  if(!file.is_open()){
    std::cerr
      << "Cannot open "
      << Config::CalibrationFile
      << std::endl;
    return;
  }

  double wl;
  double calib;

  while(file >> wl >> calib){
    m_wl.push_back(wl);
    m_en.push_back((h*c)/(wl*e*1.0e-9));
    m_calib.push_back(calib);
  }
  m_tree2->Fill();
  //m_tree2->Write();
  if(Config::BookStatus!=0){
    BookPlots(1);
    m_file->cd("Calibration");
  }
  TVectorD wlAxis(m_wl.size());
  TVectorD calibVec(m_calib.size());

  for(size_t i=0;i<m_wl.size();i++){
    wlAxis[i] = m_wl[i];
    calibVec[i] = m_calib[i];
  }

  //wlAxis.Write("WavelengthAxis");
  //calibVec.Write("CalibrationCurve");
}

void Process::ReadData(){
  namespace fs = std::filesystem;

  fs::path baseFolder = Config::BaseFolder;
  fs::path anaFolder = baseFolder/Config::AnaDirectory;

  std::vector<fs::path> txtFiles;

  int nRuns = 0;

  if(!fs::exists(anaFolder)){
    std::cerr << "Directory does not exist\n";
    return;
  }

  for(const auto& runDir : fs::directory_iterator(anaFolder)){
    if(!runDir.is_directory())
      continue;

    ++nRuns;

    fs::path dataDir = runDir.path()/"Data";

    if(!fs::exists(dataDir))
      continue;

    for(const auto& file : fs::directory_iterator(dataDir)){
      if(!file.is_regular_file())
        continue;

      if(file.path().extension() != ".txt")
        continue;

      txtFiles.push_back(file.path());
    }
  }

  //printf("-->%d\n",nRuns);

  for(const auto& txtFile : txtFiles){
    std::ifstream input(txtFile);

    if(!input.is_open())
      continue;

    m_dataWL.clear();
    m_dataReadings.clear();

    m_treeSignal.clear();
    m_treeBkg1.clear();
    m_treeBkg2.clear();

    double wl;
    double val;
    std::string line;
    //while(input >> wl >> val){
    while(std::getline(input, line)){
      if(!ReadXYLine(line, wl, val))
	continue;
      if(wl<GetLWL() || wl>GetUWL())
	continue;

      m_dataWL.push_back(wl);
      if(val>m_selectionMaskMeanCutUpper || val<m_selectionMaskMeanCutLower)
	val= 0;
      m_dataReadings.push_back(val);
    }

    input.close();

    m_treeSignal = m_dataReadings;

    
    std::string runName =
      txtFile.parent_path()
      .parent_path()
      .filename()
      .string();
    //std::cout<<runName<<std::endl;
    
    auto it = m_backgrounds.find(runName);

    if(it != m_backgrounds.end()){
      m_nBkg = it->second.nFiles;

      m_treeBkg1 = it->second.bg1;

      if(m_nBkg > 1)
        m_treeBkg2 = it->second.bg2;
    }else{
      m_nBkg = 0;
    }
    
    m_runID = std::stoi(runName.substr(3));
    auto norm = m_normalization.find(m_runID);
    //std::cout<<"Found Run: "<<norm->first<<std::endl;
    if(norm != m_normalization.end()){
      m_normFactor = norm->second;
    }
    else{
      std::cerr 
        << "No normalization found for Run "
        << m_runID
        << std::endl;
      m_normFactor = 1.0;
    }
    
    m_tree->Fill();
    if(Config::BookStatus!=0)
      BookPlots(2);
  }//Files
}

void Process::ReadBkg(){
  namespace fs = std::filesystem;

  fs::path anaFolder =
    fs::path(Config::BaseFolder)/
    Config::AnaDirectory;

  if(!fs::exists(anaFolder)){
    std::cerr << "Directory does not exist\n";
    return;
  }

  auto ReadFile = [this](const fs::path& file,std::vector<double>& wl,std::vector<double>& val){
    std::ifstream input(file);

    if(!input.is_open())
      return;

    double x,y;
    std::string line;

    while(std::getline(input,line)){
      if(!ReadXYLine(line,x,y))
	continue;
      if(x< GetLWL() || x>GetUWL())
	continue;
      wl.push_back(x);
      if(y>m_selectionMaskMeanCutUpper || y<m_selectionMaskMeanCutLower)
	y =0;
      val.push_back(y);
    }
    
  };

  m_backgrounds.clear();
  //printf("I am here\n");
  for(const auto& runDir : fs::directory_iterator(anaFolder)){
    if(!runDir.is_directory())
      continue;

    std::string runName =
      runDir.path().filename().string();

    fs::path bgDir =
      runDir.path()/"DataBG";

    if(!fs::exists(bgDir))
      continue;

    std::vector<fs::path> files;

    for(const auto& file : fs::directory_iterator(bgDir)){
      if(!file.is_regular_file())
        continue;

      if(file.path().extension() != ".txt")
        continue;

      files.push_back(file.path());
    }
    
    if(files.empty())
      continue;
    
    std::sort(files.begin(),files.end());

    BackgroundData bg;

    bg.nFiles = std::min((int)files.size(),2);

    ReadFile(files[0],bg.wl,bg.bg1);

    if(files.size() > 1){
      std::vector<double> wl2;

      ReadFile(files[1],wl2,bg.bg2);

      if(wl2.size() != bg.wl.size()){
        std::cerr
          << runName
          << ": background sizes differ\n";
        continue;
      }
    }

    m_backgrounds[runName] = std::move(bg);

    std::cout
      << runName
      << " : "
      << bg.nFiles
      << " background file(s) loaded"<<std::endl;
  }
}

void Process::BookPlots(int index){
  if(Config::BookStatus == 0){
    static bool printed =false;
    if(!printed){
      printf("\033[31mNo Booking requested\033[0m\n");
      printed =true;
    }
    return; 
  }
  if(index == 1 && Config::BookStatus!=0){
    m_file->cd("Calibration");
    m_gr_calib_Wl = std::make_unique<TGraph>();

    int i = 0;

    for(auto [c,w] : std::views::zip(m_calib,m_wl)){
      m_gr_calib_Wl->SetPoint(i,w,c);
      i++;
    }

    m_gr_calib_Wl->SetName("WaveLength");
    m_gr_calib_Wl->SetTitle("WaveLength");
    m_gr_calib_Wl->Write();
    m_gr_calib_En = std::make_unique<TGraph>();

    i = 0;

    for(auto [c,e] : std::views::zip(m_calib,m_en)){
      m_gr_calib_En->SetPoint(i,e,c);
      i++;
    }

    m_gr_calib_En->SetName("Energy");
    m_gr_calib_En->SetTitle("Energy");
    m_gr_calib_En->Write();
  }

  if(index == 2 && Config::BookStatus!=0){
    m_file->cd("Data");
    m_gr_data_Wl = std::make_unique<TGraph>();

    int i = 0;

    for(auto [c,w] : std::views::zip(m_dataReadings,m_dataWL)){
      m_gr_data_Wl->SetPoint(i,w,c);
      i++;
    }

    TString name;
    name.Form("WaveLength_%03d",m_graphCounter++);

    m_gr_data_Wl->SetName(name);
    m_gr_data_Wl->SetTitle(name);

    m_gr_data_Wl->Write();
  }
}

bool Process::ReadXYLine(const std::string line, double& x, double& y)
{
    std::stringstream ss(line);

    if(!(ss >> x >> y))
        return false;

    return true;
}
