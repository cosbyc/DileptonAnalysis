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
    skipEvents = cms.untracked.uint32(params.skipEvents)
)
if params.Dataset != '':
    dbsinstance = " instance=" + params.DBSInstance if params.DBSInstance != '' else ""
    query = "das_client -query=\"file dataset=" + params.Dataset + dbsinstance + "\""
    fnames = os.popen(query).readlines()
    for fname in fnames:
        process.source.fileNames += [fname.rstrip()]

# Output EDM file
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string("copy.root")
)

process.p = cms.EndPath(process.out)

