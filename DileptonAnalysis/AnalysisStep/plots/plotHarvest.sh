for d in /eos/user/c/ccosby/triggerSamples/* ; do
    echo "Running on sample $d"
    cp plot.py $d/*/*/*/*
    cd $d*/*/*/*
    python plot.py
    pwd
    rm plot.py
    cd -
done
