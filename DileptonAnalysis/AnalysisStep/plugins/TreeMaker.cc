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
#include "DataFormats/Candidate/interface/ShallowCloneCandidate.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
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

//For Trigger
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "L1Trigger/L1TGlobal/interface/L1TGlobalUtil.h"
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionData.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionEvaluator.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionParser.h"

// For vertex calculation
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/PointingKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/GeomPropagators/interface/AnalyticalImpactPointExtrapolator.h"


typedef reco::Candidate::LorentzVector LorentzVector;
// Defining Kalman Fit Result
struct KalmanVertexFitResult{
  float vtxProb;
  bool  valid;
  std::vector<LorentzVector> refitVectors;
  GlobalPoint position;
  GlobalError err;
  float lxy, lxyErr, sigLxy, chiSq;

  KalmanVertexFitResult():vtxProb(-1.0),valid(false),lxy(-1.0),lxyErr(-1.0),sigLxy(-1.0),chiSq(-1.0){}

  float mass() const
  {
    if (not valid) return -1.0;
    LorentzVector p4;
    for (auto v: refitVectors)
      p4 += v;
    return p4.mass();
  }
  void postprocess(const reco::BeamSpot& bs)
  {
    if (not valid) return;
    // position of the beam spot at a given z value (it takes into account the dxdz and dydz slopes)
    reco::BeamSpot::Point bs_at_z(bs.position(position.z()));
    GlobalPoint xy_displacement(position.x() - bs_at_z.x(),
				position.y() - bs_at_z.y(),
				0);
    lxy = xy_displacement.perp();
    lxyErr = sqrt(err.rerr(xy_displacement));
    if (lxyErr > 0) sigLxy = lxy/lxyErr;
  }
};

LorentzVector makeLorentzVectorFromPxPyPzM(double px, double py, double pz, double m){
  double p2 = px*px+py*py+pz*pz;
  return LorentzVector(px,py,pz,sqrt(p2+m*m));
}

using namespace edm;
using namespace reco;
using namespace std;
using namespace l1t;

class TreeMaker : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns, edm::one::WatchLuminosityBlocks> {
	public:
		explicit TreeMaker(const edm::ParameterSet&);
		~TreeMaker();
		
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
	
	
	private:

               
   // Original 
        virtual void beginJob() override;
        virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;

        virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
        virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
        virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // New members from MIT code

        bool isGoodMuon(const pat::Muon& muon);
        KalmanVertexFitResult vertexWithKalmanFitter( std::vector<const reco::Track*> trks, std::vector<float> masses);
        KalmanVertexFitResult vertexMuonsWithKalmanFitter(const pat::Muon& muon1, const pat::Muon& muon2);


        char getJetID(const pat::JetRef&);
        bool isMediumMuon(const pat::MuonRef&);

        const edm::InputTag triggerResultsTag;
        const edm::InputTag filterResultsTag;
        
        const edm::EDGetTokenT<edm::TriggerResults>                    triggerResultsToken;
        const edm::EDGetTokenT<edm::TriggerResults>                    filterResultsToken;
        const edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken;

        const edm::EDGetTokenT<bool>                                   flagBadMuonToken;
        const edm::EDGetTokenT<bool>                                   flagBadHadronToken;

        const edm::EDGetTokenT<std::vector<reco::Vertex> >             verticesToken;
        const edm::EDGetTokenT<std::vector<pat::Muon> >                muonsToken;
    //const edm::EDGetTokenT<std::vector<pat::Electron> >            electronsToken;
        const edm::EDGetTokenT<std::vector<pat::Jet> >                 jetsToken;
        const edm::EDGetTokenT<std::vector<pat::MET> >                 metToken;

        const edm::EDGetTokenT<std::vector<PileupSummaryInfo> >        pileupInfoToken;
        const edm::EDGetTokenT<std::vector<reco::GenParticle> >        gensToken;
        const edm::EDGetTokenT<GenEventInfoProduct>                    genEvtInfoToken;

    //  const edm::EDGetTokenT<edm::ValueMap<bool> >                   electronVetoIdMapToken;
    //  const edm::EDGetTokenT<edm::ValueMap<bool> >                   electronLooseIdMapToken;
    //  const edm::EDGetTokenT<edm::ValueMap<bool> >                   electronMediumIdMapToken;
    //  const edm::EDGetTokenT<edm::ValueMap<bool> >                   electronTightIdMapToken;

        std::vector<std::string>   triggerPathsVector;
        std::map<std::string, int> triggerPathsMap;
        std::vector<std::string>   filterPathsVector;
        std::map<std::string, int> filterPathsMap;

        // Flags used in the analyzer
        // - applyHLTFilter : Fill the tree only when an event passes the set of "interesting" triggers
        // - isMC           : Is this a MC sample ?
        // - useLHEWeights  : If this is an MC sample, should we read the LHE level event weights
        // - useMedium2016  : Use medium muon ID tuned for the HIP affected data
        bool                         applyHLTFilter, applyDimuonFilter, isMC, useLHEWeights, useMediumID2016, addEventInfo, filterHToMuMu;		 

        // Event coordinates
        unsigned                     event, run, lumSec;

        // Flags for the different types of triggers used in the analysis
        // For now we are interested in events passing either the single or double lepton triggers
        unsigned char                hltsinglemu, hltdoublemu, hltsingleel, hltdoubleel;       

        // Flags for various event filters
        // These are mostly relevant for MET based analyses but does not hurt to have them
        unsigned char                flags;

        // Cross-section and event weight information for MC events
        double                       xsec, wgt;

        // Generator-level information
        // 4-vector of genparticles, and their PDG IDs            
        std::vector<TLorentzVector>  gens;
        std::vector<char>            gid;
        std::vector<double>            gvtx;

        // Pileup information
        unsigned char                putrue, nvtx;

        // Collection of muon 4-vectors, muon ID bytes, muon isolation values
        std::vector<TLorentzVector>  muons;
        std::vector<int>            mid;
        std::vector<bool>            midloose;
        std::vector<bool>            midsoft;
        std::vector<bool>            midmedium;
        std::vector<bool>            midtight;
        std::vector<double>          miso;

        // Collection of dimuon 4-vectors, dimuon mass errors, and indices of the muon daughters in the muon vectors, 
        std::vector<unsigned char>   m1idx;
        std::vector<unsigned char>   m2idx;
        std::vector<double>          masserr;

        // Collection of electron 4-vectors, electron ID bytes
        //std::vector<TLorentzVector>  electrons;
        //std::vector<char>            eid;

