// Code from the H->ZZ->4L California analysis framework 

#ifndef COMPOSITECANDMASSRESOLUTION_H
#define COMPOSITECANDMASSRESOLUTION_H

#include <vector>
#include <TMatrixDSym.h>
#include "FWCore/Framework/interface/ESHandle.h"
#include "MagneticField/Engine/interface/MagneticField.h"

namespace reco { 
    class Candidate; 
    class Muon; 
    class Track; 
    class PackedCandidate; 
}

namespace edm { 
    class EventSetup; 
}

namespace pat { 
    class PackedCandidate; 
}

class CompositeCandMassResolution  {
    public:
        CompositeCandMassResolution() {}
        ~CompositeCandMassResolution() {}
        void init(const edm::EventSetup &iSetup);
        double getMassResolution(const reco::Candidate &c) const ;
        double getMassResolutionWithComponents(const reco::Candidate &c, std::vector<double> &errI) const;
        double getPScaleError(const reco::Candidate &c) const ;
        double getPScaleError(const reco::Muon &c) const ;

    private:
        void   fillP3Covariance(const reco::Candidate &c     ,                       TMatrixDSym &bigCov, int offset) const ;
        void   fillP3Covariance(const reco::Muon &ci         ,                       TMatrixDSym &bigCov, int offset) const ;
        void   fillP3Covariance(const pat::PackedCandidate &c,                       TMatrixDSym &bigCov, int offset) const ;
        void   fillP3Covariance(const reco::Candidate &c     , const reco::Track &t, TMatrixDSym &bigCov, int offset) const ;
        
        edm::ESHandle<MagneticField> magfield_;
        
        void getLeaves(const reco::Candidate &c, std::vector<const reco::Candidate *> &out) const ;
        
        double getMassResolution_(const reco::Candidate &c, std::vector<double> &errI, bool doComponents) const;
};

#endif

