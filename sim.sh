#!/bin/bash
options=("--so")
#variants=("TcpBic" "TcpHybla" "TcpHtcp" "TcpHighSpeed") # "TcpNewReno")
variants=("TcpBic" "TcpNewReno-Bic")
vvs=("Bic" "Hybla" "Htcp" "HighSpeed")
# vvs=("HighSpeed")
# drop_p=(0.01 0.001 0.0001 0.00001 0.000001)
drop_p=(0.001)
nosim=0
noplot=0
noanalyze=0
plot=""
pp=0.00001
t=Tcp
# vv=Bic

# rm -r ./data/*

for p in ${drop_p[@]}
do
  mkdir -p ./data/${p}
done

for i in "$@" ; do
  if [[ $i == "--so" ]] ;
  then
    nosim=1
    echo "Selected not to run simulator"
  elif [[ $i == "--ao" ]]
  then
    noanalyze=1
    echo "Selected not to run analyzer"
  elif [[ $i == "--po" ]]
  then
    noplot=1
    echo "Selected not to plot"
  else [[ $i == "--one" ]]
    variants=("TcpBic")
    echo "Selected to simulate only one variant"
  fi
done

for vv in ${vvs[@]}
do
    for p in ${drop_p[@]}
    do
        if [ ! $nosim -eq 1 ]
        then
            ./dumbbell \
            --legacy \
            --duration=60.0 \
            --num_flows=1 \
            --transport_prot=${vv} \
            --access_bandwidth=100Mbps \
            --access_delay=10ms \
            --bandwidth=10Mbps \
            --error_p=${p} \
            --delay=10ms \
            --run=1234 \
            --tracing \
            --prefix_name=data/${p}/Tcp
        fi
    done
done

for v in ${variants[@]}
do
    if [ ! $noanalyze -eq 1 ]
    then
        for p in ${drop_p[@]}
        do
            python calc_moving_avg_tp.py $v $p
        done
    fi

    if [ ! -z "$plot" ]
    then
        plot="${plot}, "
    else
        plot="plot "
    fi
    plot="${plot}'./data/f_c/100ms/${pp}/${v}-mov-avg.data' using 1:2 title '$v'"
done

if [ ! $noplot -eq 1 ]
then
  gnuplot moving-avg.gnu -e "$plot"
fi