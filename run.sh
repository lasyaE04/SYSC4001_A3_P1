sh build.sh
cd bin

for i in {1..20}; do
    ./interrupts_RR ../input_files/test${i}.txt
    cp execution.txt ../output_files/execution${i}_RR.txt
    cp memory_status.txt ../output_files/memory_statuses/memory_status${i}_RR.txt

    ./interrupts_EP_RR ../input_files/test${i}.txt
    cp execution.txt ../output_files/execution${i}_EP_RR.txt
    cp memory_status.txt ../output_files/memory_statuses/memory_status${i}_EP_RR.txt
done