        // Raw and Type-1 MET
        double                       met, metphi, t1met, t1metphi, t1metjecup, t1metjecupphi, t1metjecdn, t1metjecdnphi, t1metjerup, t1metjerupphi, t1metjerdn, t1metjerdnphi, t1metuncup, t1metuncupphi, t1metuncdn, t1metuncdnphi;

        // Collection of jet 4-vectors, jet ID bytes and b-tag discriminant values
        std::vector<TLorentzVector>  jets;
        std::vector<double>          jbtag;
        std::vector<char>            jid;
  
        // Vertex info
        std::vector<double>          vtxProb;
        std::vector<bool>            valid;
        std::vector<double>          lxy;    
        std::vector<double>          lxyErr;
        std::vector<double>          sigLxy;
        std::vector<double>          chiSq; 
  
        std::vector<double>          m1Impact;
        std::vector<double>          m2Impact;
        
        // New Trigger info
        std::vector<string>          triggersPassed;
        string fullList;

        // TTree carrying the event weight information
        TTree* tree;

        // Sorters to order object collections in decreasing order of pT
        template<typename T> 
        class PatPtSorter {
        public:
            bool operator()(const T& i, const T& j) const {
                return (i->pt() > j->pt());
            }
        };
        
        PatPtSorter<pat::MuonRef>     muonSorter;
       //PatPtSorter<pat::ElectronRef> electronSorter;
        PatPtSorter<pat::JetRef>      jetSorter;

       //unsigned char                trig;       
        unsigned int                trig;       

        std::vector<std::string>     l1Seeds_;
        edm::InputTag                algInputTag_;       
        edm::InputTag                extInputTag_;       
        edm::EDGetToken              algToken_;
        edm::EDGetToken              extToken_;
        l1t::L1TGlobalUtil          *l1GtUtils_;
        std::vector<bool> l1Result_;
        
        float MuonMass_    = 0.10565837;
        edm::ESHandle<TransientTrackBuilder> theTTBuilder_;
        edm::ESHandle<MagneticField> bFieldHandle_;

        edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
        const reco::BeamSpot* beamSpot_;       

  //        edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
        
 
};

TreeMaker::TreeMaker(const edm::ParameterSet& iConfig): 

    triggerResultsTag        (iConfig.getParameter<edm::InputTag>("triggerresults")),
    filterResultsTag         (iConfig.getParameter<edm::InputTag>("filterresults")),
    triggerResultsToken      (consumes<edm::TriggerResults>                    (triggerResultsTag)),
    filterResultsToken       (consumes<edm::TriggerResults>                    (filterResultsTag)),
    triggerObjectsToken      (consumes<pat::TriggerObjectStandAloneCollection> (iConfig.getParameter<edm::InputTag>("triggerobjects"))),
    flagBadMuonToken         (consumes<bool>                                   (iConfig.getParameter<edm::InputTag>("badmuon"))),
    flagBadHadronToken       (consumes<bool>                                   (iConfig.getParameter<edm::InputTag>("badhadron"))),
    verticesToken            (consumes<std::vector<reco::Vertex> >             (iConfig.getParameter<edm::InputTag>("vertices"))),
    muonsToken               (consumes<std::vector<pat::Muon> >                (iConfig.getParameter<edm::InputTag>("muons"))), 
//  electronsToken           (consumes<std::vector<pat::Electron> >            (iConfig.getParameter<edm::InputTag>("electrons"))), 
    jetsToken                (consumes<std::vector<pat::Jet> >                 (iConfig.getParameter<edm::InputTag>("jets"))),
    metToken                 (consumes<std::vector<pat::MET> >                 (iConfig.getParameter<edm::InputTag>("met"))),
    pileupInfoToken          (consumes<std::vector<PileupSummaryInfo> >        (iConfig.getParameter<edm::InputTag>("pileupinfo"))),
    gensToken                (consumes<std::vector<reco::GenParticle> >        (iConfig.getParameter<edm::InputTag>("gens"))),
    genEvtInfoToken          (consumes<GenEventInfoProduct>                    (iConfig.getParameter<edm::InputTag>("geneventinfo"))),
//  electronVetoIdMapToken   (consumes<edm::ValueMap<bool> >                   (iConfig.getParameter<edm::InputTag>("electronidveto"))),
//  electronLooseIdMapToken  (consumes<edm::ValueMap<bool> >                   (iConfig.getParameter<edm::InputTag>("electronidloose"))),
//  electronMediumIdMapToken (consumes<edm::ValueMap<bool> >                   (iConfig.getParameter<edm::InputTag>("electronidmedium"))),
//  electronTightIdMapToken  (consumes<edm::ValueMap<bool> >                   (iConfig.getParameter<edm::InputTag>("electronidtight"))),
    applyHLTFilter           (iConfig.existsAs<bool>("applyHLTFilter")    ? iConfig.getParameter<bool>  ("applyHLTFilter")    : false),
    applyDimuonFilter        (iConfig.existsAs<bool>("applyDimuonFilter") ? iConfig.getParameter<bool>  ("applyDimuonFilter") : false),
    isMC                     (iConfig.existsAs<bool>("isMC")              ? iConfig.getParameter<bool>  ("isMC")              : false),
    useLHEWeights            (iConfig.existsAs<bool>("useLHEWeights")     ? iConfig.getParameter<bool>  ("useLHEWeights")     : false),
    useMediumID2016          (iConfig.existsAs<bool>("useMediumID2016")   ? iConfig.getParameter<bool>  ("useMediumID2016")   : false),
    addEventInfo             (iConfig.existsAs<bool>("addEventInfo")      ? iConfig.getParameter<bool>  ("addEventInfo")      : false),
    filterHToMuMu            (iConfig.existsAs<bool>("filterHToMuMu")     ? iConfig.getParameter<bool>  ("filterHToMuMu")     : false),
    xsec                     (iConfig.existsAs<double>("xsec")            ? iConfig.getParameter<double>("xsec") * 1000.0     : 1.),
    l1Seeds_                 (iConfig.getParameter<std::vector<std::string> >("l1Seeds")),
    algInputTag_             (iConfig.getParameter<InputTag>("AlgInputTag")),
    extInputTag_             (iConfig.getParameter<InputTag>("ExtInputTag")),
    algToken_                (consumes<BXVector<GlobalAlgBlk> >(algInputTag_)),
    extToken_                (consumes<BXVector<GlobalExtBlk> >(extInputTag_)),
    beamSpotToken_( consumes<reco::BeamSpot> ( iConfig.getParameter<edm::InputTag>( "beamSpot" ) ) ),
    beamSpot_(nullptr)


