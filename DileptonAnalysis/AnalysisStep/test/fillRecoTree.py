from ROOT import *
from math import *
#!/usr/bin/python                                                                                                                                            
#-----------------------------------------------                                                                                                             
import sys, os, pwd, commands, glob, fnmatch
import optparse, shlex, re
import time
from time import gmtime, strftime
import math
from array import array

#define function for parsing options
def parseOptions():

    usage = ('usage: %prog [options]\n'
             + '%prog -h for help')
    parser = optparse.OptionParser(usage)

    # input options
    parser.add_option('-i', '--input', dest='INPUT', type='string', help='input file')
    parser.add_option('-o', '--output', dest='OUTPUT', type='string', help='output file')
    parser.add_option('-n', '--njobs', dest='NJOBS', type=int, help='njobs')
    parser.add_option('-j', '--job', dest='JOB', type=int, help='job')

    # store options and arguments as global variables
    global opt, args
    (opt, args) = parser.parse_args()

# define function for processing the external os commands
def processCmd(cmd, quite = 0):
    status, output = commands.getstatusoutput(cmd)
    if (status !=0 and not quite):
        print 'Error in processing command:\n   ['+cmd+']'
        print 'Output:\n   ['+output+'] \n'
    return output

def find_files(directory, pattern):
  for root, dirs, files in os.walk(directory):
    for basename in files:
      if fnmatch.fnmatch(basename, pattern):
        filename = os.path.join(root, basename)
        yield filename


