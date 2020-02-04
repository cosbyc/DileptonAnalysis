// Standard C++ includes
#include <memory>
#include <vector>
#include <iostream>

// ROOT includes
#include <TTree.h>
#include <TLorentzVector.h>
#include <TPRegexp.h>

// CMSSW framework includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

// CMSSW data formats
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenLumiInfoHeader.h"

// Other relevant CMSSW includes
#include "CommonTools/UtilAlgos/interface/TFileService.h" 
#include "CommonTools/CandUtils/interface/AddFourMomenta.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "CLHEP/Random/RandFlat.h"
#include "DileptonAnalysis/AnalysisStep/interface/CompositeCandMassResolution.h"


class HLTMuonTreeMaker : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns, edm::one::WatchLuminosityBlocks> {
	public:
		explicit HLTMuonTreeMaker(const edm::ParameterSet&);
		~HLTMuonTreeMaker();
		
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
	
	
	private:
        virtual void beginJob() override;
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;

        virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
        virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

        const edm::InputTag triggerResultsTag;
        
        const edm::EDGetTokenT<edm::TriggerResults>                    triggerResultsToken;
        const edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken;

        const edm::EDGetTokenT<std::vector<reco::Vertex> >             verticesToken;
        const edm::EDGetTokenT<std::vector<pat::Muon> >                muonsToken;

        const edm::EDGetTokenT<std::vector<PileupSummaryInfo> >        pileupInfoToken;
        const edm::EDGetTokenT<std::vector<reco::GenParticle> >        gensToken;
        const edm::EDGetTokenT<GenEventInfoProduct>                    genEvtInfoToken;

        std::vector<std::string> triggerPathsVector;
        std::map<std::string, int> triggerPathsMap;

        // Flags used in the analyzer
        // - applyHLTFilter : Fill the tree only when an event passes the set of "interesting" triggers
        // - isMC           : Is this a MC sample ?
        // - useLHEWeights  : If this is an MC sample, should we read the LHE level event weights
        bool                         applyHLTFilter, isMC, useLHEWeights;		 

        // Flag for the different types of triggers used in the analysis
        unsigned char                hlt;       

        // Cross-section and event weight information for MC events
        double                       xsec, wgt;

        // Generator-level information
        // 4-vector of genparticles, and their PDG IDs            
        std::vector<TLorentzVector>  gens;
        std::vector<char>            gid;

        // Pileup information
        unsigned                     putrue, nvtx;

        // Collection of muon 4-vectors, muon ID bytes, muon isolation values
        std::vector<TLorentzVector>  muons;
        std::vector<char>            mid;

        // TTree carrying the event weight information
        TTree* tree;

};

HLTMuonTreeMaker::HLTMuonTreeMaker(const edm::ParameterSet& iConfig): 
    triggerResultsTag        (iConfig.getParameter<edm::InputTag>("triggerresults")),
    triggerResultsToken      (consumes<edm::TriggerResults>                    (triggerResultsTag)),
    triggerObjectsToken      (consumes<pat::TriggerObjectStandAloneCollection> (iConfig.getParameter<edm::InputTag>("triggerobjects"))),
    verticesToken            (consumes<std::vector<reco::Vertex> >             (iConfig.getParameter<edm::InputTag>("vertices"))),
    muonsToken               (consumes<std::vector<pat::Muon> >                (iConfig.getParameter<edm::InputTag>("muons"))), 
    pileupInfoToken          (consumes<std::vector<PileupSummaryInfo> >        (iConfig.getParameter<edm::InputTag>("pileupinfo"))),
    gensToken                (consumes<std::vector<reco::GenParticle> >        (iConfig.getParameter<edm::InputTag>("gens"))),
    genEvtInfoToken          (consumes<GenEventInfoProduct>                    (iConfig.getParameter<edm::InputTag>("geneventinfo"))),
    applyHLTFilter           (iConfig.existsAs<bool>("applyHLTFilter")  ? iConfig.getParameter<bool>  ("applyHLTFilter")  : false),
    isMC                     (iConfig.existsAs<bool>("isMC")            ? iConfig.getParameter<bool>  ("isMC")            : false),
    useLHEWeights            (iConfig.existsAs<bool>("useLHEWeights")   ? iConfig.getParameter<bool>  ("useLHEWeights")   : false),
    xsec                     (iConfig.existsAs<double>("xsec")          ? iConfig.getParameter<double>("xsec") * 1000.0   : 1.)
{
	usesResource("TFileService");
}


