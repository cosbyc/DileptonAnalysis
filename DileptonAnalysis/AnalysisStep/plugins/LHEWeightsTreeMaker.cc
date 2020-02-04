// Standard C++ includes
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>

// ROOT includes
#include <TTree.h>

// CMSSW framework includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

// CMSSW data formats
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

// Other relevant CMSSW includes
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

class LHEWeightsTreeMaker : public edm::EDAnalyzer {
    public:
        explicit LHEWeightsTreeMaker(const edm::ParameterSet&);
        ~LHEWeightsTreeMaker();
        
        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
    
    
    private:
        virtual void beginJob() override;
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;
        
        virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
        virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

        // Tokens -- get the event weight from GenEventInfoProduct, and cross-section from LHEEventProduct 
        const edm::EDGetTokenT<LHEEventProduct> lheInfoToken;
        const edm::EDGetTokenT<GenEventInfoProduct> genInfoToken;
        const edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;

        // Token to get the PU info in MC

        // Flag to decide whether to access and use the LHE/Gen products -- these may not be always available (e.g. we don't have them for Pythia samples)
        bool useLHEWeights;

        // Event weights
        double   wgtsign, wgtxsec;

        // To build template for PU reweighting
        unsigned char putrue;

        // TTree carrying the event weight information
        TTree* tree;
};

LHEWeightsTreeMaker::LHEWeightsTreeMaker(const edm::ParameterSet& iConfig): 
    lheInfoToken    (consumes<LHEEventProduct>                (iConfig.getParameter<edm::InputTag>("lheInfo"))),
    genInfoToken    (consumes<GenEventInfoProduct>            (iConfig.getParameter<edm::InputTag>("genInfo"))),
    pileupInfoToken (consumes<std::vector<PileupSummaryInfo> >(iConfig.getParameter<edm::InputTag>("pileupinfo"))),
    useLHEWeights(iConfig.getParameter<bool>("useLHEWeights"))
{
}


LHEWeightsTreeMaker::~LHEWeightsTreeMaker() {
}

void LHEWeightsTreeMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;
    using namespace std;

    // Get handles to all the requisite collections
    Handle<LHEEventProduct> lheInfoH;
    if (useLHEWeights) iEvent.getByToken(lheInfoToken, lheInfoH);

    Handle<GenEventInfoProduct> genInfoH;
    if (useLHEWeights) iEvent.getByToken(genInfoToken, genInfoH);

    Handle<vector<PileupSummaryInfo> > pileupInfoH;
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    
    // Read the event weights
    wgtsign = 1.0;
    wgtxsec = 1.0;
    if (useLHEWeights) {
        wgtsign = genInfoH->weight();
        wgtxsec = lheInfoH->originalXWGTUP();
    }
    
    // Pileup information
    putrue = 0;
    if (pileupInfoH.isValid()) {
        for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) {
            if (pileupInfo_iter->getBunchCrossing() == 0) putrue = (unsigned char)pileupInfo_iter->getTrueNumInteractions();
        }
    }

    // Fill the tree
    tree->Fill();

}


void LHEWeightsTreeMaker::beginJob() {
    // Access the TFileService
    edm::Service<TFileService> fs;

    // Create the TTree
    tree = fs->make<TTree>("tree"       , "tree");

    // Create the branches for the event weights
    tree->Branch("wgtsign"              , &wgtsign              , "wgtsign/D");
    tree->Branch("wgtxsec"              , &wgtxsec              , "wgtxsec/D");

    // Pileup info
    tree->Branch("putrue"               , &putrue               , "putrue/b" );
}

void LHEWeightsTreeMaker::endJob() {
}

void LHEWeightsTreeMaker::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
}

void LHEWeightsTreeMaker::endRun(edm::Run const&, edm::EventSetup const&) {
}

void LHEWeightsTreeMaker::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

void LHEWeightsTreeMaker::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

void LHEWeightsTreeMaker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(LHEWeightsTreeMaker);