L1_12_5_on = {"300087": [[38, 38], [41, 231]], "300105": [[1, 12]], "300106": [[1, 71]], "300107": [[1, 47]], "300117": [[37, 37], [41, 67]], "300122": [[46, 730], [735, 735], [738, 924], [928, 928], [931, 1285]], "300123": [[1, 384], [388, 388], [392, 596]], "300155": [[37, 1220]], "300156": [[1, 54]], "300157": [[9, 1107]], "300226": [[45, 45], [48, 89], [91, 448]], "300233": [[46, 46], [49, 152]], "300234": [[1, 50], [52, 52], [54, 54]], "300235": [[1, 171]], "300236": [[11, 178]], "300237": [[1, 713]], "300238": [[30, 58], [62, 63], [66, 314]], "300239": [[1, 145], [150, 150], [153, 167], [172, 172], [175, 206]], "300240": [[2, 7], [12, 12], [15, 356]], "300280": [[55, 56], [61, 61], [65, 69], [74, 74], [78, 150], [155, 155], [158, 165], [178, 178], [181, 198], [209, 222], [226, 227], [230, 251], [256, 256], [260, 268], [277, 345], [350, 350], [353, 370], [384, 548], [553, 553], [557, 607], [617, 639], [663, 690]], "300281": [[3, 7]], "300282": [[1, 9], [13, 59], [73, 92], [97, 114], [142, 151], [156, 178]], "300283": [[1, 25]], "300284": [[1, 22], [38, 47], [50, 82], [90, 98], [108, 130], [133, 152], [156, 250], [260, 414], [420, 561], [568, 585], [590, 680], [691, 751]], "300364": [[27, 27], [30, 42]], "300365": [[1, 13]], "300366": [[1, 14]], "300367": [[1, 12]], "300368": [[1, 12]], "300369": [[1, 12]], "300370": [[1, 11]], "300371": [[1, 12]], "300372": [[1, 7]], "300373": [[1, 12]], "300374": [[1, 11]], "300375": [[1, 93]], "300389": [[1, 1], [14, 14], [18, 20], [25, 25], [29, 39], [63, 142]], "300390": [[2, 12]], "300391": [[1, 12]], "300392": [[1, 12]], "300393": [[1, 12]], "300394": [[1, 11]], "300395": [[1, 12]], "300396": [[1, 11]], "300397": [[1, 12]], "300398": [[1, 13]], "300399": [[1, 16]], "300400": [[1, 5], [7, 671]], "300401": [[19, 673]], "300459": [[43, 43], [47, 110], [112, 331]], "300461": [[1, 87]], "300462": [[1, 90]], "300463": [[1, 123]], "300464": [[1, 256]], "300466": [[1, 642]], "300467": [[1, 254], [256, 563]], "300497": [[28, 29], [32, 49]], "300514": [[40, 41], [44, 61], [100, 136]], "300515": [[1, 15], [17, 1012]], "300516": [[1, 100]], "300517": [[1, 623]], "300558": [[8, 540]], "300560": [[1, 844]], "300574": [[17, 17], [20, 107]], "300575": [[1, 44]], "300576": [[7, 1206]], "300631": [[43, 43], [47, 49], [66, 66], [77, 218]], "300632": [[1, 16]], "300633": [[1, 447]], "300635": [[1, 175]], "300636": [[1, 1572]], "300674": [[1, 28]], "300675": [[1, 10]], "300676": [[1, 3], [5, 26]], "300742": [[56, 343]], "300777": [[24, 24], [28, 466]], "300780": [[3, 332]], "300785": [[1, 750], [755, 755], [758, 1272]], "300806": [[37, 38], [43, 68], [201, 211]], "300811": [[6, 143], [145, 344], [346, 504]], "300812": [[1, 41]], "300816": [[6, 140]], "300817": [[1, 493]], "301046": [[162, 223]], "301086": [[37, 37], [42, 82], [314, 331]], "301142": [[1, 116], [118, 897]], "301161": [[40, 40], [45, 208], [210, 248], [250, 791]], "301165": [[1, 145]], "301179": [[38, 38], [41, 52]], "301180": [[1, 75]], "301183": [[3, 10], [15, 15], [19, 303]], "301281": [[40, 40], [44, 147]], "301283": [[3, 345], [347, 886]], "301298": [[45, 250], [252, 539], [541, 949]], "301323": [[37, 37], [41, 664], [666, 990]], "301330": [[24, 24], [28, 353]], "301359": [[36, 36], [39, 319]], "301383": [[32, 32]], "301384": [[1, 476]], "301391": [[40, 40], [44, 211]], "301392": [[1, 608]], "301393": [[2, 29]], "301396": [[1, 25]], "301397": [[1, 719]], "301398": [[1, 6]], "301399": [[1, 108]], "301417": [[50, 367]], "301447": [[86, 96], [99, 415], [417, 510]], "301448": [[1, 324]], "301449": [[1, 385]], "301450": [[1, 173]], "301461": [[31, 31], [34, 581]], "301472": [[37, 38], [41, 835]], "301475": [[1, 11]], "301476": [[1, 844]], "301480": [[8, 166]], "301519": [[45, 45], [48, 95], [97, 238]], "301524": [[1, 265]], "301525": [[1, 56]], "301528": [[1, 14]], "301529": [[1, 33], [36, 48]], "301530": [[1, 105]], "301531": [[1, 101], [105, 392]], "301532": [[1, 611]], "301567": [[16, 16], [20, 372]], "301627": [[60, 943]], "301664": [[30, 435]], "301665": [[1, 294], [319, 487]], "301694": [[39, 102]], "301912": [[43, 52], [101, 415]], "301913": [[1, 54]], "301914": [[1, 350]], "301941": [[34, 568]], "301959": [[32, 1934]], "301960": [[1, 147]], "301970": [[6, 123]], "301984": [[20, 309]], "301985": [[1, 357]], "301986": [[1, 376]], "301987": [[1, 1128]], "301997": [[39, 393]], "301998": [[1, 1704]], "302019": [[37, 86]], "302026": [[26, 53]], "302029": [[1, 34], [37, 81]],"302031": [[1, 98], [101, 102], [105, 812]], "302033": [[1, 39]], "302034": [[1, 4]], "302036": [[10, 31]], "302037": [[3, 17]], "302038": [[1, 9]], "302040": [[1, 167]], "302041": [[1, 64]], "302042": [[1, 514]], "302043": [[1, 228]], "302131": [[71, 943]], "302159": [[36, 140]], "302163": [[34, 1220]], "302165": [[1, 79]], "302166": [[1, 16]], "302225": [[54, 133], [136, 923]], "302228": [[58, 78], [81, 291]], "302229": [[1, 457]], "302240": [[1, 255], [257, 960]], "302262": [[39, 463]], "302263": [[1, 1213], [1215, 1250]], "302277": [[18, 391]], "302279": [[1, 11], [13, 69]], "302280": [[1, 152]], "302322": [[36, 870]], "302328": [[45, 722]], "302337": [[30, 148]], "302342": [[19, 71]], "302343": [[1, 84]], "302344": [[3, 512]], "302350": [[1, 136]], "302388": [[29, 395], [399, 717]], "302392": [[47, 392]], "302393": [[1, 887]], "302448": [[24, 1926]], "302472": [[31, 801]], "302473": [[1, 396]], "302474": [[1, 305]], "302475": [[1, 1]], "302476": [[1, 259]], "302479": [[32, 340]], "302484": [[8, 170]], "302485": [[1, 921]], "302492": [[10, 21], [23, 53]], "302494": [[1, 618]], "302509": [[73, 92]], "302513": [[40, 89]], "302522": [[32, 34]], "302523": [[1, 48]], "302525": [[1, 677], [747, 767]], "302526": [[1, 582]], "302548": [[43, 115]], "302551": [[1, 1]], "302553": [[1, 182]], "302554": [[1, 1]], "302555": [[1, 3], [5, 11]], "302563": [[43, 44]], "302565": [[1, 1]], "302566": [[1, 62]], "302572": [[6, 283]], "302573": [[1, 569], [571, 1182], [1184, 1285]], "302596": [[50, 979]], "302597": [[1, 1054]], "302634": [[37, 37], [40, 73], [75, 123], [125, 131], [133, 165], [168, 175], [177, 216], [218, 358], [361, 375], [378, 404], [407, 423], [425, 503], [505, 578], [581, 594], [596, 633]], "302635": [[1, 22], [24, 28], [30, 39], [41, 53], [55, 132], [134, 144], [146, 265], [267, 271], [274, 344], [347, 357], [359, 375], [378, 384], [386, 414], [416, 494], [497, 608], [611, 634], [637, 684], [687, 706], [708, 724], [726, 901], [904, 954], [957, 982], [984, 1072], [1075, 1124], [1126, 1129], [1132, 1206], [1209, 1234], [1236, 1291]], "302651": [[1, 142], [144, 145]], "302654": [[1, 317]], "302661": [[1, 49], [60, 60], [64, 64]], "302663": [[1, 706]],"303824": [[54, 172], [401, 767]], "303825": [[1, 180]], "303832": [[63, 1913]], "303838": [[54, 54], [83, 621], [623, 2044]], "303885": [[60, 434], [436, 2052]], "303948": [[55, 130], [133, 157], [162, 176], [178, 282], [285, 318], [321, 388], [391, 456], [459, 462], [466, 558], [561, 561], [563, 564], [569, 1678]], "303998": [[58, 78], [131, 315]], "303999": [[1, 744]], "304000": [[1, 56]], "304062": [[54, 1625], [1627, 2014]], "304120": [[40, 248]], "304125": [[1, 1769]], "304144": [[411, 1501], [1522, 1542], [1544, 1551], [1553, 2596], [2598, 2656]], "304158": [[165, 315], [317, 895], [897, 1750], [1752, 2087]], "304169": [[79, 1730]], "304170": [[1, 620]], "304199": [[10, 12]], "304200": [[1, 321]], "304204": [[59, 60], [259, 607]], "304209": [[178, 189], [219, 477]], "304292": [[185, 363], [382, 525], [528, 1779], [1781, 1811]], "304333": [[448, 1653]], "304366": [[507, 736], [738, 1387], [1390, 1396], [1399, 1402], [1404, 1407], [1409, 1412], [1414, 1416], [1419, 1421], [1424, 1873]], "304447": [[289, 726], [728, 1643]], "304451": [[1, 53]], "304507": [[76, 234]], "304508": [[1, 1324]], "304562": [[416, 836], [838, 848]], "304616": [[461, 740], [747, 1002]], "304654": [[538, 697]], "304655": [[1, 286], [288, 288], [290, 475], [477, 1194]], "304663": [[127, 203], [205, 591], [593, 689]], "304671": [[544, 1184], [1187, 1187]], "304672": [[1, 60]], "304738": [[489, 1676]], "304739": [[3, 12]], "304740": [[1, 278]], "304778": [[4, 1300]], "304797": [[554, 1202], [1205, 1697], [1699, 2273], [2275, 2950]],"305045": [[2, 213], [215, 872]], "305046": [[1, 667], [671, 686]], "305064": [[97, 653], [655, 2045]], "305081": [[756, 1107]], "305112": [[720, 1521]], "305113": [[9, 66]], "305114": [[1, 526]], "305186": [[232, 422]], "305188": [[1, 1002]], "305204": [[1, 1229]], "305207": [[786, 1076]], "305208": [[1, 372]], "305237": [[595, 1147]], "305248": [[294, 951]], "305252": [[1, 548]], "305312": [[99, 226]], "305313": [[1, 178], [180, 734]], "305314": [[1, 404]], "305349": [[1, 16]], "305350": [[1, 14]],"305365": [[569, 668], [676, 829]], "305366": [[1, 721], [724, 756], [769, 1254]], "305377": [[591, 1292], [1294, 1383], [1386, 1525]], "305406": [[126, 394], [401, 520], [528, 535], [540, 1475]], "305517": [[37, 116], [119, 119], [158, 161]], "305518": [[1, 603], [605, 1134]], "305588": [[56, 85], [89, 114]], "305589": [[2, 690]], "305590": [[1, 500], [517, 1020]], "305636": [[673, 2390]], "305766": [[717, 902]], "305814": [[651, 1912]], "305821": [[720, 830]], "305840": [[1, 6], [8, 1136], [1138, 1139]], "305842": [[1, 862]], "305898": [[747, 780]], "306042": [[70, 363]], "306048": [[1, 133]], "306049": [[1, 357]], "306051": [[1, 415]], "306092": [[56, 974]], "306095": [[1, 300]], "306125": [[435, 756], [770, 3000]], "306126": [[1, 497]], "306135": [[705, 1095]], "306138": [[6, 1297]], "306139": [[1, 1112]], "306154": [[594, 1231]], "306155": [[1, 1439]], "306169": [[1, 734]], "306170": [[1, 14]], "306171": [[10, 503]], "306423": [[111, 322]], "306432": [[1, 339]], "306456": [[484, 783]], "306457": [[1, 27]], "306458": [[1, 17], [20, 35], [37, 41], [43, 47], [49, 53], [56, 60], [62, 66], [68, 72], [74, 77], [79, 83], [85, 89], [93, 102], [104, 108], [110, 114], [116, 120], [122, 126], [129, 139], [141, 145], [147, 151], [153, 166], [169, 173], [175, 179], [181, 185], [187, 191], [193, 197], [200, 210], [212, 216], [218, 222], [225, 229], [231, 235], [237, 241], [243, 247], [249, 249], [252, 256], [258, 263]], "306459": [[1, 512], [514, 2273]], "306460": [[1, 62]]}

