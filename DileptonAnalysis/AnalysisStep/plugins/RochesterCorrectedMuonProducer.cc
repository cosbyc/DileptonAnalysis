#include <memory>

#include "CLHEP/Random/RandFlat.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "DataFormats/Candidate/interface/ShallowCloneCandidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "DileptonAnalysis/AnalysisStep/src/RoccoR.cc"

class RochesterCorrectedMuonProducer : public edm::stream::EDProducer<> {
    public:
        explicit RochesterCorrectedMuonProducer(const edm::ParameterSet&);
        ~RochesterCorrectedMuonProducer();
        
        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
    
    private:
        virtual void beginStream(edm::StreamID) override;
        virtual void produce(edm::Event&, const edm::EventSetup&) override;
        virtual void endStream() override;

        RoccoR rc;
        bool isMC;
        bool correct;
		const edm::EDGetTokenT<edm::View<reco::Candidate> >     muonsToken;
        const edm::EDGetTokenT<std::vector<reco::GenParticle> > gensToken;
};

RochesterCorrectedMuonProducer::RochesterCorrectedMuonProducer(const edm::ParameterSet& iConfig):
    rc     (iConfig.existsAs<std::string>("data") ? iConfig.getParameter<std::string>("data") : "../data/Rochester"),
    isMC   (iConfig.existsAs<bool>("isMC")        ? iConfig.getParameter<bool>("isMC")        : false),
    correct(iConfig.existsAs<bool>("correct")     ? iConfig.getParameter<bool>("correct")     : false),
	muonsToken(consumes<edm::View<reco::Candidate> >    (iConfig.getParameter<edm::InputTag>("src"))),
	gensToken (consumes<std::vector<reco::GenParticle> >(iConfig.getParameter<edm::InputTag>("gens")))
{
    produces<std::vector<pat::Muon> >();
}

RochesterCorrectedMuonProducer::~RochesterCorrectedMuonProducer() {
}

void RochesterCorrectedMuonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;
    using namespace reco;
    using namespace std;

    Handle<View<Candidate> > muonsH;
    iEvent.getByToken(muonsToken, muonsH);
    
    Handle<vector<GenParticle> > gensH;
    if (isMC) iEvent.getByToken(gensToken, gensH);
    

    std::vector<pat::Muon> * out = new std::vector<pat::Muon>();
    out->reserve(muonsH->size());

    edm::Service<edm::RandomNumberGenerator> rng;
    CLHEP::HepRandomEngine& engine = rng->getEngine(iEvent.streamID());
    
    for (View<Candidate>::const_iterator muons_iter = muonsH->begin(); muons_iter != muonsH->end(); ++muons_iter) {
        double sf = 1.0;
        pat::Muon clone = *RefToBase<Candidate>(muonsH, muons_iter - muonsH->begin()).castTo<pat::MuonRef>();

        double genpt = -1.0;
        double minDR =  1.0;
        if (isMC && gensH.isValid()) {
            for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) { 
                if (gens_iter->pdgId()*clone.pdgId() != 169) continue;
                if (gens_iter->fromHardProcessFinalState()) continue;
                double dR = deltaR(gens_iter->eta(), gens_iter->phi(), clone.eta(), clone.phi());
                if (dR > 0.1 || dR > minDR || clone.pt()/gens_iter->pt() < 0.5 || clone.pt()/gens_iter->pt() > 2.0) continue;
                minDR = dR;
                genpt = gens_iter->pt();
            }
        }

        int nTkLayers = 0;
        if (clone.track().isNonnull()) nTkLayers = clone.track()->hitPattern().trackerLayersWithMeasurement();
        if (correct) {
            if (not isMC)             sf = rc.kScaleDT        (clone.charge(), clone.pt(), clone.eta(), clone.phi());
            if (isMC && genpt >  0.0) sf = rc.kScaleFromGenMC (clone.charge(), clone.pt(), clone.eta(), clone.phi(), nTkLayers, genpt        , engine.flat());
            if (isMC && genpt <= 0.0) sf = rc.kScaleAndSmearMC(clone.charge(), clone.pt(), clone.eta(), clone.phi(), nTkLayers, engine.flat(), engine.flat());
            clone.setP4(Particle::PolarLorentzVector(sf*clone.pt(), clone.eta(), clone.phi(), clone.mass()));   
        }
        out->push_back(clone);
    }

    std::unique_ptr<std::vector<pat::Muon> > ptr(out);
    iEvent.put(std::move(ptr));                
}

void RochesterCorrectedMuonProducer::beginStream(edm::StreamID) {
}

void RochesterCorrectedMuonProducer::endStream() {
}

void RochesterCorrectedMuonProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(RochesterCorrectedMuonProducer);