HLTMuonTreeMaker::~HLTMuonTreeMaker() {
}

void HLTMuonTreeMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;
    using namespace reco;
    using namespace std;
    
    // Handles to the EDM content
    Handle<edm::TriggerResults> triggerResultsH;
    iEvent.getByToken(triggerResultsToken, triggerResultsH);
   
    Handle<pat::TriggerObjectStandAloneCollection> triggerObjectsH;
    iEvent.getByToken(triggerObjectsToken, triggerObjectsH);
    
    Handle<vector<Vertex> > verticesH;
    iEvent.getByToken(verticesToken, verticesH);
    
    Handle<vector<pat::Muon> > muonsH;
    iEvent.getByToken(muonsToken, muonsH);
    
    Handle<vector<PileupSummaryInfo> > pileupInfoH;
    if (isMC) iEvent.getByToken(pileupInfoToken, pileupInfoH);
    
    Handle<GenEventInfoProduct> genEvtInfoH;
    if (isMC && useLHEWeights) iEvent.getByToken(genEvtInfoToken, genEvtInfoH);
    
    Handle<vector<GenParticle> > gensH;
    if (isMC) iEvent.getByToken(gensToken, gensH);
    
    muons    .clear(); mid.clear();
    gens     .clear(); gid.clear();
    
    // Event information - MC weight, event ID (run, lumi, event) and so on
    wgt = 1.0;
    if (isMC && useLHEWeights && genEvtInfoH.isValid()) wgt = genEvtInfoH->weight(); 

    // Trigger info
    hlt = 0;

    // Which triggers fired
    for (size_t i = 0; i < triggerPathsVector.size(); i++) {
        if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;
        if (i == 0  && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hlt += 1; // Single muon trigger
        if (i == 1  && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hlt += 2; // Single muon trigger
    }

    bool triggered = false;
    if (hlt > 0) triggered = true;
    if (applyHLTFilter && !triggered) return;

    // Pileup information
    putrue = 0;
    if (isMC && pileupInfoH.isValid()) {
        for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) {
            if (pileupInfo_iter->getBunchCrossing() == 0) putrue = pileupInfo_iter->getTrueNumInteractions();
        }
    }

    nvtx  = verticesH->size();
    if (nvtx == 0) return;

    // Muon information
    const edm::TriggerNames& trigNames = iEvent.triggerNames(*triggerResultsH);
    for (pat::TriggerObjectStandAlone trgobj : *triggerObjectsH) {
        trgobj.unpackPathNames(trigNames);
        if (not trgobj.hasCollection("hltL3MuonCandidates")) continue;

        TLorentzVector m4;
        m4.SetPtEtaPhiM(trgobj.pt(), trgobj.eta(), trgobj.phi(), trgobj.mass());
        muons.push_back(m4);

        bool isMatchedToTightMu = false;
        bool isMatchedToIsoMu   = false;
        bool isMatchedToDST     = false;
        for (auto muons_iter = muonsH->begin(); muons_iter != muonsH->end(); ++muons_iter) {
            if (deltaR(trgobj.eta(), trgobj.phi(), muons_iter->eta(), muons_iter->phi()) > 0.1 || trgobj.pt()/muons_iter->pt() < 0.5 || trgobj.pt()/muons_iter->pt() > 2.0) continue;
            double muonisoval = max(0., muons_iter->pfIsolationR04().sumNeutralHadronEt + muons_iter->pfIsolationR04().sumPhotonEt - 0.5*muons_iter->pfIsolationR04().sumPUPt);
            muonisoval += muons_iter->pfIsolationR04().sumChargedHadronPt;
            if (muonisoval/muons_iter->pt() < 0.15) isMatchedToIsoMu = true;
            if (muons_iter->isTightMuon(verticesH->at(0)) && deltaR(trgobj.eta(), trgobj.phi(), muons_iter->eta(), muons_iter->phi()) < 0.1) isMatchedToTightMu = true;
        }
        if (trgobj.hasPathName("DST_DoubleMu3_Mass10_CaloScouting_PFScouting_v*", true, false) or trgobj.hasPathName("DST_DoubleMu3_Mass10_CaloScouting_PFScouting_v*", true, true)) isMatchedToDST = true;

        char midval = 1;
        if (isMatchedToTightMu) midval += 2;
        if (isMatchedToIsoMu)   midval += 4;
        if (isMatchedToDST)     midval += 8;
        midval *= trgobj.pdgId()/abs(trgobj.pdgId());
        mid.push_back(midval);
    }

    // GEN information
    if (isMC && gensH.isValid()) {
        for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) { 
            if (gens_iter->pdgId() ==  23 || abs(gens_iter->pdgId()) == 24 || gens_iter->pdgId() == 25) {
                TLorentzVector g4;
                g4.SetPtEtaPhiM(gens_iter->pt(), gens_iter->eta(), gens_iter->phi(), gens_iter->mass());
                gens.push_back(g4);
                gid.push_back(char(gens_iter->pdgId()));
            }
            if (abs(gens_iter->pdgId()) > 10 && abs(gens_iter->pdgId()) < 17 && gens_iter->fromHardProcessFinalState()) {
                TLorentzVector g4;
                g4.SetPtEtaPhiM(gens_iter->pt(), gens_iter->eta(), gens_iter->phi(), gens_iter->mass());
                gens.push_back(g4);
                gid.push_back(char(gens_iter->pdgId()));
            }
        }
    }

    tree->Fill();
}


