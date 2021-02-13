mkdir -p report
for var in ./testfiles/*.gra; do
noext=${var%.*}	# remove extension
nopre=${noext##*/} 	# keep just name, remove path
#echo "$nopre"
#../../grail "$noext.gra" 5 "$noext.que" >> standard.report
valgrind --tool=massif --massif-out-file=./report/"$nopre.massif" ../../grail "$noext.gra" 5 "$noext.que" >> standard.report
done
