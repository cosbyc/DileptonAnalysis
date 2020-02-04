#include <cmath>
#include <TMatrixD.h>

#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "TrackingTools/AnalyticalJacobians/interface/JacobianCurvilinearToCartesian.h"
#include "TrackingTools/TrajectoryParametrization/interface/GlobalTrajectoryParameters.h"
#include "TrackingTools/TrajectoryParametrization/interface/CartesianTrajectoryError.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "RecoParticleFlow/PFClusterTools/interface/PFEnergyResolution.h"

#include "DileptonAnalysis/AnalysisStep/interface/CompositeCandMassResolution.h"

void CompositeCandMassResolution::init(const edm::EventSetup &iSetup) {
    iSetup.get<IdealMagneticFieldRecord>().get(magfield_);
}

void CompositeCandMassResolution::getLeaves(const reco::Candidate &c, std::vector<const reco::Candidate *> &out) const {
    if (c.hasMasterClonePtr())   getLeaves(*c.masterClonePtr(), out);
    else if (c.hasMasterClone()) getLeaves(*c.masterClone()   , out);
    else if (c.numberOfDaughters() > 0 && (c.pdgId() != 22 || dynamic_cast<const pat::PackedCandidate *>(&c) == 0)) {
        for (int i = 0, n = c.numberOfDaughters(); i < n; ++i) getLeaves(*c.daughter(i), out);
    } 
    else out.push_back(&c);
}


double CompositeCandMassResolution::getMassResolution(const reco::Candidate &c) const  {
    std::vector<double> dummy;
    return getMassResolution_(c, dummy, false);
}
double CompositeCandMassResolution::getMassResolutionWithComponents(const reco::Candidate &c, std::vector<double> &errs) const {
    return getMassResolution_(c, errs, true);
}

double CompositeCandMassResolution::getMassResolution_(const reco::Candidate &c, std::vector<double> &errs, bool doComponents) const {
    std::vector<const reco::Candidate *> leaves;
    getLeaves(c, leaves);
    int n = leaves.size(), ndim = n*3;
    TMatrixDSym bigCov(ndim);
    TMatrixD jacobian(1,ndim);
    for (int i = 0, o = 0; i < n; ++i, o += 3) {
        const reco::Candidate &ci = *leaves[i];
        fillP3Covariance(ci, bigCov, o);
        jacobian(0, o+0) = (c.energy()*(ci.px()/ci.energy()) - c.px())/c.mass();
        jacobian(0, o+1) = (c.energy()*(ci.py()/ci.energy()) - c.py())/c.mass();
        jacobian(0, o+2) = (c.energy()*(ci.pz()/ci.energy()) - c.pz())/c.mass();
    }
    if (doComponents) {
        errs.resize(n);
        for (int i = 0, o = 0; i < n; ++i, o += 3) {
            TMatrixDSym bigCovOne(ndim);
            for (int ir = 0; ir < 3; ++ir) { 
                for (int ic = 0; ic < 3; ++ic) bigCovOne(o+ir,o+ic) = bigCov(o+ir,o+ic);
            }
            double dm2 = bigCovOne.Similarity(jacobian)(0,0);
            errs[i] = dm2 > 0 ? std::sqrt(dm2) : 0.0;
        }
    }
    
    TMatrixDSym massCov = bigCov.Similarity(jacobian);
    double dm2 = massCov(0,0);
    return (dm2 > 0 ? std::sqrt(dm2) : 0.0);
}

void CompositeCandMassResolution::fillP3Covariance(const reco::Candidate &c, TMatrixDSym &bigCov, int offset) const {
    const reco::Muon *mu; const pat::PackedCandidate *pf;
    if ((mu = dynamic_cast<const reco::Muon *>(&c)) != 0) fillP3Covariance(*mu, bigCov, offset);
    else if ((pf = dynamic_cast<const pat::PackedCandidate *>(&c)) != 0 && pf->pdgId() == 22) fillP3Covariance(*pf, bigCov, offset);
    else throw cms::Exception("Unknown type") << "Candidate of type " << typeid(c).name() << " and pdgId = " << c.pdgId() << "\n";
}

double CompositeCandMassResolution::getPScaleError(const reco::Candidate &c) const {
    if (c.hasMasterClonePtr()) return getPScaleError(*c.masterClonePtr());
    else if (c.hasMasterClone()) return getPScaleError(*c.masterClone());

    const reco::Muon *mu;
    if ((mu = dynamic_cast<const reco::Muon *>(&c)) != 0) return getPScaleError(*mu);
    else throw cms::Exception("Unknown type") << "Candidate of type " << typeid(c).name() << " and pdgId = " << c.pdgId() << " in getPScaleError\n";
}

double CompositeCandMassResolution::getPScaleError(const reco::Muon &c) const {
    return std::abs(c.muonBestTrack()->qoverpError()/c.muonBestTrack()->qoverp());
}

void CompositeCandMassResolution::fillP3Covariance(const reco::Muon &c, TMatrixDSym &bigCov, int offset) const {
    fillP3Covariance(c, *c.track(), bigCov, offset);
}


void CompositeCandMassResolution::fillP3Covariance(const reco::Candidate &c, const reco::Track &t, TMatrixDSym &bigCov, int offset) const {
    // In order to produce a 3x3 matrix, we need a jacobian from (p) to (px,py,pz), i.e.
    //            [ Px/P  ]                
    //  C_(3x3) = [ Py/P  ] * sigma^2(P) * [ Px/P Py/P Pz/P  ]
    //            [ Pz/P  ]                

    GlobalTrajectoryParameters gp(GlobalPoint(t.vx(), t.vy(), t.vz()), GlobalVector(t.px(),t.py(),t.pz()), t.charge(), magfield_.product());
    JacobianCurvilinearToCartesian curv2cart(gp);
    CartesianTrajectoryError cartErr= ROOT::Math::Similarity(curv2cart.jacobian(), t.covariance());
    const AlgebraicSymMatrix66 mat = cartErr.matrix();
    for (int i = 0; i < 3; ++i) { 
        for (int j = 0; j < 3; ++j) bigCov(offset+i,offset+j) = mat(i+3,j+3);
    } 
}


void CompositeCandMassResolution::fillP3Covariance(const pat::PackedCandidate &c, TMatrixDSym &bigCov, int offset) const {
    double dp = PFEnergyResolution().getEnergyResolutionEm(c.energy(), c.eta());
    AlgebraicMatrix31 ptop3;
    ptop3(0,0) = c.px()/c.p();
    ptop3(1,0) = c.py()/c.p();
    ptop3(2,0) = c.pz()/c.p();
    AlgebraicSymMatrix33 mat = ROOT::Math::Similarity(ptop3, AlgebraicSymMatrix11(dp*dp) );
    for (int i = 0; i < 3; ++i) { 
        for (int j = 0; j < 3; ++j) bigCov(offset+i,offset+j) = mat(i,j);
    } 
}
