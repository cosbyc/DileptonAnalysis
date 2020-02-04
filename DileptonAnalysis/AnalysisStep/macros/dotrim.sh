#!/bin/bash

declare -a D=()
declare -a M=()

D+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/SingleMuon*` )
#D+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/DoubleMuon*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/DY*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/TT*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/ST*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/tZ*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/WW*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/WZ*` )
#M+=( `ls -d /media/Disk1/avartak/CMS/Data/Dileptons/ZZ*` )

for X in ${D[@]};
do
    SCRIPT=trim.C\(\"${X}/tree*.root\"\,\"${X}/trim.root\",false\)
    root -l -b -q $SCRIPT
done

for X in ${M[@]};
do
    SCRIPT=trim.C\(\"${X}/tree*.root\"\,\"${X}/trim.root\",true\)
    root -l -b -q $SCRIPT
done


