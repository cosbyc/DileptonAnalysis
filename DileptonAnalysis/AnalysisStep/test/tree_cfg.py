import os
import FWCore.ParameterSet.Config as cms

# Set parameters externally 
from FWCore.ParameterSet.VarParsing import VarParsing
params = VarParsing('analysis')

params.register(
    'skipEvents',
    0,
    VarParsing.multiplicity.singleton,VarParsing.varType.int,
    'Path of local input files'
)

params.register(
    'reportFreq',
    100,
    VarParsing.multiplicity.singleton,VarParsing.varType.int,
    'Path of local input files'
)

params.register(
    'localDatasetPath',
    '',
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Path of local input files'
)

params.register(
    'Dataset',
    '',
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Get input files of a published dataset'
)

params.register(
    'DBSInstance',
    '',
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'DBS instance, e.g. prod/phys03 for USER samples'
)

params.register(
    'JSONFile',
    '',
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'JSON file'
)

params.register(
    'isMC', 
    True, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether the sample is simulation or data'
)

params.register(
    'useWeights', 
    True, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to use the events weights from a Monte Carlo generator'
)

params.register(
    'filterTrigger', 
    False, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to ask the event to fire a trigger used in the analysis'
)

params.register(
    'filterDimuons', 
    True, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to ask the event to have at least two muons with pT > 4 GeV'
)

params.register(
    'filterHToMuMu', 
    False, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to filter events containing Higgs decays to dimuons'
)

params.register(
    'useMediumID2016', 
    False, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to use the events weights from a Monte Carlo generator'
)

params.register(
    'addEventInfo', 
    True, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to save the event coordinates in the tree'
)

params.register(
    'correctMuonP', 
    True, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to apply the Rochester corrections to the muons'
)

params.register(
    'roccorData', 
    '../data/Rochester', 
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Path of the Rochester correction data files'
)

params.register(
    'redoJetsMET', 
    False, 
    VarParsing.multiplicity.singleton,VarParsing.varType.bool,
    'Flag to indicate whether or not to remake the jets and MET collections with specified JEC payloads'
)

params.register(
    'xsec', 
    0.001, 
    VarParsing.multiplicity.singleton,VarParsing.varType.float,
    'Cross-section for a Monte Carlo Sample'
)

params.register(
    'trigProcess', 
    'HLT', 
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Process name for the HLT paths'
)

params.register(
    'miniAODProcess', 
    'PAT', 
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Process name for the MET filter paths'
)

params.register(
    'GlobalTagMC',
    '102X_upgrade2018_realistic_v18',  
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Process name for the HLT paths'
)

params.register(
    'GlobalTagData', 
    '102X_dataRun2_v4', 
    VarParsing.multiplicity.singleton,VarParsing.varType.string,
    'Process name for the HLT paths'
)

# Define the process
process = cms.Process("LL")

# Parse command line arguments
params.parseArguments()

# Message Logger settings
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.FwkReport.reportEvery = 100

# Set the process options -- Display summary at the end, enable unscheduled execution
process.options = cms.untracked.PSet( 
    allowUnscheduled = cms.untracked.bool(True),
    wantSummary      = cms.untracked.bool(False) 
)

# How many events to process
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(params.maxEvents) )

# Input EDM files
process.source = cms.Source("PoolSource",
    fileNames  = cms.untracked.vstring([]),
    skipEvents = cms.untracked.uint32(params.skipEvents)
)

if params.localDatasetPath != '':
    for fname in os.listdir(params.localDatasetPath):
        path = params.localDatasetPath + "/" + fname
        if os.path.getsize(path) == 0 :
            print path + " is invalid ... skipping"
        else :
            path = "file:" + path
            process.source.fileNames += [path]

if params.Dataset != '':
    dbsinstance = " instance=" + params.DBSInstance if params.DBSInstance != '' else ""
    query = "das_client -query=\"file dataset=" + params.Dataset + dbsinstance + "\""
    fnames = os.popen(query).readlines()
    for fname in fnames:
        process.source.fileNames += [fname.rstrip()]

