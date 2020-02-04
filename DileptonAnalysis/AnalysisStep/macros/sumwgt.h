#ifndef SUMWGT_H
#define SUMWGT_H


double sumwgt(const char* treepath, const char* wgtstr = "wgtsign") {
    TFileCollection fc("fc");
    fc.Add(treepath);
    TChain* chain = new TChain("gentree/tree");
    chain->AddFileInfoList(fc.GetList());

    TTreeReader reader(chain);
    TTreeReaderValue<double> weight(reader, wgtstr);

    double weightsum = 0.;
    while(reader.Next()) weightsum += (*weight);
    return weightsum;
}

#endif