{
	usesResource("TFileService");
    l1GtUtils_  = new L1TGlobalUtil(iConfig, consumesCollector(), *this, algInputTag_, extInputTag_);
}


TreeMaker::~TreeMaker() {
}

void TreeMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    //cout << "1st test point" << endl;
    iSetup.get<IdealMagneticFieldRecord>().get(bFieldHandle_);
    //cout << "2nd test point" << endl;
    iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theTTBuilder_);
    //cout << "3rd test point"  << endl;

    edm::Handle<reco::BeamSpot> beamSpotHandle;
    
    iEvent.getByToken(beamSpotToken_, beamSpotHandle);    
    beamSpot_ = beamSpotHandle.product();
 

    // Handles to the EDM content
    Handle<edm::TriggerResults> triggerResultsH;
    iEvent.getByToken(triggerResultsToken, triggerResultsH);
   
    Handle<TriggerResults> filterResultsH;
    iEvent.getByToken(filterResultsToken, filterResultsH);
 
    Handle<pat::TriggerObjectStandAloneCollection> triggerObjectsH;
    iEvent.getByToken(triggerObjectsToken, triggerObjectsH);
    
    Handle<bool> flagBadMuonH;
    iEvent.getByToken(flagBadMuonToken, flagBadMuonH);
    
    Handle<bool> flagBadHadronH;
    iEvent.getByToken(flagBadHadronToken, flagBadHadronH);
    
    Handle<vector<Vertex> > verticesH;
    iEvent.getByToken(verticesToken, verticesH);
    
    Handle<vector<pat::Muon> > muonsH;
    iEvent.getByToken(muonsToken, muonsH);
    
    Handle<vector<pat::Jet> > jetsH;
    iEvent.getByToken(jetsToken, jetsH);
    
    Handle<vector<pat::MET> > metH;
    iEvent.getByToken(metToken, metH);

    /*
    Handle<vector<pat::Electron> > electronsH;
    iEvent.getByToken(electronsToken, electronsH);
    
    Handle<ValueMap<bool> > electronVetoIdH;
    iEvent.getByToken(electronVetoIdMapToken, electronVetoIdH);
    
    Handle<ValueMap<bool> > electronLooseIdH;
    iEvent.getByToken(electronLooseIdMapToken, electronLooseIdH);
    
    Handle<ValueMap<bool> > electronMediumIdH;
    iEvent.getByToken(electronMediumIdMapToken, electronMediumIdH);
    
    Handle<ValueMap<bool> > electronTightIdH;
    iEvent.getByToken(electronTightIdMapToken, electronTightIdH);
    */

    Handle<vector<PileupSummaryInfo> > pileupInfoH;
    if (isMC) iEvent.getByToken(pileupInfoToken, pileupInfoH);
    
    Handle<GenEventInfoProduct> genEvtInfoH;
    if (isMC && useLHEWeights) iEvent.getByToken(genEvtInfoToken, genEvtInfoH);
    
    Handle<vector<GenParticle> > gensH;
    if (isMC) iEvent.getByToken(gensToken, gensH);


    // new trigger collection
    edm::Handle<edm::TriggerResults> trigger;
    iEvent.getByToken(triggerResultsToken,trigger);
    const edm::TriggerNames trigNames = iEvent.triggerNames(*trigger);


    Handle<BXVector<GlobalAlgBlk>> alg;
    iEvent.getByToken(algToken_,alg);

    Handle<BXVector<GlobalExtBlk>> ext;
    iEvent.getByToken(extToken_,ext);

    l1GtUtils_->retrieveL1(iEvent,iSetup,algToken_);

    const std::vector<std::pair<std::string, bool> > initialDecisions = l1GtUtils_->decisionsInitial();
    const std::vector<std::pair<std::string, bool> > intermDecisions = l1GtUtils_->decisionsInterm();
    const std::vector<std::pair<std::string, bool> > finalDecisions = l1GtUtils_->decisionsFinal();
    const std::vector<std::pair<std::string, int> >  prescales = l1GtUtils_->prescales();
    const std::vector<std::pair<std::string, std::vector<int>> > masks = l1GtUtils_->masks();

    //cout << "\n===================================== Trigger Results for BX=0 =============================================\n" << endl;
    //cout << "    Bit                  Algorithm Name                  Init    aBXM  Final   PS Factor     Num Bx Masked" << endl;
    //cout << "============================================================================================================" << endl;

    for(unsigned int i=0; i<initialDecisions.size(); i++) {
        
      // get the name and trigger result
      std::string name = (initialDecisions.at(i)).first;
      if(name == "NULL") continue;

      //bool found = false;
      //for( unsigned int iseed = 0; iseed < l1Seeds_.size(); iseed++ ) {
      //    if (name == string(l1Seeds_[iseed])) found = true;
      //}  
      //if (!found) continue;

      bool resultInit = (initialDecisions.at(i)).second;

      // get prescaled and final results (need some error checking here)
      bool resultInterm = (intermDecisions.at(i)).second;
      bool resultFin = (finalDecisions.at(i)).second;

      // get the prescale and mask (needs some error checking here)
      int prescale = (prescales.at(i)).second;
      std::vector<int> mask    = (masks.at(i)).second;

      //cout << std::dec << setfill(' ') << "   " << setw(5) << i << "   " << setw(40) << name.c_str() << "   " << setw(7) << resultInit << setw(7) << resultInterm << setw(7) << resultFin << setw(10) << prescale << setw(11) << mask.size() << endl;
    }

    l1Result_.clear();
    for( unsigned int iseed = 0; iseed < l1Seeds_.size(); iseed++ ) {
        bool l1htbit = 0;
        l1GtUtils_->getFinalDecisionByName(string(l1Seeds_[iseed]), l1htbit);
        //cout<<string(l1Seeds_[iseed])<<" "<<l1htbit<<endl;
        l1Result_.push_back( l1htbit );
    }

    
    m1idx.clear()    ; m2idx.clear(); masserr.clear();
    muons    .clear(); mid.clear(); miso.clear();
    //electrons.clear(); eid.clear();
    jets     .clear(); jid.clear(); jbtag.clear();
    gens     .clear(); gid.clear(); gvtx.clear();
    midloose.clear();
    midsoft.clear();
    midmedium.clear();
    midtight.clear();

    vtxProb.clear();
    valid.clear();
    lxy.clear();
    lxyErr.clear();
    sigLxy.clear();
    chiSq.clear();

    m1Impact.clear();
    m2Impact.clear();
    
    triggersPassed.clear();
    fullList.clear();


    // Event information - MC weight, event ID (run, lumi, event) and so on
    wgt = 1.0;
    if (isMC && useLHEWeights && genEvtInfoH.isValid()) wgt = genEvtInfoH->weight(); 

    event = iEvent.id().event();
    run   = iEvent.id().run();
    lumSec  = iEvent.luminosityBlock();



    unsigned int _tSize = trigger->size();
    // create a string with all passing trigger names
    for (unsigned int i=0; i<_tSize; ++i) {
      std::string triggerName = trigNames.triggerName(i);
      if (strstr(triggerName.c_str(),"_step")) continue;
      if (strstr(triggerName.c_str(),"MC_")) continue;
      if (strstr(triggerName.c_str(),"AlCa_")) continue;
      if (strstr(triggerName.c_str(),"DST_")) continue;
      if (strstr(triggerName.c_str(),"HLT_HI")) continue;
      if (strstr(triggerName.c_str(),"HLT_Physics")) continue;
      if (strstr(triggerName.c_str(),"HLT_Random")) continue;
      if (strstr(triggerName.c_str(),"HLT_ZeroBias")) continue;
      if (strstr(triggerName.c_str(),"HLT_IsoTrack")) continue;
      if (strstr(triggerName.c_str(),"Hcal")) continue;
      if (strstr(triggerName.c_str(),"Ecal")) continue;
      if (!strstr(triggerName.c_str(),"mu")&&!strstr(triggerName.c_str(),"Mu")) continue;
      //cout << triggerName;
      if (trigger->accept(i)) fullList += triggerName;
    }
    triggersPassed.push_back(fullList);

    // Trigger info
    hltsinglemu = 0;
    hltdoublemu = 0;
    hltsingleel = 0;
    hltdoubleel = 0;

    trig=0;
    // Which triggers fired
    for (size_t i = 0; i < triggerPathsVector.size(); i++) {
        if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;
        if (i == 0  && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) trig +=   1; // DST_DoubleMu3_noVtx_CaloScouting
        //if (i == 1  && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) trig +=   2; // DST_L1DoubleMu_CaloScouting_PFScouting
    }

    /*
    bool triggered = false;
    if (hltsinglemu  > 0) triggered = true;
    if (hltdoublemu  > 0) triggered = true;
    if (hltsingleel  > 0) triggered = true;
    if (hltdoubleel  > 0) triggered = true;
    if (applyHLTFilter && !triggered) return;
    */

    // MET filter info
    unsigned char flagvtx       = 1 * 1;
    unsigned char flaghalo      = 1 * 2;
    unsigned char flaghbhe      = 1 * 4;
    unsigned char flaghbheiso   = 1 * 8;
    unsigned char flagecaltp    = 1 * 16;
    unsigned char flageebadsc   = 1 * 32;
    unsigned char flagbadmuon   = (*flagBadMuonH   ? 0 : 1) * 64;
    unsigned char flagbadhad    = (*flagBadHadronH ? 0 : 1) * 128;

    // Which MET filters passed
    for (size_t i = 0; i < filterPathsVector.size(); i++) {
        if (filterPathsMap[filterPathsVector[i]] == -1) continue;
        if (i == 0  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flagvtx       = 0; // goodVertices
        if (i == 1  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flaghalo      = 0; // CSCTightHaloFilter
        if (i == 2  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flaghbhe      = 0; // HBHENoiseFilter
        if (i == 3  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flaghbheiso   = 0; // HBHENoiseIsoFilter
        if (i == 4  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flagecaltp    = 0; // EcalDeadCellTriggerPrimitiveFilter
        if (i == 5  && filterResultsH->accept(filterPathsMap[filterPathsVector[i]])) flageebadsc   = 0; // eeBadScFilter
    }

    flags = flagvtx + flaghalo + flaghbhe + flaghbheiso + flagecaltp + flageebadsc + flagbadmuon + flagbadhad;

    // Pileup information
    putrue = 0;
    if (isMC && pileupInfoH.isValid()) {
        for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) {
            if (pileupInfo_iter->getBunchCrossing() == 0) putrue = (unsigned char)pileupInfo_iter->getTrueNumInteractions();
        }
    }

    nvtx  = (unsigned char)verticesH->size();
    if (nvtx == 0) return;


    // Muon information
    vector<pat::MuonRef> muonv;
    for (auto muons_iter = muonsH->begin(); muons_iter != muonsH->end(); ++muons_iter) {
        if (muons_iter->pt() < 4.0) continue;
        if (fabs(muons_iter->eta()) > 1.9) continue;
        pat::MuonRef mref(muonsH, muons_iter - muonsH->begin());
        muonv.push_back(mref);
    }
    if (not (muonv.size() >= 2)) {
        if (applyDimuonFilter) return;
    }
    else sort(muonv.begin(), muonv.end(), muonSorter);

  
    /*  
    // Vertex Calculation
    if (not isGoodMuon(*muonv[0].get())) return;
    if (not isGoodMuon(*muonv[1].get())) return;
    auto kalmanMuMuVertexFit = vertexMuonsWithKalmanFitter(*muonv[0].get(), *muonv[1].get());
    kalmanMuMuVertexFit.postprocess(*beamSpot_);
    //cout << "Lxy of leading dimuon pair = " << kalmanMuMuVertexFit.lxy << endl;
    */



    // Vertex Calculation (Take best p-value pair)
    float bestP = 1;
    float bestLxy = 0;
    float bestLxyErr = 0;
    float bestSigLxy = 0;
    bool bestValid = 0;
    vector<int> bestMu = {0,0};
    cout << "iter. start" << endl;
    for (unsigned int i = 0; i<(sizeof(muonv)-1); i++){
      cout << i << endl;
      if (not isGoodMuon(*muonv[i].get())) continue;
      for (unsigned int j = i+1; j<sizeof(muonv); j++){
	cout << j <<endl;
	if (not isGoodMuon(*muonv[j].get())) continue;
	auto kalmanMuMuVertexFit = vertexMuonsWithKalmanFitter(*muonv[i].get(), *muonv[j].get());
	kalmanMuMuVertexFit.postprocess(*beamSpot_);
        cout << "new pair, mu " << i << "mu " << j << endl; 
	//cout << "Lxy of leading dimuon pair = " << kalmanMuMuVertexFit.lxy << endl;
	if (kalmanMuMuVertexFit.vtxProb < bestP){
	  bestP = kalmanMuMuVertexFit.vtxProb;
	  cout << "new best P value: " << bestP << endl;
	  bestSigLxy = kalmanMuMuVertexFit.sigLxy;
	  bestLxyErr = kalmanMuMuVertexFit.lxyErr;
	  bestLxy = kalmanMuMuVertexFit.lxy;
	  bestValid = kalmanMuMuVertexFit.valid;
	  bestMu[0] = i;
	  bestMu[1] = j;
	}
      }
    }
    if (bestMu[0] == 0 && bestMu[1] == 0) return;
    vtxProb.push_back(bestP);
    valid.push_back(bestValid);
    lxy.push_back(bestLxy);
    lxyErr.push_back(bestLxyErr);
    sigLxy.push_back(bestSigLxy);

    /*
    vtxProb.push_back(kalmanMuMuVertexFit.vtxProb);
    valid.push_back(kalmanMuMuVertexFit.valid);
    lxy.push_back(kalmanMuMuVertexFit.lxy);
    lxyErr.push_back(kalmanMuMuVertexFit.lxyErr);
    sigLxy.push_back(kalmanMuMuVertexFit.sigLxy);
    chiSq.push_back(kalmanMuMuVertexFit.chiSq);
    */
    m1Impact.push_back(muonv[bestMu[0]]->track().get()->dxy());
    m2Impact.push_back(muonv[bestMu[1]]->track().get()->dxy());
    cout << "passed muons" << endl;

    int nLoose=0;
    for (size_t i = 0; i < muonv.size(); i++) {
        TLorentzVector m4;
        m4.SetPtEtaPhiM(muonv[i]->pt(), muonv[i]->eta(), muonv[i]->phi(), 0.1057);
        muons.push_back(m4);

        // Muon isolation
        double muonisoval = 0.0;
        muonisoval  = max(0., muonv[i]->pfIsolationR04().sumNeutralHadronEt + muonv[i]->pfIsolationR04().sumPhotonEt - 0.5*muonv[i]->pfIsolationR04().sumPUPt);
        muonisoval += muonv[i]->pfIsolationR04().sumChargedHadronPt;
        muonisoval /= muonv[i]->pt();
        miso.push_back(muonisoval);

        // Muon ID
        if (muonv[i]->isLooseMuon()) nLoose+=1;

        midloose.push_back(muonv[i]->isLooseMuon());
        midsoft.push_back(muonv[i]->isSoftMuon (verticesH->at(0)));
        midmedium.push_back(muonv[i]->isMediumMuon());
        midtight.push_back(muonv[i]->isTightMuon (verticesH->at(0)));

        /*
        if (muonv[i]->isLooseMuon ()){nLoose+=1;                   midval += 1;}
        if (muonv[i]->isSoftMuon (verticesH->at(0)))               midval += 2;
        if (muonv[i]->isMediumMuon() && !useMediumID2016)          midval += 4;
        if (isMediumMuon(muonv[i])   &&  useMediumID2016)          midval += 4;
        if (muonv[i]->isTightMuon (verticesH->at(0)))              midval += 8;

        // Muon HLT matching
        bool hlt1mmatch = false;
        bool hlt2mmatch = false;
        bool dstmatch   = false;
        const edm::TriggerNames& trigNames = iEvent.triggerNames(*triggerResultsH);
        for (pat::TriggerObjectStandAlone trgobj : *triggerObjectsH) {
            trgobj.unpackPathNames(trigNames);
            if (not (deltaR(trgobj.eta(), trgobj.phi(), muonv[i]->eta(), muonv[i]->phi()) < 0.1)) continue;
            if (trgobj.hasPathName("HLT_IsoMu24_v*"                                 , true, false) or trgobj.hasPathName("HLT_IsoMu24_v*"                                 , true, true)) hlt1mmatch = true;
            if (trgobj.hasPathName("HLT_IsoTkMu24_v*"                               , true, false) or trgobj.hasPathName("HLT_IsoTkMu24_v*"                               , true, true)) hlt1mmatch = true;
            if (trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v*"            , true, false) or trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v*"            , true, true)) hlt2mmatch = true;
            if (trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v*"          , true, false) or trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v*"          , true, true)) hlt2mmatch = true;
            if (trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*"         , true, false) or trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v*"         , true, true)) hlt2mmatch = true;
            if (trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*"       , true, false) or trgobj.hasPathName("HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v*"       , true, true)) hlt2mmatch = true;
            if (trgobj.hasPathName("DST_DoubleMu3_Mass10_CaloScouting_PFScouting_v*", true, false) or trgobj.hasPathName("DST_DoubleMu3_Mass10_CaloScouting_PFScouting_v*", true, true)) dstmatch   = true;
        }
        if (hlt1mmatch) midval += 16;
        if (hlt2mmatch) midval += 32;
        if (  dstmatch) midval += 64;
        midval *= muonv[i]->pdgId() / abs(muonv[i]->pdgId());
        */

        mid.push_back(muonv[i]->pdgId());
    }

    if (nLoose < 2) {
        if (applyDimuonFilter) return;
    }

    for (size_t i = 0; i < muonv.size(); i++) {
        for (size_t j = i+1; j < muonv.size(); j++) {
            
            CompositeCandidate mm("mm");
            mm.addDaughter(ShallowCloneCandidate(CandidateBaseRef(muonv[i])), "muon1");
            mm.addDaughter(ShallowCloneCandidate(CandidateBaseRef(muonv[j])), "muon2");
            AddFourMomenta addp4;
            addp4.set(mm);
            
            double masserrval = -1.;    
            if (muonv[i]->track().isNonnull() && muonv[j]->track().isNonnull()) {       
                CompositeCandMassResolution merr;
                merr.init(iSetup);
                masserrval = merr.getMassResolution(mm);
            }
            masserr.push_back(masserrval);

            m1idx.push_back((unsigned char)i);
            m2idx.push_back((unsigned char)j);
        }
    }

    /*
    // Electron information
    vector<pat::ElectronRef> electronv;
    for (auto electrons_iter = electronsH->begin(); electrons_iter != electronsH->end(); ++electrons_iter) {
        pat::ElectronRef eref(electronsH, electrons_iter - electronsH->begin());
        if (eref->pt() < 10.0) continue;
        if (fabs(eref->eta()) > 2.5) continue;
        if (not ((*electronVetoIdH)[eref])) continue;
        if (fabs(eref->superCluster()->eta()) <= 1.479 && fabs(eref->gsfTrack()->dxy(verticesH->at(0).position())) > 0.05) continue;
        if (fabs(eref->superCluster()->eta()) >  1.479 && fabs(eref->gsfTrack()->dxy(verticesH->at(0).position())) > 0.10) continue;
        if (fabs(eref->superCluster()->eta()) <= 1.479 && fabs(eref->gsfTrack()->dz (verticesH->at(0).position())) > 0.10) continue;
        if (fabs(eref->superCluster()->eta()) >  1.479 && fabs(eref->gsfTrack()->dz (verticesH->at(0).position())) > 0.20) continue;
        electronv.push_back(eref);
    }
    if (electronv.size() > 0) sort(electronv.begin(), electronv.end(), electronSorter);

    for (size_t i = 0; i < electronv.size(); i++) {
        TLorentzVector e4;
        e4.SetPtEtaPhiM(electronv[i]->pt(), electronv[i]->eta(), electronv[i]->phi(), 5.11e-4);
        electrons.push_back(e4);

        char eidval = 1;

        if ((*electronLooseIdH )[electronv[i]]) eidval += 2;
        if ((*electronMediumIdH)[electronv[i]]) eidval += 4;
        if ((*electronTightIdH )[electronv[i]]) eidval += 8;

        // Electron HLT matching
        bool hlt1ematch = false;
        bool hlt2ematch = false;
        const edm::TriggerNames& trigNames = iEvent.triggerNames(*triggerResultsH);
        for (pat::TriggerObjectStandAlone trgobj : *triggerObjectsH) {
            trgobj.unpackPathNames(trigNames);
            if(not (deltaR(trgobj.eta(), trgobj.phi(), electronv[i]->eta(), electronv[i]->phi()) < 0.3)) continue;
            if (trgobj.hasPathName("HLT_Ele27_WPTight_Gsf_v*"                     , true, false) or trgobj.hasPathName("HLT_Ele27_WPTight_Gsf_v*"                     , true, true)) hlt1ematch = true;
            if (trgobj.hasPathName("HLT_Ele105_CaloIdVT_GsfTrkIdT_v*"             , true, false) or trgobj.hasPathName("HLT_Ele105_CaloIdVT_GsfTrkIdT_v*"             , true, true)) hlt1ematch = true;
            if (trgobj.hasPathName("HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*" , true, false) or trgobj.hasPathName("HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*" , true, true)) hlt2ematch = true;
            if (trgobj.hasPathName("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*" , true, false) or trgobj.hasPathName("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v*" , true, true)) hlt2ematch = true;
        }
        if (hlt1ematch) eidval += 16;
        if (hlt2ematch) eidval += 32;

        eidval *= electronv[i]->pdgId() / abs(electronv[i]->pdgId());
        eid.push_back(eidval);
    }
    */

    // Jets information
    vector<pat::JetRef> jetv;
    for (auto jets_iter = jetsH->begin(); jets_iter != jetsH->end(); ++jets_iter) {
        pat::JetRef jref(jetsH, jets_iter - jetsH->begin());
        if (jref->pt() < 20.0) continue;
        if (fabs(jref->eta()) > 5.0) continue;
        jetv.push_back(jref);
    }
    if (jetv.size() > 0) sort(jetv.begin(), jetv.end(), jetSorter);

    for (size_t i = 0; i < jetv.size(); i++) {
        TLorentzVector j4;
        j4.SetPtEtaPhiM(jetv[i]->pt(), jetv[i]->eta(), jetv[i]->phi(), jetv[i]->mass());
        jets.push_back(j4);
        double btv = -10.0;
        if (fabs(jetv[i]->eta()) < 2.4) btv = jetv[i]->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags");
        jbtag.push_back(btv);
        jid  .push_back(getJetID(jetv[i]));
    }

    // MET information
    met           = metH->front().uncorPt();
    metphi        = metH->front().uncorPhi();
    t1met         = metH->front().et();
    t1metphi      = metH->front().phi();
    t1metjecup    = metH->front().shiftedP2(pat::MET::METUncertainty::JetEnUp          ).pt();
    t1metjecupphi = metH->front().shiftedP2(pat::MET::METUncertainty::JetEnUp          ).phi();
    t1metjecdn    = metH->front().shiftedP2(pat::MET::METUncertainty::JetEnDown        ).pt();
    t1metjecdnphi = metH->front().shiftedP2(pat::MET::METUncertainty::JetEnDown        ).phi();
    t1metjerup    = metH->front().shiftedP2(pat::MET::METUncertainty::JetResUp         ).pt();
    t1metjerupphi = metH->front().shiftedP2(pat::MET::METUncertainty::JetResUp         ).phi();
    t1metjerdn    = metH->front().shiftedP2(pat::MET::METUncertainty::JetResDown       ).pt();
    t1metjerdnphi = metH->front().shiftedP2(pat::MET::METUncertainty::JetResDown       ).phi();
    t1metuncup    = metH->front().shiftedP2(pat::MET::METUncertainty::UnclusteredEnUp  ).pt();
    t1metuncupphi = metH->front().shiftedP2(pat::MET::METUncertainty::UnclusteredEnUp  ).phi();
    t1metuncdn    = metH->front().shiftedP2(pat::MET::METUncertainty::UnclusteredEnDown).pt();
    t1metuncdnphi = metH->front().shiftedP2(pat::MET::METUncertainty::UnclusteredEnDown).phi();


    // GEN information
    if (isMC && gensH.isValid()) {

        for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) { 
            if (abs(gens_iter->pdgId()) > 0.0 && abs(gens_iter->pdgId())==13 && gens_iter->fromHardProcessFinalState()) {
                TLorentzVector g4;
                g4.SetPtEtaPhiM(gens_iter->pt(), gens_iter->eta(), gens_iter->phi(), gens_iter->mass());
                gens.push_back(g4);
                gid.push_back(char(gens_iter->pdgId()));
		double y = gens_iter->vy();
		double x = gens_iter->vx();
		gvtx.push_back(sqrt(pow(x,2) + pow(y,2)));
            }

        }
    }

    //if (filterHToMuMu && !foundHiggsToMuMu) return;
    tree->Fill();
}


void TreeMaker::beginJob() {
    // Access the TFileService
    edm::Service<TFileService> fs;

    // Create the TTree
    tree = fs->make<TTree>("tree"       , "tree");

    // Event weights
    tree->Branch("xsec"                 , &xsec                          , "xsec/D");
    tree->Branch("wgt"                  , &wgt                           , "wgt/D");

    // Event coordinates
    if (addEventInfo) {
    tree->Branch("event"                , &event                         , "event/i");
    tree->Branch("run"                  , &run                           , "run/i");
    tree->Branch("lumSec"                 , &lumSec                          , "lumSec/i");
    }

    // Triggers
    tree->Branch("hltsinglemu"          , &hltsinglemu                   , "hltsinglemu/b");
    tree->Branch("hltdoublemu"          , &hltdoublemu                   , "hltdoublemu/b");
    tree->Branch("hltsingleel"          , &hltsingleel                   , "hltsingleel/b");
    tree->Branch("hltdoubleel"          , &hltdoubleel                   , "hltdoubleel/b");
    tree->Branch("l1Result", "std::vector<bool>" ,&l1Result_ , 32000, 0); 
    tree->Branch("trig"                , &trig                         , "trig/i");

    tree->Branch("triggersPassed"              , "std::vector<string>"          , &triggersPassed);

    // Flags
    tree->Branch("flags"                , &flags                         , "flags/b");

    // Pileup info
    tree->Branch("putrue"               , &putrue                        , "putrue/b");
    tree->Branch("nvtx"                 , &nvtx                          , "nvtx/b");

    // Muon info
    tree->Branch("muons"                , "std::vector<TLorentzVector>"  , &muons    , 32000, 0);
    tree->Branch("mid"                  , "std::vector<int>"            , &mid      );
    tree->Branch("midloose"                  , "std::vector<bool>"            , &midloose      );
    tree->Branch("midsoft"                  , "std::vector<bool>"            , &midsoft      );
    tree->Branch("midmedium"                  , "std::vector<bool>"            , &midmedium      );
    tree->Branch("midtight"                  , "std::vector<bool>"            , &midtight      );
    tree->Branch("miso"                 , "std::vector<double>"          , &miso     );
    tree->Branch("m1Impact"                 , "std::vector<double>"          , &m1Impact     );
    tree->Branch("m2Impact"                 , "std::vector<double>"          , &m2Impact     );



    // Dimuon info
    tree->Branch("m1idx"                , "std::vector<unsigned char>"   , &m1idx    );
    tree->Branch("m2idx"                , "std::vector<unsigned char>"   , &m2idx    );
    tree->Branch("masserr"              , "std::vector<double>"          , &masserr  );


    // Vertex info
    tree->Branch("vtxProb"                     , "std::vector<double>"          , &vtxProb    );
    tree->Branch("valid"                     , "std::vector<bool>"          , &valid          );
    tree->Branch("lxy"                     , "std::vector<double>"          , &lxy            );
    tree->Branch("lxyErr"                     , "std::vector<double>"          , &lxyErr      );
    tree->Branch("sigLxy"                     , "std::vector<double>"          , &sigLxy      );
    tree->Branch("chiSq"                     , "std::vector<double>"          , &chiSq      );

    // Electron info
    //tree->Branch("electrons"            , "std::vector<TLorentzVector>"  , &electrons, 32000, 0);
    //tree->Branch("eid"                  , "std::vector<char>"            , &eid      );

    // MET info
    /*
    tree->Branch("met"                  , &met                           , "met/D"          );
    tree->Branch("metphi"               , &metphi                        , "metphi/D"       );
    tree->Branch("t1met"                , &t1met                         , "t1met/D"        );
    tree->Branch("t1metphi"             , &t1metphi                      , "t1metphi/D"     );
    tree->Branch("t1metjecup"           , &t1metjecup                    , "t1metjecup/D"   );
    tree->Branch("t1metjecupphi"        , &t1metjecupphi                 , "t1metjecupphi/D");
    tree->Branch("t1metjecdn"           , &t1metjecdn                    , "t1metjecdn/D"   );
    tree->Branch("t1metjecdnphi"        , &t1metjecdnphi                 , "t1metjecdnphi/D");
    tree->Branch("t1metjerup"           , &t1metjerup                    , "t1metjerup/D"   );
    tree->Branch("t1metjerupphi"        , &t1metjerupphi                 , "t1metjerupphi/D");
    tree->Branch("t1metjerdn"           , &t1metjerdn                    , "t1metjerdn/D"   );
    tree->Branch("t1metjerdnphi"        , &t1metjerdnphi                 , "t1metjerdnphi/D");
    tree->Branch("t1metuncup"           , &t1metuncup                    , "t1metuncup/D"   );
    tree->Branch("t1metuncupphi"        , &t1metuncupphi                 , "t1metuncupphi/D");
    tree->Branch("t1metuncdn"           , &t1metuncdn                    , "t1metuncdn/D"   );
    tree->Branch("t1metuncdnphi"        , &t1metuncdnphi                 , "t1metuncdnphi/D");
    */

    // Jet info
    tree->Branch("jets"                 , "std::vector<TLorentzVector>"  , &jets     , 32000, 0);
    tree->Branch("jbtag"                , "std::vector<double>"          , &jbtag);
    tree->Branch("jid"                  , "std::vector<char>"            , &jid  );

    // Gen info
    tree->Branch("gens"                 , "std::vector<TLorentzVector>"  , &gens     , 32000, 0);
    tree->Branch("gid"                  , "std::vector<char>"            , &gid      );
    tree->Branch("gvtx"                  , "std::vector<double>"            , &gvtx      );
}

void TreeMaker::endJob() {
}

void TreeMaker::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
    // HLT paths
    triggerPathsVector.push_back("DST_DoubleMu3_noVtx_CaloScouting_v*");
    /*
    triggerPathsVector.push_back("DST_L1DoubleMu_CaloScouting_PFScouting_v");
    triggerPathsVector.push_back("DST_ZeroBias_CaloScouting_PFScouting_v");
    triggerPathsVector.push_back("DST_L1HTT_CaloScouting_PFScouting_v*");
    triggerPathsVector.push_back("DST_CaloJet40_CaloScouting_PFScouting_v*");
    triggerPathsVector.push_back("DST_HT250_CaloScouting_v*");
    triggerPathsVector.push_back("DST_HT410_PFScouting_v*");
    triggerPathsVector.push_back("DST_HT450_PFScouting_v*");
    */

    /*

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

    // MET filter Paths
    filterPathsVector.push_back("Flag_goodVertices");
    filterPathsVector.push_back("Flag_globalTightHalo2016Filter");
    filterPathsVector.push_back("Flag_HBHENoiseFilter");
    filterPathsVector.push_back("Flag_HBHENoiseIsoFilter");
    filterPathsVector.push_back("Flag_EcalDeadCellTriggerPrimitiveFilter");
    filterPathsVector.push_back("Flag_eeBadScFilter");

    HLTConfigProvider fltrConfig;
    fltrConfig.init(iRun, iSetup, filterResultsTag.process(), changedConfig);

    for (size_t i = 0; i < filterPathsVector.size(); i++) {
        filterPathsMap[filterPathsVector[i]] = -1;
    }

    for(size_t i = 0; i < filterPathsVector.size(); i++){
        TPRegexp pattern(filterPathsVector[i]);
        for(size_t j = 0; j < fltrConfig.triggerNames().size(); j++){
            std::string pathName = fltrConfig.triggerNames()[j];
            if(TString(pathName).Contains(pattern)){
                filterPathsMap[filterPathsVector[i]] = j;
            }
        }
    }

    */
}

void TreeMaker::endRun(edm::Run const&, edm::EventSetup const&) {
}

void TreeMaker::beginLuminosityBlock(edm::LuminosityBlock const& iLumi, edm::EventSetup const&) {
}

void TreeMaker::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

char TreeMaker::getJetID(const pat::JetRef& jet) {

    char jetid  = 0;

    double eta  = jet->eta();
    double nhf  = jet->neutralHadronEnergyFraction();
    double nemf = jet->neutralEmEnergyFraction();
    double chf  = jet->chargedHadronEnergyFraction();
    double muf  = jet->muonEnergyFraction();
    double cemf = jet->muonEnergyFraction();
    int    np   = jet->chargedMultiplicity() + jet->neutralMultiplicity();
    int    nnp  = jet->neutralMultiplicity();
    int    chm  = jet->chargedMultiplicity();

    if ((nhf < 0.99 && nemf < 0.99 && np > 1             ) && ((fabs(eta) <= 2.4 && chf > 0 && chm > 0 && cemf < 0.99) || fabs(eta) > 2.4) && fabs(eta) <= 2.7) jetid += 1;
    if ((nhf < 0.90 && nemf < 0.90 && np > 1             ) && ((fabs(eta) <= 2.4 && chf > 0 && chm > 0 && cemf < 0.99) || fabs(eta) > 2.4) && fabs(eta) <= 2.7) jetid += 2;
    if ((nhf < 0.90 && nemf < 0.90 && np > 1 && muf < 0.8) && ((fabs(eta) <= 2.4 && chf > 0 && chm > 0 && cemf < 0.90) || fabs(eta) > 2.4) && fabs(eta) <= 2.7) jetid += 4;

    if (nemf < 0.90 && nnp > 2 && fabs(eta) > 2.7 && fabs(eta) <= 3.0) jetid += 1;
    if (nemf < 0.90 && nnp > 2 && fabs(eta) > 2.7 && fabs(eta) <= 3.0) jetid += 2;
    if (nemf < 0.90 && nnp > 2 && fabs(eta) > 2.7 && fabs(eta) <= 3.0) jetid += 4;

    if (nemf < 0.90 && nnp > 10 && fabs(eta) > 3.0) jetid += 1;
    if (nemf < 0.90 && nnp > 10 && fabs(eta) > 3.0) jetid += 2;
    if (nemf < 0.90 && nnp > 10 && fabs(eta) > 3.0) jetid += 4;

    double puidval   = jet->userFloat("pileupJetId:fullDiscriminant");
    if (fabs(eta) >= 0.00 && fabs(eta) < 2.50 && puidval > -0.63) jetid += 8;
    if (fabs(eta) >= 2.50 && fabs(eta) < 2.75 && puidval > -0.60) jetid += 8;
    if (fabs(eta) >= 2.75 && fabs(eta) < 3.00 && puidval > -0.55) jetid += 8;
    if (fabs(eta) >= 3.00 && fabs(eta) < 5.00 && puidval > -0.45) jetid += 8;

    return jetid;
}

bool TreeMaker::isMediumMuon(const pat::MuonRef& muon) {
      bool goodGlob = muon->isGlobalMuon() && 
                      muon->globalTrack()->normalizedChi2() < 3 && 
                      muon->combinedQuality().chi2LocalPosition < 12 && 
                      muon->combinedQuality().trkKink < 20; 
      bool isMedium = muon::isLooseMuon(*muon) && 
                      muon->innerTrack()->validFraction() > 0.49 && 
                      muon::segmentCompatibility(*muon) > (goodGlob ? 0.303 : 0.451); 
      return isMedium; 
}

void TreeMaker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	edm::ParameterSetDescription desc;
	desc.setUnknown();
	descriptions.addDefault(desc);
}