# Apply JSON if specified and if running on data 
import FWCore.PythonUtilities.LumiList as LumiList
if (not params.isMC) and (params.JSONFile != ''):
    process.source.lumisToProcess = LumiList.LumiList(filename = params.JSONFile).getVLuminosityBlockRange()




# Load the standard set of configuration modules
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

# Tracking Tools
process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")
process.load("Configuration.Geometry.GeometryRecoDB_cff")


# Load the global tag
from Configuration.AlCa.GlobalTag import GlobalTag
if params.isMC : 
    process.GlobalTag.globaltag = params.GlobalTagMC
else :
    process.GlobalTag.globaltag = params.GlobalTagData

# Define the services needed for the Rochester corrections and the treemaker
process.TFileService = cms.Service("TFileService", 
    fileName = cms.string("tree.root")
)
process.RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
    correctedMuons = cms.PSet(
        initialSeed = cms.untracked.uint32(1),
        engineName  = cms.untracked.string('TRandom3')
    )
)

# Tree for the generator weights
process.gentree = cms.EDAnalyzer("LHEWeightsTreeMaker",
    lheInfo = cms.InputTag("externalLHEProducer"),
    genInfo = cms.InputTag("generator"),
    useLHEWeights = cms.bool(params.useWeights),
    pileupinfo = cms.InputTag("addPileupInfo")
)

# Select good primary vertices
process.goodVertices = cms.EDFilter("VertexSelector",
    src    = cms.InputTag("offlineSlimmedPrimaryVertices"),
    cut    = cms.string("!isFake && ndof > 4 && abs(z) <= 24 && position.Rho <= 2"),
    filter = cms.bool(True)
)

# Correct muons with the Rochester corrections 
process.correctedMuons = cms.EDProducer("RochesterCorrectedMuonProducer",
    src     = cms.InputTag("slimmedMuons"),
    gens    = cms.InputTag("prunedGenParticles"),
    data    = cms.string(params.roccorData),
    isMC    = cms.bool(params.isMC),
    correct = cms.bool(params.correctMuonP) 
)

# MET filters
process.load("RecoMET.METFilters.BadPFMuonFilter_cfi")
process.BadPFMuonFilter.muons = cms.InputTag("slimmedMuons")
process.BadPFMuonFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadPFMuonFilter.taggingMode = cms.bool(True)

process.load("RecoMET.METFilters.BadChargedCandidateFilter_cfi")
process.BadChargedCandidateFilter.muons = cms.InputTag("slimmedMuons")
process.BadChargedCandidateFilter.PFCandidates = cms.InputTag("packedPFCandidates")
process.BadChargedCandidateFilter.taggingMode = cms.bool(True)

process.metfilters = cms.Sequence(process.goodVertices * process.BadPFMuonFilter * process.BadChargedCandidateFilter)



## Electron ValueMaps for identification
#from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
#dataFormat = DataFormat.MiniAOD
#switchOnVIDElectronIdProducer(process, dataFormat)

#ele_id_modules = ['RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff']
#for idmod in ele_id_modules:
#    setupAllVIDIdsInModule(process, idmod, setupVIDElectronSelection)

