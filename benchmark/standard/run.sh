for var in ./testfiles/*.gra; do
tmp=${var%.*}	# remove extension
echo ${tmp##*/}	# print just name, not the path
../../grail "$tmp.gra" 5 "$tmp.que" >> standard.report
#valgrind --tool=massif ../../grail "$tmp.gra" 5 "$tmp.que" >> standard.report
done