void HLTMuonTreeMaker::beginJob() {
    // Access the TFileService
    edm::Service<TFileService> fs;

    // Create the TTree
    tree = fs->make<TTree>("tree"       , "tree");

    // Event weights
    if (isMC) {
    tree->Branch("xsec"                 , &xsec                          , "xsec/D"  );
    tree->Branch("wgt"                  , &wgt                           , "wgt/D"   );
    }
    
    // Triggers
    tree->Branch("hlt"                  , &hlt                           , "hlt/b"   );

    // Pileup info
    if (isMC)
    tree->Branch("putrue"               , &putrue                        , "putrue/I");
    tree->Branch("nvtx"                 , &nvtx                          , "nvtx/i"  );

    // Muon info
    tree->Branch("muons"                , "std::vector<TLorentzVector>"  , &muons    , 32000, 0);
    tree->Branch("mid"                  , "std::vector<char>"            , &mid      );

    // Gen info
    if (isMC) {
    tree->Branch("gens"                 , "std::vector<TLorentzVector>"  , &gens     , 32000, 0);
    tree->Branch("gid"                  , "std::vector<char>"            , &gid      );
    }
}

void HLTMuonTreeMaker::endJob() {
}

void HLTMuonTreeMaker::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // HLT paths
    triggerPathsVector.push_back("HLT_PFHT800_v");
    triggerPathsVector.push_back("DST_DoubleMu3_Mass10_CaloScouting_PFScouting_v");

    HLTConfigProvider hltConfig;
    bool changedConfig = false;
    hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changedConfig);

    for (size_t i = 0; i < triggerPathsVector.size(); i++) {
        triggerPathsMap[triggerPathsVector[i]] = -1;
    }

    for(size_t i = 0; i < triggerPathsVector.size(); i++){
        TPRegexp pattern(triggerPathsVector[i]);
        for(size_t j = 0; j < hltConfig.triggerNames().size(); j++){
            std::string pathName = hltConfig.triggerNames()[j];
            if(TString(pathName).Contains(pattern)){
                triggerPathsMap[triggerPathsVector[i]] = j;
            }
        }
    }

}

void HLTMuonTreeMaker::endRun(edm::Run const&, edm::EventSetup const&) {
}

void HLTMuonTreeMaker::beginLuminosityBlock(edm::LuminosityBlock const& iLumi, edm::EventSetup const&) {
}

void HLTMuonTreeMaker::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

void HLTMuonTreeMaker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	edm::ParameterSetDescription desc;
	desc.setUnknown();
	descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(HLTMuonTreeMaker);
