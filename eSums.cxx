#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include "TChain.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetFilterDataFormat.h"
    
bool met_filter(Bool_t,Bool_t,Bool_t,Bool_t,Bool_t,Bool_t,Bool_t,Bool_t);
double calc_dPHI(double,double);

struct energySums{
	float ett;
	float ettBx;
	float met;
	float metBx;
	float metPhi;
	float htt;
	float httBx;
	float mht;
	float mhtBx;
	float mhtPhi;
};

//MAIN FUNCTION//
void esums_data(){

  bool hwOn = false;  //are we using data from hardware?
  bool emuOn = true;  //are we using data from emulator?
  bool metFiltersOn = true;  //avoids using bad events (might kill everything in commisioning runs...)

  if (hwOn==false && emuOn==false){
    cout << "exiting as neither hardware or emulator selected" << endl;
    return;}

  if (hwOn==true && emuOn==true){
    cout << "exiting as both hardware and emulator selected" << endl;
    return;}

  //create a ROOT file to save all the histograms to (actually at end of script)
  //first check the file doesn't exist already so we don't overwrite
  string dirName = "output_rates/run259721_zeroBiasReReco_v39p1/"; //include whether hw/emu
  string outputFilename = dirName + "histos.root";
  TFile *kk = TFile::Open( outputFilename.c_str() );
  if (kk!=0){
    cout << "TERMINATE:not going to overwrite file" << endl;
    return;
  }

  TChain * recoTree = new TChain("l1JetRecoTree/JetRecoTree");
  recoTree->Add("/afs/cern.ch/work/s/sbreeze/public/jets_and_sums/l1t-integration-v37p2/ttHTobb/Ntuples/*.root");

  TChain * metFilterTree = new TChain("l1MetFilterRecoTree/MetFilterRecoTree");
  metFilterTree->Add("/afs/cern.ch/work/s/sbreeze/public/jets_and_sums/l1t-integration-v37p2/ttHTobb/Ntuples/*.root");

  TChain * l1emuTree = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
  if (emuOn){
    l1emuTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBiasReReco_run259721_v39p1/Run2015D_1/*.root");
    l1emuTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBiasReReco_run259721_v39p1/Run2015D_2/*.root");
    l1emuTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBiasReReco_run259721_v39p1/Run2015D_4/*.root");
  }

  TChain * l1hwTree = new TChain("l1UpgradeTree/L1UpgradeTree");
  if (hwOn){
    l1hwTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBias_run269224_v34p0/com2016_1/*.root");
    l1hwTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBias_run269224_v34p0/com2016_2/*.root");
    l1hwTree->Add("/hdfs/L1JEC/CMSSW_8_0_2/L1JetEnergyCorrections/ZeroBias_run269224_v34p0/com2016_3/*.root");
  }

  //load the number of event entries
  Int_t nevent = (Int_t)recoTree->GetEntries();

  //Structures used in script
  struct energySums l1Sums;
  struct energySums recoSums;

  //set the branch addresses
  L1Analysis::L1AnalysisRecoMetDataFormat      *recoMet_ = new L1Analysis::L1AnalysisRecoMetDataFormat();
  recoTree->SetBranchAddress("Sums", &recoMet_);
  L1Analysis::L1AnalysisRecoMetFilterDataFormat      *recoMetFilter_ = new L1Analysis::L1AnalysisRecoMetFilterDataFormat();
  metFilterTree->SetBranchAddress("MetFilters", &recoMetFilter_);
  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1emu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  l1emuTree->SetBranchAddress("L1Upgrade", &l1emu_);
  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1hw_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  l1hwTree->SetBranchAddress("L1Upgrade", &l1hw_);

///////////////////////////////////////////
//create the framework for the histograms//
///////////////////////////////////////////

//distributions
// TH1F * hMETphi_l1 = new TH1F("hMETphi_l1", ";#phi_{L1}^{MET}", 40, -M_PI, M_PI);
// TH1F * hMHTphi_l1 = new TH1F("hMHTphi_l1", ";#phi_{L1}^{MHT}", 40, -M_PI, M_PI);
// TH1F * hMETphi_reco = new TH1F("hMETphi_reco", ";#phi_{RECO}^{MET}", 40, -M_PI, M_PI);
// TH1F * hMHTphi_reco = new TH1F("hMHTphi_reco", ";#phi_{RECO}^{MHT}", 40, 0, 2*M_PI);

// //resolutions
// TH1F * hdET_ETT = new TH1F("hdET_ETT", ";(ETT_{L1} - ETT_{RECO})/ETT_{RECO}", 100, -1.0, 1.0);
// TH1F * hdET_MET = new TH1F("hdET_MET", ";(MET_{L1} - MET_{RECO})/MET_{RECO}", 100, -1.0, 3.5);
// TH1F * hdET_HTT = new TH1F("hdET_HTT", ";(HTT_{L1} - HTT_{RECO})/HTT_{RECO}", 100, -1.0, 3.0);
// TH1F * hdET_MHT = new TH1F("hdET_MHT", ";(MHT_{L1} - MHT_{RECO})/MHT_{RECO}", 100, -1.0, 2.0);
// TH1F * hdPhi_MET = new TH1F("hdPhi_MET", ";#phi_{RECO}^{MET} - #phi_{L1}^{MET}", 50, -M_PI, M_PI);
// TH1F * hdPhi_MHT = new TH1F("hdPhi_MHT", ";#phi_{RECO}^{MHT} - #phi_{L1}^{MHT}", 50, -M_PI, M_PI);
// TH2F * hETS_ETT = new TH2F("hETS_ETT", "", 200, 0, 2000, 200, 0, 800);
// hETS_ETT->GetXaxis()->SetTitle("RECO ETT (GeV)");
// hETS_ETT->GetYaxis()->SetTitle("L1 upgrade ETT (GeV)");
// TH2F * hETS_MET = new TH2F("hETS_MET", "", 200, 0, 300, 200, 0, 300);
// hETS_MET->GetXaxis()->SetTitle("RECO MET (GeV)");
// hETS_MET->GetYaxis()->SetTitle("L1 upgrade MET (GeV)");
// TH2F * hETS_HTT = new TH2F("hETS_HTT", "", 200, 0, 1200, 200, 0, 1200);
// hETS_HTT->GetXaxis()->SetTitle("RECO HTT (GeV)");
// hETS_HTT->GetYaxis()->SetTitle("L1 upgrade HTT (GeV)");
// TH2F * hETS_MHT = new TH2F("hETS_MHT", "", 200, 0, 300, 200, 0, 300);
// hETS_MHT->GetXaxis()->SetTitle("RECO MHT (GeV)");
// hETS_MHT->GetYaxis()->SetTitle("L1 upgrade MHT (GeV)");


//turnOns//

//put this into vectors...

// TH1F * hden_ETT = new TH1F("hden_ETT", "", 40, 0, 3000);
// TH1F * hden_MET = new TH1F("hden_MET", "", 40, 0, 500);
// TH1F * hden_HTT = new TH1F("hden_HTT", "", 40, 0, 3000);
// TH1F * hden_MHT = new TH1F("hden_MHT", "", 40, 0, 500);
// TH1F * hnum_ETT_100 = new TH1F("hnum_ETT_100", "", 40, 0, 3000);
// TH1F * hnum_ETT_125 = new TH1F("hnum_ETT_125", "", 40, 0, 3000);
// TH1F * hnum_ETT_150 = new TH1F("hnum_ETT_150", "", 40, 0, 3000);
// TH1F * hnum_ETT_175 = new TH1F("hnum_ETT_175", "", 40, 0, 3000);
// TH1F * hnum_ETT_200 = new TH1F("hnum_ETT_200", "", 40, 0, 3000);
// TH1F * hnum_ETT_250 = new TH1F("hnum_ETT_250", "", 40, 0, 3000);
// TH1F * hnum_MET_40 = new TH1F("hnum_MET_40", "", 40, 0, 500);
// TH1F * hnum_MET_60 = new TH1F("hnum_MET_60", "", 40, 0, 500);
// TH1F * hnum_MET_80 = new TH1F("hnum_MET_80", "", 40, 0, 500);
// TH1F * hnum_MET_100 = new TH1F("hnum_MET_100", "", 40, 0, 500);
// TH1F * hnum_HTT_100 = new TH1F("hnum_HTT_100", "", 40, 0, 3000);
// TH1F * hnum_HTT_125 = new TH1F("hnum_HTT_125", "", 40, 0, 3000);
// TH1F * hnum_HTT_150 = new TH1F("hnum_HTT_150", "", 40, 0, 3000);
// TH1F * hnum_HTT_175 = new TH1F("hnum_HTT_175", "", 40, 0, 3000);
// TH1F * hnum_HTT_200 = new TH1F("hnum_HTT_200", "", 40, 0, 3000);
// TH1F * hnum_HTT_250 = new TH1F("hnum_HTT_250", "", 40, 0, 3000);
// TH1F * hnum_MHT_40 = new TH1F("hnum_MHT_40", "", 40, 0, 500);
// TH1F * hnum_MHT_60 = new TH1F("hnum_MHT_60", "", 40, 0, 500);
// TH1F * hnum_MHT_80 = new TH1F("hnum_MHT_80", "", 40, 0, 500);
// TH1F * hnum_MHT_100 = new TH1F("hnum_MHT_100", "", 40, 0, 500);
// TH1F * hEff_ETT_100 = new TH1F("hEff_ETT_100", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_ETT_125 = new TH1F("hEff_ETT_125", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_ETT_150 = new TH1F("hEff_ETT_150", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_ETT_175 = new TH1F("hEff_ETT_175", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_ETT_200 = new TH1F("hEff_ETT_200", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_ETT_250 = new TH1F("hEff_ETT_250", ";reco ETT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_MET_40 = new TH1F("hEff_MET_40", ";reco MET (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MET_60 = new TH1F("hEff_MET_60", ";reco MET (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MET_80 = new TH1F("hEff_MET_80", ";reco MET (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MET_100 = new TH1F("hEff_MET_100", ";reco MET (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_HTT_100 = new TH1F("hEff_HTT_100", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_HTT_125 = new TH1F("hEff_HTT_125", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_HTT_150 = new TH1F("hEff_HTT_150", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_HTT_175 = new TH1F("hEff_HTT_175", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_HTT_200 = new TH1F("hEff_HTT_200", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_HTT_250 = new TH1F("hEff_HTT_250", ";reco HTT (GeV);efficiency", 40, 0, 3000);
// TH1F * hEff_MHT_40 = new TH1F("hEff_MHT_40", ";reco MHT (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MHT_60 = new TH1F("hEff_MHT_60", ";reco MHT (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MHT_80 = new TH1F("hEff_MHT_80", ";reco MHT (GeV);efficiency", 40, 0, 500);
// TH1F * hEff_MHT_100 = new TH1F("hEff_MHT_100", ";reco MHT (GeV);efficiency", 40, 0, 500);


///////////////////////////////
//loop through all the events//
///////////////////////////////
//nevent = 10;
 for (Int_t i=0; i<nevent; i++){
			
		//load info for the event
    recoTree->GetEntry(i);
    metFilterTree->GetEntry(i);
    if (emuOn){l1emuTree->GetEntry(i);}
    if (hwOn){l1hwTree->GetEntry(i);}

    recoSums.ett = recoMet_->sumEt;
    recoSums.met = recoMet_->met;
    recoSums.htt = recoMet_->Ht;
    recoSums.mht = recoMet_->mHt;
   
    if (emuOn){
      l1Sums.ett = l1emu_->sumEt[0];
      l1Sums.met = l1emu_->sumEt[2];
      l1Sums.htt = l1emu_->sumEt[1];
      l1Sums.mht = l1emu_->sumEt[3];
      l1Sums.metPhi = l1emu_->sumPhi[2];
      l1Sums.mhtPhi = l1emu_->sumPhi[3];
    }

    if (hwOn){
      l1Sums.ett = l1hw_->sumEt[0];
      l1Sums.met = l1hw_->sumEt[2];
      l1Sums.htt = l1hw_->sumEt[1];
      l1Sums.mht = l1hw_->sumEt[3];
      l1Sums.metPhi = l1hw_->sumPhi[2];
      l1Sums.mhtPhi = l1hw_->sumPhi[3];
    }

    if(metFiltersOn==false || met_filter(recoMetFilter_->hbheNoiseFilter, recoMetFilter_->hbheNoiseIsoFilter, recoMetFilter_->cscTightHalo2015Filter, recoMetFilter_->ecalDeadCellTPFilter,
                              recoMetFilter_->goodVerticesFilter, recoMetFilter_->eeBadScFilter, recoMetFilter_->chHadTrackResFilter, recoMetFilter_->muonBadTrackFilter)){

      //fill the histograms
      hMETphi_reco->Fill(recoSums.metPhi);
      hMHTphi_reco->Fill(recoSums.mhtPhi);
  

      hdET_ETT->Fill((l1Sums.ett-recoSums.ett)/recoSums.ett);
      hETS_ETT->Fill(recoSums.ett,l1Sums.ett);



      hMET_l1->Fill(l1Sums.met);
      hMETphi_l1->Fill(l1Sums.metPhi);  
      hdET_MET->Fill((l1Sums.met-recoSums.met)/recoSums.met);
      hETS_MET->Fill(recoSums.met,l1Sums.met);
      hdPhi_MET->Fill(calc_dPHI(recoSums.metPhi,l1Sums.metPhi));

  		
      hHTT_l1->Fill(l1Sums.htt);
      hdET_HTT->Fill((l1Sums.htt-recoSums.htt)/recoSums.htt);
      hETS_HTT->Fill(recoSums.htt,l1Sums.htt);


      hMHT_l1->Fill(l1Sums.mht);
      hMHTphi_l1->Fill(l1Sums.mhtPhi);
      hdET_MHT->Fill((l1Sums.mht-recoSums.mht)/recoSums.mht);
      hETS_MHT->Fill(recoSums.mht,l1Sums.mht);
      hdPhi_MHT->Fill(calc_dPHI(recoSums.mhtPhi,l1Sums.mhtPhi));
       
      // trigger turnOns
      hden_ETT->Fill(recoSums.ett);
      hden_MET->Fill(recoSums.met);
      hden_HTT->Fill(recoSums.htt);
      hden_MHT->Fill(recoSums.mht);

      if(l1Sums.ett>100){hnum_ETT_100->Fill(recoSums.ett);}
      if(l1Sums.ett>125){hnum_ETT_125->Fill(recoSums.ett);}
      if(l1Sums.ett>150){hnum_ETT_150->Fill(recoSums.ett);}        
      if(l1Sums.ett>175){hnum_ETT_175->Fill(recoSums.ett);}
      if(l1Sums.ett>200){hnum_ETT_200->Fill(recoSums.ett);}
      if(l1Sums.ett>250){hnum_ETT_250->Fill(recoSums.ett);}

      if(l1Sums.met>40){hnum_MET_40->Fill(recoSums.met);}
      if(l1Sums.met>60){hnum_MET_60->Fill(recoSums.met);}
      if(l1Sums.met>80){hnum_MET_80->Fill(recoSums.met);}        
      if(l1Sums.met>100){hnum_MET_100->Fill(recoSums.met);}

      if(l1Sums.htt>100){hnum_HTT_100->Fill(recoSums.htt);}
      if(l1Sums.htt>125){hnum_HTT_125->Fill(recoSums.htt);}
      if(l1Sums.htt>150){hnum_HTT_150->Fill(recoSums.htt);}        
      if(l1Sums.htt>175){hnum_HTT_175->Fill(recoSums.htt);}
      if(l1Sums.htt>200){hnum_HTT_200->Fill(recoSums.htt);}
      if(l1Sums.htt>250){hnum_HTT_250->Fill(recoSums.htt);}

      if(l1Sums.mht>40){hnum_MHT_40->Fill(recoSums.mht);}
      if(l1Sums.mht>60){hnum_MHT_60->Fill(recoSums.mht);}
      if(l1Sums.mht>80){hnum_MHT_80->Fill(recoSums.mht);}        
      if(l1Sums.mht>100){hnum_MHT_100->Fill(recoSums.mht);}

    }//closes 'if' we pass the reco met filter
    if (i % 10000 == 0){
	    cout << i << " out of " << nevent << endl;}		
  ///////////////////////////////////
  }//closes loop through the events//
  ///////////////////////////////////

  //save the output ROOT file
  //write the histograms
  TFile g( outputFilename.c_str() , "new");

  //distributions
  // hMETphi_l1->Write();
  // hMHTphi_l1->Write();
  // hETT_l1->Write();
  // hMET_l1->Write();
  // hHTT_l1->Write();

  // hMHT_l1->Write();
  // hMETphi_reco->Write();
  // hMHTphi_reco->Write();
  // hETT_reco->Write();
  // hMET_reco->Write();
  // hHTT_reco->Write();
  // hMHT_reco->Write();

  // //resolutions
  // hdET_ETT->Write();
  // hdET_MET->Write();
  // hdET_HTT->Write();
  // hdET_MHT->Write();
  // hdPhi_MET->Write();
  // hdPhi_MHT->Write();
  // hETS_ETT->Write();
  // hETS_MET->Write();
  // hETS_HTT->Write();
  // hETS_MHT->Write();

  //turnOns
  hEff_ETT_100->Divide(hnum_ETT_100, hden_ETT);
  hEff_ETT_125->Divide(hnum_ETT_125, hden_ETT);
  hEff_ETT_150->Divide(hnum_ETT_150, hden_ETT);
  hEff_ETT_175->Divide(hnum_ETT_175, hden_ETT);
  hEff_ETT_200->Divide(hnum_ETT_200, hden_ETT);
  hEff_ETT_250->Divide(hnum_ETT_250, hden_ETT);
  hEff_ETT_100->Write();
  hEff_ETT_125->Write();
  hEff_ETT_150->Write();
  hEff_ETT_175->Write();
  hEff_ETT_200->Write();
  hEff_ETT_250->Write();
  hnum_ETT_100->Write();
  hnum_ETT_125->Write();
  hnum_ETT_150->Write();
  hnum_ETT_175->Write();
  hnum_ETT_200->Write();
  hnum_ETT_250->Write();
  hden_ETT->Write();

  hEff_MET_40->Divide(hnum_MET_40, hden_MET);
  hEff_MET_60->Divide(hnum_MET_60, hden_MET);
  hEff_MET_80->Divide(hnum_MET_80, hden_MET);
  hEff_MET_100->Divide(hnum_MET_100, hden_MET);
  hEff_MET_40->Write();
  hEff_MET_60->Write();
  hEff_MET_80->Write();
  hEff_MET_100->Write();
  hnum_MET_40->Write();
  hnum_MET_60->Write();
  hnum_MET_80->Write();
  hnum_MET_100->Write();
  hden_MET->Write();

  hEff_HTT_100->Divide(hnum_HTT_100, hden_HTT);
  hEff_HTT_125->Divide(hnum_HTT_125, hden_HTT);
  hEff_HTT_150->Divide(hnum_HTT_150, hden_HTT);
  hEff_HTT_175->Divide(hnum_HTT_175, hden_HTT);
  hEff_HTT_200->Divide(hnum_HTT_200, hden_HTT);
  hEff_HTT_250->Divide(hnum_HTT_250, hden_HTT);
  hEff_HTT_100->Write();
  hEff_HTT_125->Write();
  hEff_HTT_150->Write();
  hEff_HTT_175->Write();
  hEff_HTT_200->Write();
  hEff_HTT_250->Write();
  hnum_HTT_100->Write();
  hnum_HTT_125->Write();
  hnum_HTT_150->Write();
  hnum_HTT_175->Write();
  hnum_HTT_200->Write();
  hnum_HTT_250->Write();
  hden_HTT->Write();

  hEff_MHT_40->Divide(hnum_MHT_40, hden_MHT);
  hEff_MHT_60->Divide(hnum_MHT_60, hden_MHT);
  hEff_MHT_80->Divide(hnum_MHT_80, hden_MHT);
  hEff_MHT_100->Divide(hnum_MHT_100, hden_MHT);
  hEff_MHT_40->Write();
  hEff_MHT_60->Write();
  hEff_MHT_80->Write();
  hEff_MHT_100->Write();
  hnum_MHT_40->Write();
  hnum_MHT_60->Write();
  hnum_MHT_80->Write();
  hnum_MHT_100->Write();
  hden_MHT->Write();

}//closes the 'main' function

bool met_filter(Bool_t hbheNoiseFilter, Bool_t hbheNoiseIsoFilter, Bool_t cscTightHalo2015Filter, Bool_t ecalDeadCellTPFilter,
                Bool_t goodVerticesFilter, Bool_t eeBadScFilter, Bool_t chHadTrackResFilter, Bool_t muonBadTrackFilter){
  bool metPass;
  if (hbheNoiseFilter==1 && hbheNoiseIsoFilter==1 && cscTightHalo2015Filter==1 && ecalDeadCellTPFilter==1 
      && goodVerticesFilter==1 && eeBadScFilter==1 && chHadTrackResFilter==1 && muonBadTrackFilter==1){
    metPass = true;}//this met has passed
  else{metPass = false;}//this met has failed 
  return metPass;
}

double calc_dPHI(double phi1, double phi2){
  double dPhi = phi1 - phi2;
  if (dPhi>M_PI){dPhi=dPhi-2*M_PI;}
  if (dPhi<-M_PI){dPhi=dPhi+2*M_PI;}
  return dPhi;
}