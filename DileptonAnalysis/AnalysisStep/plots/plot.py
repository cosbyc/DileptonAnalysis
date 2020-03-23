from ROOT import *

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
    
    # store options and arguments as global variables
    global opt, args
    (opt, args) = parser.parse_args()


def find(name, path):
    for root, dirs, files in os.walk(path):
        if name in files:
            print(str(os.path.join(root, name)))
            return os.path.join(root, name)

def plot():

    global opt, args
    parseOptions()

    f1 = TFile(str(find("postProc_1.root", opt.INPUT)),"READ")
    t1 = f1.Get("tree")

    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)

    h = TH1F("h","h", 50, 0, 10)
    t1.Draw("lxy>>h","","goff")

    ### Use "goff same" for the second two
    ### and don't use "matTauTau>>h_matTauTau".... just "matTauTau"
    c1 = TCanvas("c1","c1",900,800)
    c1.SetLeftMargin(0.15)
    c1.SetRightMargin(0.05)
    c1.SetLogy()
    c1.cd()


    h.SetLineColor(2)
    h.SetLineWidth(5)

    h.GetXaxis().SetTitle("Lxy [mm]")
    h.GetYaxis().SetTitle("Events")
    h.Draw("hist")



    leg = TLegend(0.6,0.7,0.85,0.8)
    leg.AddEntry(h,"DY Simulation","l")
    leg.Draw("same")

    latex2 = TLatex()
    latex2.SetNDC()
    latex2.SetTextSize(0.4*c1.GetTopMargin())
    latex2.SetTextFont(42)
    latex2.SetTextAlign(31)
    #latex2.DrawLatex(0.82, 0.93,"59.7 fb^{-1} (13 TeV)")
    latex2.SetTextSize(0.5*c1.GetTopMargin())
    latex2.SetTextFont(62)
    latex2.SetTextAlign(11)
    latex2.DrawLatex(0.15, 0.93, "CMS")
    latex2.SetTextSize(0.45*c1.GetTopMargin())
    latex2.SetTextFont(52)
    latex2.SetTextAlign(11)
    #latex2.DrawLatex(0.26, 0.93, "Preliminary")


    c1.SaveAs(opt.OUTPUT)

if __name__ == "__main__":
  plot()
