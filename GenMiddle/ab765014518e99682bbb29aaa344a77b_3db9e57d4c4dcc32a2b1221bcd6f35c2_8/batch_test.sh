#!/bin/bash
rm mips.txt
rm 4code.txt
rm myresult.txt
rm jjresult.txt
rm -r wrong_samples
mkdir wrong_samples

for file in ./*.in
do
   if [ $file ]; then
     echo "testing $file"
cp $file ./testfile.txt

../Debug/GenMiddle.exe 1>/dev/null
java -jar MARS-jdk7-Re.jar mips.txt 1>myresult.txt
rm tmpCode.txt

if diff -q myresult.txt ${file%.*}.out ;then
	echo "~~~~~~~~~~~~~~~~~~~the Same~~~~~~~~~~~~~~~~~"
	rm testfile.txt
else 
	echo "WAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWA"
	cp $file ./wrong_samples/
	new=${file#*/}
	d="cp myresult.txt ./wrong_samples/my_${new%.*}.out"
	eval $d
	s="cp ${file%.*}.out ./wrong_samples/"
	eval $s
fi

fi
done
