# Argument for this script is program filename of second player

cat data/s11.txt > data/status.txt
cat data/s12.txt > data/status2.txt
./bin/KnightsMediator data/mapa1.txt bin/Knights bin/$1 data/status.txt data/status2.txt data/rozkazy.txt data/rozkazy2.txt 3

cat data/s21.txt > data/status.txt
cat data/s22.txt > data/status2.txt
./bin/KnightsMediator data/mapa2.txt bin/Knights bin/$1 data/status.txt data/status2.txt data/rozkazy.txt data/rozkazy2.txt 3

cat data/s31.txt > data/status.txt
cat data/s32.txt > data/status2.txt
./bin/KnightsMediator data/mapa3.txt bin/Knights bin/$1 data/status.txt data/status2.txt data/rozkazy.txt data/rozkazy2.txt 3

cat data/s41.txt > data/status.txt
cat data/s42.txt > data/status2.txt
./bin/KnightsMediator data/mapa4.txt bin/Knights bin/$1 data/status.txt data/status2.txt data/rozkazy.txt data/rozkazy2.txt 3