def injson(run, LS):
  if (run in L1_12_5_on):  
    for i in range(len(L1_12_5_on[run])):
      if (LS>=L1_12_5_on[run][i][0] and LS<=L1_12_5_on[run][i][1]): return True
  return False

def fillTree():

  global opt, args
  parseOptions()

  print opt.INPUT
  fin = TFile(opt.INPUT,"READ")
  t = fin.Get("mmtree/tree")
  N=t.GetEntriesFast()
  print N,"total events"

  first = int(float(N)/float(opt.NJOBS)*float(opt.JOB-1))
  last = int(float(N)/float(opt.NJOBS)*float(opt.JOB))

  '''
  ## split based on number of files
  #files = glob.glob(opt.INPUT+"/scout*.root")
  files = []
  for f in find_files(opt.INPUT,"tree*.root"):
  #for f in find_files(opt.INPUT,"scout_1-1.root"):
    if ("failed" in f): continue
    files.append(f)

  N=len(files)
  print N,"total files"

  if (N<1): sys.exit()

  first = int(float(N)/float(opt.NJOBS)*float(opt.JOB-1))
  last = int(float(N)/float(opt.NJOBS)*float(opt.JOB))

  print first,last

  t = TChain("mmtree/tree")
  for i in range(len(files)):
    if (i<first or i>last): continue
    print files[i]
    t.Add(files[i])
  '''

  print first,last
  print t.GetEntries()

  fout = TFile( opt.OUTPUT+'_'+str(opt.JOB)+'.root', 'recreate' )
  tout = TTree( 'tree', 'tree' )

  mll = array( 'f', [ 0. ] )
  tout.Branch( 'mll', mll, 'mll/F' )
 
  ptll = array( 'f', [ 0. ] )
  tout.Branch( 'ptll', ptll, 'ptll/F' )

  drll = array( 'f', [ 0. ] )
  tout.Branch( 'drll', drll, 'drll/F' )

  pt1 = array( 'f', [ 0. ] )
  tout.Branch( 'pt1', pt1, 'pt1/F' )

  pt2 = array( 'f', [ 0. ] )
  tout.Branch( 'pt2', pt2, 'pt2/F' )

  eta1 = array( 'f', [ 0. ] )
  tout.Branch( 'eta1', eta1, 'eta1/F' )

  eta2 = array( 'f', [ 0. ] )
  tout.Branch( 'eta2', eta2, 'eta2/F' )

  passSel = array( 'i', [ 0 ] )
  tout.Branch( 'passSel', passSel, 'passSel/I' )

  passNumTrig = array( 'i', [ 0 ] )
  tout.Branch( 'passNumTrig', passNumTrig, 'passNumTrig/I' )

  passDenTrig = array( 'i', [ 0 ] )
  tout.Branch( 'passDenTrig', passDenTrig, 'passDenTrig/I' )

  passDST = array( 'i', [ 0 ] )
  tout.Branch( 'passDST', passDST, 'passDST/I' )

  pass125 = array( 'i', [ 0 ] )
  tout.Branch( 'pass125', pass125, 'pass125/I' )

  pass157 = array( 'i', [ 0 ] )
  tout.Branch( 'pass157', pass157, 'pass157/I' )

  passDouble4p5mass = array( 'i', [ 0 ] )
  tout.Branch( 'passDouble4p5mass', passDouble4p5mass, 'passDouble4p5mass/I' )

  passDouble4dR = array( 'i', [ 0 ] )
  tout.Branch( 'passDouble4dR', passDouble4dR, 'passDouble4dR/I' )

  passL3 = array( 'i', [ 0 ] )
  tout.Branch( 'passL3', passL3, 'passL3/I' )

  eff125 = array( 'f', [ 0. ] )
  tout.Branch( 'eff125', eff125, 'eff125/F' )

  effDouble4p5mass = array( 'f', [ 0. ] )
  tout.Branch( 'effDouble4p5mass', effDouble4p5mass, 'effDouble4p5mass/F' )

  effOR = array( 'f', [ 0. ] )
  tout.Branch( 'effOR', effOR, 'effOR/F' )

  effL2 = array( 'f', [ 0. ] )
  tout.Branch( 'effL2', effL2, 'effL2/F' )

  effL3 = array( 'f', [ 0. ] )
  tout.Branch( 'effL3', effL3, 'effL3/F' )

  effData125 = array( 'f', [ 0. ] )
  tout.Branch( 'effData125', effData125, 'effData125/F' )

  effDataDouble4p5mass = array( 'f', [ 0. ] )
  tout.Branch( 'effDataDouble4p5mass', effDataDouble4p5mass, 'effDataDouble4p5mass/F' )

  effDataOR = array( 'f', [ 0. ] )
  tout.Branch( 'effDataOR', effDataOR, 'effDataOR/F' )

  effDataL2 = array( 'f', [ 0. ] )
  tout.Branch( 'effDataL2', effDataL2, 'effDataL2/F' )

  effDataL3 = array( 'f', [ 0. ] )
  tout.Branch( 'effDataL3', effDataL3, 'effDataL3/F' )

  run = array( 'i', [ 0 ] )
  tout.Branch( 'run', run, 'run/I' )

  ls = array( 'i', [ 0 ] )
  tout.Branch( 'ls', ls, 'ls/I' )

  event = array( 'i', [ 0 ] )
  tout.Branch( 'event', event, 'event/I' )

  L1_12_5_On = array( 'i', [ 0 ] )
  tout.Branch( 'L1_12_5_On', L1_12_5_On, 'L1_12_5_On/I' )

  maxiso = array( 'f', [ 0. ] )
  tout.Branch( 'maxiso', maxiso, 'maxiso/F' )

  met = array( 'f', [ 0. ] )
  tout.Branch( 'met', met, 'met/F' )

  ht = array( 'f', [ 0. ] )
  tout.Branch( 'ht', ht, 'ht/F' )

  softid = array( 'i', [ 0 ] )
  tout.Branch( 'softid', softid, 'softid/I' )

  mediumid = array( 'i', [ 0 ] )
  tout.Branch( 'mediumid', mediumid, 'mediumid/I' )

  tightid = array( 'i', [ 0 ] )
  tout.Branch( 'tightid', tightid, 'tightid/I' )


  vtxProb = array( 'f',[ 0. ] )
  tout.Branch( 'vtxProb', vtxProb, 'vtxProb/F' )

  valid = array( 'i',[ 0 ] )
  tout.Branch( 'valid', valid, 'valid/i' )

  lxy = array( 'f',[ 0. ] )
  tout.Branch( 'lxy', lxy, 'lxy/F' )

  lxyErr = array( 'f',[ 0. ] )
  tout.Branch( 'lxyErr', lxyErr, 'lxyErr/F' )

  sigLxy = array( 'f',[ 0. ] )
  tout.Branch( 'sigLxy', sigLxy, 'sigLxy/F' )



  # Load all the efficiencies
  f = {}
  eff = {}

  # Load the tag and probe efficiencies
  jpsidir = '/afs/cern.ch/work/d/dsperka/www/private/testTnP_Jpsi_dR03_vetoEMTF_Dec10/'
  for leg in ['L1_0_eta_0.0_0.9','L1_0_eta_0.9_1.9','L1_4p5_eta_0.0_0.9','L1_4p5_eta_0.9_1.9','L1_5_eta_0.0_0.9','L1_5_eta_0.9_1.9','L1_12_eta_0.0_0.9','L1_12_eta_0.9_1.9',
              'L2L1_eta_0.0_0.9','L2L1_eta_0.9_1.9','L3L2_eta_0.0_0.9','L3L2_eta_0.9_1.9']:

    if ('L2L1' in leg): era = 'BCDEF'
    else: era = 'CDEF'

    if ('L3L2' in leg):
      # don't have mc eff for L3|L2 in 2017 yet, so use the 2017 data one assuming SF is small
      f[leg+"_jpsi"] = TFile('/afs/cern.ch/work/d/dsperka/www/private/testTnP_Jpsi_dR03_vetoEMTF_Dec10/mu_JCB_bern4_'+leg+'_C.root','READ')
      eff[leg+"_jpsi_data"] = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_C")
      eff[leg+"_jpsi_mc"]   = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_C")

      ## don't have mc eff for L3|L2 in 2017 yet, so use the 2018 one assuming difference w.r.t 2017 is small 
      #f[leg+"_jpsi"] = TFile('/afs/cern.ch/work/d/dsperka/www/private/testTnP_Jpsi_dR03_vetoEMTF_2018_Jan21/mu_JCB_bern4_'+leg+'_CD.root','READ')
      #eff[leg+"_jpsi_data"] = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era)
      #eff[leg+"_jpsi_mc"]   = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era+"_ref")
    elif ('L1_' in leg and (not 'L2' in leg)):
      # use the one with no dR cut, looks better
      f[leg+"_jpsi"] = TFile('/afs/cern.ch/work/d/dsperka/www/private/testTnP_Jpsi_dR03_vetoEMTF_Dec10_testNoL1DRcut/mu_JCB_bern4_'+leg+'_'+era+'.root','READ')
      eff[leg+"_jpsi_data"] = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era)
      eff[leg+"_jpsi_mc"]   = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era+"_ref")
    else:
      f[leg+"_jpsi"] = TFile(jpsidir+'mu_JCB_bern4_'+leg+'_'+era+'.root','READ')
      eff[leg+"_jpsi_data"] = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era)
      eff[leg+"_jpsi_mc"]   = f[leg+"_jpsi"].Get("mu_JCB_bern4_"+leg+"_"+era+"_ref")

  zdir = '/afs/cern.ch/work/d/dsperka/www/private/testTnP_Z_dR03_vetoEMTF_Dec10/'
  for leg in ['L1_4p5_eta_0.0_0.9','L1_4p5_eta_0.9_1.9','L1_5_eta_0.0_0.9','L1_5_eta_0.9_1.9','L1_12_eta_0.0_0.9','L1_12_eta_0.9_1.9', 
              'L2L1_eta_0.0_0.9','L2L1_eta_0.9_1.9','L3L2_eta_0.0_0.9','L3L2_eta_0.9_1.9']:

    era = 'CDEF'

    f[leg+"_Z"] = TFile(zdir+'mu_BWDCB_bern4_'+leg+'_'+era+'.root','READ')
    eff[leg+"_Z_data"] = f[leg+"_Z"].Get("mu_BWDCB_bern4_"+leg+"_"+era)
    eff[leg+"_Z_mc"]   = f[leg+"_Z"].Get("mu_BWDCB_bern4_"+leg+"_"+era+"_ref")

  zlodir = '/afs/cern.ch/work/d/dsperka/www/private/testTnP_Z_dR03_vetoEMTF_Dec10_testNoL1DRcut/'
  for leg in ['L1_12_eta_0.0_0.9','L1_12_eta_0.9_1.2', 'L1_12_eta_1.2_1.9']:

    era = 'CDEF'

    f[leg+"_Zlo"] = TFile(zlodir+'/mu_BWDCB_bern4_'+leg+'_'+era+'.root','READ')
    eff[leg+"_Zlo_data"] = f[leg+"_Zlo"].Get("mu_BWDCB_bern4_"+leg+"_"+era)
    eff[leg+"_Zlo_mc"]   = f[leg+"_Zlo"].Get("mu_BWDCB_bern4_"+leg+"_"+era+"_ref")        

  # Load the scouting ID efficiency
  for leg in ['ID_0.0_0.9','ID_0.9_1.9']:
    f[leg+"_data"] = TFile('eff_data_'+leg+'.root','READ')
    eff[leg+"_data"] = f[leg+"_data"].Get('eff_data_'+leg)

    f[leg+"_mc"] = TFile('eff_mc_'+leg+'.root','READ')
    eff[leg+"_mc"] = f[leg+"_mc"].Get('eff_mc_'+leg)

  # Load the inv. mass. efficiency (only mc)
  f['mass7to18_mc'] = TFile("l1_masscut_eff_pt6_DY10to50.root","READ")
  eff['mass7to18_mc'] =  f['mass7to18_mc'].Get("l1_masscut_eff_pt6_DY10to50")

  # Load the correlation cuts efficiency (only mc)
  f['corrCuts_mc'] = TFile("l1_corrCuts_eff_DY10to50.root","READ")
  eff['corrCuts_mc'] = f['corrCuts_mc'].Get("l1_corrCuts_eff_DY10to50")



  Nthischain = t.GetEntries()
  #for ev in xrange(Nthischain):
  for ev in xrange(first,last):

    #if (ev>0): break

    t.GetEntry(ev)

    if (ev%1000==0): print ev,Nthischain


    # Now applying the selection
    run[0] = t.run
    ls[0] = t.lumSec
    event[0] = t.event

    if (injson(str(run[0]),ls[0])): L1_12_5_On[0] = 1
    else:  L1_12_5_On[0] = 0

    passNumTrig[0] = 0
    pass125[0] = 0
    pass157[0] = 0
    passDouble4p5mass[0] = 0
    passDouble4dR[0] = 0
    if (t.l1Result[0]): 
      passNumTrig[0] = 1
      pass125[0] = 1
    if (t.l1Result[4]): 
      passNumTrig[0] = 1
      pass157[0] = 1
    if (t.l1Result[11]): 
      passNumTrig[0] = 1
      passDouble4p5mass[0] = 1
    if (t.l1Result[12]): 
      passNumTrig[0] = 1
      passDouble4dR[0] = 1

    #if (t.l1Result[4]): passNumTrig[0] = 1
    #if (t.l1Result[12]): passNumTrig[0] = 1

    # for making eff. w.r.t. prescaled L1 trigger L1_DoubleMu0_SQ
    if (t.l1Result[len(t.l1Result)-1]): passDenTrig[0] = 1
    else: passDenTrig[0] = 0

    # check HLT is passed
    if (t.trig==1): passDST[0] = 1
    else: passDST[0] = 0

    #if (not passDenTrig[0]): continue

    passSel[0] = 0
    pt1[0] = -9.0
    eta1[0] = -9.0
    pt2[0] = -9.0
    eta2[0] = -9.0
    mll[0] = -9.0
    ptll[0] = -9.0
    drll[0] = -9.0
    tightid[0] = 0
    mediumid[0] = 0
    softid[0] = 0
    maxiso[0] = -9.0

    met[0] = -9.0
    
    ht[0] = 0.0
    for j in range(len(t.jets)): 
      if (abs(t.jets[j].Eta())<2.4): ht[0] += t.jets[j].Pt()
  
    maxsumpt=0.0
    
    nmu = len(t.muons)

    if (nmu<2): continue


    if (t.nvtx>0 and nmu>=2):

      for mu1 in range(nmu):
        for mu2 in range(mu1+1,nmu):
            

          # loose id
          if (not bool(t.midloose[mu1])): continue
          if (not bool(t.midloose[mu2])): continue

          if (not ((t.mid[mu1] + t.mid[mu2])==0)): continue

          passSel[0]=1

          muon1 = t.muons[mu1]
          muon2 = t.muons[mu2]

          sumpt = muon1.Pt()+muon2.Pt()
          if (sumpt<maxsumpt): continue
          maxsumpt=sumpt

          if (t.muons[mu1].Pt()>t.muons[mu2].Pt()):
            pt1[0] = t.muons[mu1].Pt()
            eta1[0] = t.muons[mu1].Eta()
            pt2[0] = t.muons[mu2].Pt()
            eta2[0] = t.muons[mu2].Eta()
          else:
            pt1[0] = t.muons[mu2].Pt()
            eta1[0] = t.muons[mu2].Eta()
            pt2[0] = t.muons[mu1].Pt()
            eta2[0] = t.muons[mu1].Eta()
  
          dimu = muon1+muon2
          
          mll[0] = dimu.M()
          ptll[0] = dimu.Pt()
          drll[0] = muon1.DeltaR(muon2)

          maxiso[0]=max(t.miso[mu1],t.miso[mu2])

          if (bool(t.midsoft[mu1]) and bool(t.midsoft[mu2])): softid[0]=1
          else: softid[0]=0

          if (bool(t.midmedium[mu1]) and bool(t.midmedium[mu2])): mediumid[0]=1
          else: mediumid[0]=0

          if (bool(t.midtight[mu1]) and bool(t.midtight[mu2])): tightid[0]=1
          else: tightid[0]=0

    if (not passSel[0]==1): continue

    eff125[0]=1.0
    effDouble4p5mass[0]=1.0
    effL2[0]=1.0
    effL3[0]=1.0

    effData125[0]=1.0
    effDataDouble4p5mass[0]=1.0
    effDataL2[0]=1.0
    effDataL3[0]=1.0

    ########## MC eff. ################

    # use the Z results if they are available, or else jpsi        
    # first leading leg.
    if (pt1[0]>15.0): 
      histpt = min(pt1[0],49.9) 
      if (abs(eta1[0])<0.9):
        eff125[0] *= eff["L1_12_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.0_0.9_Z_mc"].Eval(histpt)
      elif (abs(eta1[0])<1.9):
        eff125[0] *= eff["L1_12_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.9_1.9_Z_mc"].Eval(histpt)
      else:
        eff125[0] *= 0.0
        effDouble4p5mass[0] *= 0.0
        effL2[0] *= 0.0
        effL3[0] *= 0.0
    elif (pt1[0]>4.0):
      histpt = min(pt1[0],14.75) 
      if (abs(eta1[0])<0.9):
        if (histpt>8.0):  
          eff125[0] *= eff["L1_12_eta_0.0_0.9_Zlo_mc"].Eval(histpt)          
        else:
          eff125[0] *= eff["L1_12_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_jpsi_mc"].Eval(histpt) 
        effL2[0] *= eff["L2L1_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
      elif (abs(eta1[0])<1.9):
        if (histpt>8.0):
          if (abs(eta1[0])<1.2): eff125[0] *= eff["L1_12_eta_0.9_1.2_Zlo_mc"].Eval(histpt)
          else: eff125[0] *= eff["L1_12_eta_1.2_1.9_Zlo_mc"].Eval(histpt)
        else: eff125[0] *= eff["L1_12_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
      else:
        eff125[0] *= 0.0
        effDouble4p5mass[0] *= 0.0
        effL2[0] *= 0.0
        effL3[0] *= 0.0
    else:
      eff125[0] *= 0.0
      effDouble4p5mass[0] *= 0.0
      effL2[0] *= 0.0
      effL3[0] *= 0.0

    #pt1 14.9811258316 eta1 0.876170635223 pt2 12.5327596664 eta2 0.777780294418
    # second leg.
    if (pt2[0]>13.0): 
      histpt = min(pt2[0],49.5) 
      if (abs(eta2[0])<0.9):
        eff125[0] *= eff["L1_5_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.0_0.9_Z_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.0_0.9_Z_mc"].Eval(histpt)
      elif (abs(eta2[0])<1.2):
        eff125[0] *= eff["L1_5_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.9_1.9_Z_mc"].Eval(histpt)
      elif (abs(eta2[0])<1.9):
        eff125[0] *= eff["L1_5_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.9_1.9_Z_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.9_1.9_Z_mc"].Eval(histpt)
      else:
        eff125[0] *= 0.0
        effDouble4p5mass[0] *= 0.0
        effL2[0] *= 0.0
        effL3[0] *= 0.0
    elif (pt2[0]>4.0):
      histpt = min(pt2[0],12.5) 
      if (abs(eta2[0])<0.9):
        eff125[0] *= eff["L1_5_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_jpsi_mc"].Eval(histpt) 
        effL2[0] *= eff["L2L1_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.0_0.9_jpsi_mc"].Eval(histpt)
      elif (abs(eta2[0])<1.9):
        eff125[0] *= eff["L1_5_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effL2[0] *= eff["L2L1_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
        effL3[0] *= eff["L3L2_eta_0.9_1.9_jpsi_mc"].Eval(histpt)
      else:
        eff125[0] *= 0.0
        effDouble4p5mass[0] *= 0.0
        effL2[0] *= 0.0
        effL3[0] *= 0.0
    else:
      eff125[0] *= 0.0
      effDouble4p5mass[0] *= 0.0
      effL2[0] *= 0.0
      effL3[0] *= 0.0


    effOR[0] = eff125[0] + effDouble4p5mass[0] - eff125[0]*effDouble4p5mass[0]

    ########## Data eff. ################

    # use the Z results if they are available, or else jpsi        
    # first leading leg.
    if (pt1[0]>15.0): 
      histpt = min(pt1[0],49.9) 
      if (abs(eta1[0])<0.9):
        effData125[0] *= eff["L1_12_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.0_0.9_Z_data"].Eval(histpt)
      elif (abs(eta1[0])<1.9):
        effData125[0] *= eff["L1_12_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.9_1.9_Z_data"].Eval(histpt)
      else:
        effData125[0] *= 0.0
        effDataDouble4p5mass[0] *= 0.0
        effDataL2[0] *= 0.0
        effDataL3[0] *= 0.0
    elif (pt1[0]>4.0):
      histpt = min(pt1[0],14.75) 
      if (abs(eta1[0])<0.9):
        if (histpt>8.0):  
          effData125[0] *= eff["L1_12_eta_0.0_0.9_Zlo_data"].Eval(histpt)          
        else:
          effData125[0] *= eff["L1_12_eta_0.0_0.9_jpsi_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_jpsi_data"].Eval(histpt) 
        effDataL2[0] *= eff["L2L1_eta_0.0_0.9_jpsi_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.0_0.9_jpsi_data"].Eval(histpt)
      elif (abs(eta1[0])<1.9):
        if (histpt>8.0):
          if (abs(eta1[0])<1.2): effData125[0] *= eff["L1_12_eta_0.9_1.2_Zlo_data"].Eval(histpt)
          else: effData125[0] *= eff["L1_12_eta_1.2_1.9_Zlo_data"].Eval(histpt)
        else: effData125[0] *= eff["L1_12_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.9_1.9_jpsi_data"].Eval(histpt)
      else:
        effData125[0] *= 0.0
        effDataDouble4p5mass[0] *= 0.0
        effDataL2[0] *= 0.0
        effDataL3[0] *= 0.0
    else:
      effData125[0] *= 0.0
      effDataDouble4p5mass[0] *= 0.0
      effDataL2[0] *= 0.0
      effDataL3[0] *= 0.0

    # second leg.
    if (pt2[0]>13.0): 
      histpt = min(pt2[0],49.5) 
      if (abs(eta2[0])<0.9):
        effData125[0] *= eff["L1_5_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.0_0.9_Z_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.0_0.9_Z_data"].Eval(histpt)
      elif (abs(eta2[0])<1.2):
        effData125[0] *= eff["L1_5_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.9_1.9_Z_data"].Eval(histpt)
      elif (abs(eta2[0])<1.9):
        effData125[0] *= eff["L1_5_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.9_1.9_Z_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.9_1.9_Z_data"].Eval(histpt)
      else:
        effData125[0] *= 0.0
        effDataDouble4p5mass[0] *= 0.0
        effDataL2[0] *= 0.0
        effDataL3[0] *= 0.0
    elif (pt2[0]>4.0):
      histpt = min(pt2[0],12.5) 
      if (abs(eta2[0])<0.9):
        effData125[0] *= eff["L1_5_eta_0.0_0.9_jpsi_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.0_0.9_jpsi_data"].Eval(histpt) 
        effDataL2[0] *= eff["L2L1_eta_0.0_0.9_jpsi_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.0_0.9_jpsi_data"].Eval(histpt)
      elif (abs(eta2[0])<1.9):
        effData125[0] *= eff["L1_5_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataDouble4p5mass[0] *= eff["L1_4p5_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataL2[0] *= eff["L2L1_eta_0.9_1.9_jpsi_data"].Eval(histpt)
        effDataL3[0] *= eff["L3L2_eta_0.9_1.9_jpsi_data"].Eval(histpt)
      else:
        effData125[0] *= 0.0
        effDataDouble4p5mass[0] *= 0.0
        effDataL2[0] *= 0.0
        effDataL3[0] *= 0.0
    else:
      effData125[0] *= 0.0
      effDataDouble4p5mass[0] *= 0.0
      effDataL2[0] *= 0.0
      effDataL3[0] *= 0.0

    effDataOR[0] = effData125[0] + effDataDouble4p5mass[0] - effData125[0]*effDataDouble4p5mass[0]


    # fill the tree for every event      
    tout.Fill()


  # Done with loop, write the file
  fout.Write()
  fout.Close()
    


if __name__ == "__main__":
  fillTree()