KalmanVertexFitResult TreeMaker::vertexWithKalmanFitter(std::vector<const reco::Track*> trks, 
					 std::vector<float> masses){
  if (trks.size()!=masses.size()) 
    throw cms::Exception("Error") << "number of tracks and number of masses should match";
  KalmanVertexFitResult results;
  std::vector<reco::TransientTrack> transTrks;
  for (auto trk: trks){
    transTrks.push_back((*theTTBuilder_).build(trk));
  }
  KalmanVertexFitter kvf(true);
  TransientVertex tv = kvf.vertex(transTrks);

  if ( tv.isValid() ){
    results.chiSq = tv.totalChiSquared();
    results.vtxProb = TMath::Prob(tv.totalChiSquared(), (int)tv.degreesOfFreedom());
    results.valid = true;
    results.position = tv.position();
    results.err = tv.positionError();
    if (tv.hasRefittedTracks()){
      assert(tv.refittedTracks().size()==transTrks.size());
      for (unsigned int i=0; i<transTrks.size(); ++i){
	// Is it safe to assume that the order hasn't changed?
	GlobalVector gvP = tv.refittedTracks()[i].trajectoryStateClosestToPoint(tv.position()).momentum();
	// GlobalVector gvP = tv.originalTracks()[i].trajectoryStateClosestToPoint(tv.position()).momentum();
	results.refitVectors.push_back(makeLorentzVectorFromPxPyPzM(gvP.x(), gvP.y(), gvP.z(), masses[i]));
      }
    }
  }
  return results;
}

KalmanVertexFitResult TreeMaker::vertexMuonsWithKalmanFitter(const pat::Muon& muon1, const pat::Muon& muon2) {
  std::vector<const reco::Track*> trks;
  std::vector<float> masses;
  trks.push_back( muon1.innerTrack().get() );
  masses.push_back(MuonMass_);
  trks.push_back( muon2.innerTrack().get() );
  masses.push_back(MuonMass_);
  return vertexWithKalmanFitter(trks,masses);
}

bool TreeMaker::isGoodMuon(const pat::Muon& muon){
  if ( not muon.isLooseMuon() ) return false;
  if ( not muon.isTrackerMuon() ) return false;
  return true;
}



DEFINE_FWK_MODULE(TreeMaker);
