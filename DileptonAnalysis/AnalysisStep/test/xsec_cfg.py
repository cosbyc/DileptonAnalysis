import os
import FWCore.ParameterSet.Config as cms

# Set parameters externally 
from FWCore.ParameterSet.VarParsing import VarParsing
params = VarParsing('analysis')

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


import FWCore.ParameterSet.Config as cms

# Define the process
process = cms.Process("COPY")

# Parse command line arguments
params.parseArguments()

# How many events to process
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(params.maxEvents) )

# Input EDM files
process.source = cms.Source("PoolSource",
    fileNames  = cms.untracked.vstring([]),
)
if params.Dataset != '':
    dbsinstance = " instance=" + params.DBSInstance if params.DBSInstance != '' else ""
    query = "das_client -query=\"file dataset=" + params.Dataset + dbsinstance + "\""
    fnames = os.popen(query).readlines()
    for fname in fnames:
        process.source.fileNames += [fname.rstrip()]

# Report every 1000 events
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

# This is the analyzer that computes the cross section 
process.xsec = cms.EDAnalyzer("GenXSecAnalyzer")

process.p = cms.Path(process.xsec)
