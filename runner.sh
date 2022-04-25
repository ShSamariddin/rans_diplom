for file in res/*;
 do
 	filename=$(basename -- "$file")
 	init_file="wfd/${filename}"
 	diff $file $init_file
 done