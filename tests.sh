( ./test_message_on_file w ; ./test_message_on_file r )                    > message_on_file.txt
./test_message_on_iostream                                                 > message_on_iostream.txt
./test_message_on_separate_stringstream                                    > message_on_separate_stringstream.txt
( ./test_message_on_file w ; cat output.dat | ./test_message_on_file - )   > message_from_cat.txt
( ./test_message_on_file w ; dd if=output.dat | ./test_message_on_file - ) > message_from_dd.txt
( ./test_message_on_file w ; ./test_message_on_file - < ./output.dat )     > message_from_pipe.txt
md5sum message_*
