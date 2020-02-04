import os

from DileptonAnalysis.AnalysisStep.samples.Samples import samples

keys  = samples.viewkeys()    
keylist = list(keys)

for x in range(len(samples)):
    if (os.path.exists(os.getcwd()+"/"+keylist[x]) and os.path.isdir(os.getcwd()+"/"+keylist[x])):
        print keylist[x] + " exists, skipping"
    else :
        print keylist[x] + " does not exist, making it"
        os.mkdir(os.getcwd()+"/"+keylist[x])