# Remake jets and recompute MET using specified JECs
if params.redoJetsMET :

    from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
    from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
    from PhysicsTools.PatUtils.tools.corMETFromMuonAndEG import corMETFromMuonAndEG

    if params.isMC :
        JECLevels = ['L1FastJet', 'L2Relative', 'L3Absolute']
    else :
        JECLevels = ['L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual']

    updateJetCollection(
        process,
        jetSource = cms.InputTag('slimmedJets'),
        labelName = '',
        jetCorrections = ('AK4PFchs', cms.vstring(JECLevels), 'None')
    )
    jetColl = "updatedPatJets"

    runMetCorAndUncFromMiniAOD(process,
        isData = (not params.isMC)
    )
    if (not params.isMC) :
        corMETFromMuonAndEG(
            process,
            pfCandCollection="",
            electronCollection="slimmedElectronsBeforeGSFix",
            photonCollection="slimmedPhotonsBeforeGSFix",
            corElectronCollection="slimmedElectrons", 
            corPhotonCollection="slimmedPhotons",
            allMETEGCorrected=True,
            muCorrection=False,
            eGCorrection=True,
            runOnMiniAOD=True,
            postfix="MuEGClean"
        )
        process.slimmedMETsMuEGClean = process.slimmedMETs.clone()
        process.slimmedMETsMuEGClean.src = cms.InputTag("patPFMetT1MuEGClean")
        process.slimmedMETsMuEGClean.rawVariation = cms.InputTag("patPFMetRawMuEGClean")
        process.slimmedMETsMuEGClean.t1Uncertainties = cms.InputTag("patPFMetT1%sMuEGClean")
        del process.slimmedMETsMuEGClean.caloMET
else :
    jetColl = "slimmedJets"

if (params.isMC) :
    metColl = "slimmedMETs"
else :
#    metColl = "slimmedMETsMuEGClean"
    metColl = "slimmedMETs"

from DileptonAnalysis.AnalysisStep.TriggerPaths_2017_cfi import getL1Conf
##--- l1 stage2 digis ---
process.load("EventFilter.L1TRawToDigi.gtStage2Digis_cfi")
process.gtStage2Digis.InputLabel = cms.InputTag( "hltFEDSelectorL1" )

# Make tree
process.mmtree = cms.EDAnalyzer('TreeMaker',
	applyHLTFilter    = cms.bool(params.filterTrigger),
	applyDimuonFilter = cms.bool(params.filterDimuons),
	isMC              = cms.bool(params.isMC),
	useLHEWeights     = cms.bool(params.useWeights),
    useMediumID2016   = cms.bool(params.useMediumID2016),
    addEventInfo      = cms.bool(params.addEventInfo),
    filterHToMuMu     = cms.bool(params.filterHToMuMu),
	xsec              = cms.double(params.xsec),
    triggerresults    = cms.InputTag("TriggerResults", "", params.trigProcess),
    filterresults     = cms.InputTag("TriggerResults", "", params.miniAODProcess),
    triggerobjects    = cms.InputTag("slimmedPatTrigger"),
        AlgInputTag = cms.InputTag("gtStage2Digis"),
        ExtInputTag = cms.InputTag("gtStage2Digis"),
        ReadPrescalesFromFile =  cms.bool(False),                        
        DumpRecord = cms.bool(True),                        
        DumpTrigResults = cms.bool(True),                        
        DumpTrigSummary = cms.bool(True),                        
        l1Seeds = cms.vstring(getL1Conf()),
	badmuon           = cms.InputTag("BadPFMuonFilter"),
	badhadron         = cms.InputTag("BadChargedCandidateFilter"),
	vertices          = cms.InputTag("offlineSlimmedPrimaryVertices"),
	muons             = cms.InputTag("slimmedMuons"),
	jets              = cms.InputTag(jetColl),
	met               = cms.InputTag(metColl),
#    electrons         = cms.InputTag("slimmedElectrons"),
#    electronidveto    = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto"),
#    electronidloose   = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-loose"),
#    electronidtight   = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-medium"),
#    electronidmedium  = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight"),
    pileupinfo        = cms.InputTag("slimmedAddPileupInfo"),
    geneventinfo      = cms.InputTag("generator"),
    genlumiheader     = cms.InputTag("generator"),
    gens              = cms.InputTag("prunedGenParticles"),
    beamSpot=cms.InputTag("offlineBeamSpot")
)

# Analysis path
if params.isMC : 
    process.p = cms.Path(process.gentree + process.metfilters + process.mmtree)
else : 
    process.p = cms.Path(                  process.metfilters + process.mmtree)

