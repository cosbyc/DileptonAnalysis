# 2017 older version without HLT info
#python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/Apr17/DY5to50.root -o testReco_DY5to50_Apr17 -n 1 -j 1 >& logdy5.txt &
#python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/Apr17/DoubleEG.root -o testReco_DoubleEG_Apr17 -n 1 -j 1 >& logdoubleeg.txt &
#python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/Apr17/SingleElectron.root -o testReco_SingleElectron_Apr17 -n 1 -j 1 >& logsingleg.txt &

# 2017 newer version with HLT info
python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/May23/DY5to50.root -o testReco_DY5to50_May23 -n 1 -j 1 >& logdy5.txt &
python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/May23/DoubleEG.root -o testReco_DoubleEG_May23 -n 1 -j 1 >& logdoubleeg.txt &
python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/May23/SingleElectron.root -o testReco_SingleElectron_May23 -n 1 -j 1 >& logsingleeg.txt & 

# 2018
python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/May23_2018/DY10to50.root -o testReco_DY10to50_May23_2018 -n 1 -j 1 >& logdy10_2018.txt &
python -u fillRecoTree.py -i /eos/cms/store/user/dsperka/Zdark/May23_2018/EGamma.root -o testReco_EGamma_May23_2018 -n 1 -j 1 >& logEGamma_2018.txt &
