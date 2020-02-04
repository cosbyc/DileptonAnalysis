#include "sumwgt.h"

void trim(const char* treepath = "/media/Disk1/avartak/CMS/Data/Dileptons/DarkPhoton/tree_M160.root", const char* outfilename = "/media/Disk1/avartak/CMS/Data/Dileptons/DarkPhoton/trim_M160.root", bool isMC = true) {

    TFileCollection fc("fc");
    fc.Add(treepath);

    TChain* chain = new TChain("mmtree/tree");
    chain->AddFileInfoList(fc.GetList());

    // PU reweighting  
    TH1D*  histoPUData;
    TFile* fileInputData;
    
    fileInputData = TFile::Open("../data/pudata.root");
    histoPUData   = (TH1D*) fileInputData->Get("pileup");
    
    histoPUData->SetName("histoPUData");
    histoPUData->Scale(1./histoPUData->Integral());
    
    TH1D* histoPUMC = (TH1D*) histoPUData->Clone("histoPUMC");
    chain->Draw("putrue >> histoPUMC","","goff");
    histoPUMC->Scale(1./histoPUMC->Integral());
    
    TH1D* puRatio = (TH1D*) histoPUData->Clone("histoRatio");
    puRatio->Divide(histoPUMC);
    
    // PU reweighting based on the number of vertices
    TFile purwtfile("../data/purwt.root");
    TH1F* purwthist = (TH1F*)purwtfile.Get("puhist");


    // Muon ID, isolation scale factors
    TFile muoidfile("../data/muonIDIsoSF.root");
    TH2F* muoidhist = (TH2F*)muoidfile.Get("scalefactors_MuonMediumId_Muon");
    TH2F* muisohist = (TH2F*)muoidfile.Get("scalefactors_Iso_MuonMediumId");

    TTreeReader reader(chain);

    TTreeReaderValue<double>                       xsec     (reader, "xsec"       );
    TTreeReaderValue<double>                       wgt      (reader, "wgt"        );
    TTreeReaderValue<unsigned char>                vert     (reader, "nvtx"       );
    TTreeReaderValue<unsigned char>                putrue   (reader, "putrue"     );
    TTreeReaderValue<std::vector<unsigned char> >  m1idx    (reader, "m1idx"      );
    TTreeReaderValue<std::vector<unsigned char> >  m2idx    (reader, "m2idx"      );
    TTreeReaderValue<std::vector<double> >         masserr  (reader, "masserr"    );
    TTreeReaderValue<std::vector<TLorentzVector> > muons    (reader, "muons"      );
    TTreeReaderValue<std::vector<double> >         miso     (reader, "miso"       );
    TTreeReaderValue<std::vector<char> >           mid      (reader, "mid"        );
    TTreeReaderValue<unsigned char>                hlt1m    (reader, "hltsinglemu");
    TTreeReaderValue<std::vector<TLorentzVector> > jets     (reader, "jets"       );
    TTreeReaderValue<std::vector<char> >           jid      (reader, "jid"        );
    TTreeReaderValue<std::vector<double> >         jbtag    (reader, "jbtag"      );
    TTreeReaderValue<double>                       etm      (reader, "t1met"      );
    TTreeReaderValue<double>                       etmphi   (reader, "t1metphi"   );

    TFile* outfile = new TFile(outfilename, "RECREATE");
    TTree* outtree = new TTree("tree", "tree");

    double        wgtsum    = isMC ? sumwgt(treepath) : 1.0;
    double        mcweight  =  1.0;
    double        puweight  =  1.0;
    double        exweight  =  1.0;

    double        m1pt      =  0.0;        
    double        m1eta     =  0.0;        
    double        m1phi     =  0.0;        
    double        m2pt      =  0.0;        
    double        m2eta     =  0.0;        
    double        m2phi     =  0.0;        
    double        mmpt      =  0.0;        
    double        mmeta     =  0.0;        
    double        mmphi     =  0.0;        
    double        mass      =  0.0;        
    double        merr      =  0.0;        

    double        met       =  0.0;
    double        metphi    =  0.0;

    unsigned char njets     =  0  ;        
    unsigned char nbjets    =  0  ;        

    char          m1id      =  0  ;
    char          m2id      =  0  ;
    unsigned char nvtx      =  0  ;

    outtree->Branch("mcweight" , &mcweight , "mcweight/D" );
    outtree->Branch("puweight" , &puweight , "puweight/D" );
    outtree->Branch("exweight" , &exweight , "exweight/D" );

    outtree->Branch("m1pt"     , &m1pt     , "m1pt/D"      );
    outtree->Branch("m1eta"    , &m1eta    , "m1eta/D"     );
    outtree->Branch("m1phi"    , &m1phi    , "m1phi/D"     );
    outtree->Branch("m1id"     , &m1id     , "m1id/B"      );
    outtree->Branch("m2pt"     , &m2pt     , "m2pt/D"      );
    outtree->Branch("m2eta"    , &m2eta    , "m2eta/D"     );
    outtree->Branch("m2phi"    , &m2phi    , "m2phi/D"     );
    outtree->Branch("m2id"     , &m2id     , "m2id/B"      );
    outtree->Branch("mmpt"     , &mmpt     , "mmpt/D"      );
    outtree->Branch("mmeta"    , &mmeta    , "mmeta/D"     );
    outtree->Branch("mmphi"    , &mmphi    , "mmphi/D"     );
    outtree->Branch("mass"     , &mass     , "mass/D"      );
    outtree->Branch("merr"     , &merr     , "merr/D"      );

    outtree->Branch("met"      , &met      , "met/D"       );
    outtree->Branch("metphi"   , &metphi   , "metphi/D"    );

    outtree->Branch("njets"    , &njets    , "njets/b"     );
    outtree->Branch("nbjets"   , &nbjets   , "nbjets/b"    );

    outtree->Branch("nvtx"     , &nvtx     , "nvtx/b"      );

    while(reader.Next()) {

        // Require the event to fire the single muon trigger 
        if (*hlt1m < 1) continue;

        // Number of reconstructed vertices in the event
        nvtx = *vert;
        unsigned char nvert = nvtx;
        if (nvert > 40) nvert = 40;

        // Require two OS muons passing the medium ID and loose isolation 
        // Take the leading combination (in terms muon pT) in case of multiple possible dimuon combinations
        int idx1 = -1;
        int idx2 = -1;
        for (size_t i = 0; i < muons->size(); i++) {
            if (idx1 >= 0 && idx2 >= 0) continue;
    
            unsigned char id = abs((*mid)[i]);

            if ((id & 4) == 0) continue;
            if (miso->at(i) > 0.25) continue;

            if (idx1 < 0) idx1 = i;
            else {
                if (mid->at(idx1) * mid->at(i) < 0) idx2 = i;
            }
        }
        if (idx1 < 0 || idx2 < 0) continue;

        m1id = 1;
        m2id = 1;

        // Require at least one of the muons to fire the trigger and have pT > 26 GeV (single muon trigger plateau)
        if (((unsigned char)abs(mid->at(idx1)) & 16) > 0) m1id += 2;
        if (((unsigned char)abs(mid->at(idx2)) & 16) > 0) m2id += 2;

        bool triggervalid = false;
        if (m1id == 3 &&  muons->at(idx1).Pt() > 30.0) triggervalid = true;
        if (m2id == 3 &&  muons->at(idx2).Pt() > 30.0) triggervalid = true;
        if (not triggervalid) continue;

        // Saving muon ID information for the two muons
        // Sign of the ID value corresponds to the muon charge
        // ID value is 1 if the muon only passes the ID/iso requirements
        // ID value is 3 if the muon also fires the HLT
        if (mid->at(idx1) < 0) m1id *= -1;
        if (mid->at(idx2) < 0) m2id *= -1;

        // Kinematic information of the two muons
        m1pt   = muons->at(idx1).Pt();
        m1eta  = muons->at(idx1).Eta();
        m1phi  = muons->at(idx1).Phi();

        m2pt   = muons->at(idx2).Pt();
        m2eta  = muons->at(idx2).Eta();
        m2phi  = muons->at(idx2).Phi();

        TLorentzVector mm;
        mm += muons->at(idx1);
        mm += muons->at(idx2);

        mmpt   = mm.Pt();
        mmeta  = mm.Eta();
        mmphi  = mm.Phi();
        mass   = mm.M();

        merr   = -1.0;
        for (size_t i = 0; i < masserr->size(); i++) {
            if (m1idx->at(i) == idx1 && m2idx->at(i) == idx2) merr = masserr->at(i);
        } 
        //if (mass < 10.0) continue;

        // Jet information
        // Jet is required to have pT > 30 GeV, |eta| < 4.7, and pass the loose jet ID
        njets  = 0;
        nbjets = 0;
        for (size_t i = 0; i < jets->size(); i++) {
            if ((jid->at(i) & 1) == 0  ) continue;
            if (jets->at(i).Pt() < 30.0) continue;
            if (fabs(jets->at(i).Eta()) > 4.7) continue;
            if (jets->at(i).DeltaR(muons->at(idx1)) < 0.4) continue;
            if (jets->at(i).DeltaR(muons->at(idx2)) < 0.4) continue;

            njets++;
            if (jbtag->at(i) > 0.8484)         nbjets++;
        }

        // MET information
        met       = *etm;
        metphi    = *etmphi;

        // Computing MC event weights
        double pt1  = muons->at(idx1).Pt();
        double pt2  = muons->at(idx2).Pt();

        double eta1 = fabs(muons->at(idx1).Eta());
        double eta2 = fabs(muons->at(idx2).Eta());

        if (pt1 >= 120.0) pt1 = 119.9;
        if (pt2 >= 120.0) pt2 = 119.9;

        if (pt1 <=  20.0) pt1 =  20.1;
        if (pt2 <=  20.0) pt2 =  20.1;

        mcweight = 1.0;
        puweight = 1.0;
        exweight = 1.0;

        if (isMC) {
            puweight *= puRatio->GetBinContent(puRatio->FindBin(*putrue));

            exweight *= muoidhist->GetBinContent(muoidhist->FindBin(eta1, pt1));
            exweight *= muoidhist->GetBinContent(muoidhist->FindBin(eta2, pt2));
            exweight *= muisohist->GetBinContent(muisohist->FindBin(eta1, pt1));
            exweight *= muisohist->GetBinContent(muisohist->FindBin(eta2, pt2));

            mcweight  = 35.9 * (*wgt) * (*xsec) / wgtsum;
        }

        // Fill the tree
        outtree->Fill();

    }

    outtree->Write();

    outfile->Close();

}
