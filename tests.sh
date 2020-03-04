( ./test_message_on_file w out.dat ; ./test_message_on_file r out.dat ) > message_on_file.txt
./test_message_on_iostream                                              > message_on_iostream.txt
./test_message_on_separate_stringstream                                 > message_on_separate_stringstream.txt
( ./test_message_on_file w ; cat out.dat | ./test_message_on_file - )   > message_from_cat.txt
( ./test_message_on_file w ; dd if=out.dat | ./test_message_on_file - ) > message_from_dd.txt
( ./test_message_on_file w ; ./test_message_on_file - < ./out.dat )     > message_from_pipe.txt
md5sum message_*
