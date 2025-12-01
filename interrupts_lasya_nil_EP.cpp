/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include<interrupts_lasya_nil.hpp>

void EP(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.PID > second.PID); 
                } 
            );
}

//bonus mem
std::string print_mem_status(unsigned int current_time){
    const int tableWidth = 62;
    std::stringstream buffer;

    unsigned int total_used = 0;
    unsigned int total_free = 0;
    unsigned int avail_free = 0;


    // Print top border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << "\n";
    buffer << "Memory Status at Time " << current_time << "\n";
    // Print headers
    buffer << "|"
              << std::setfill(' ') << std::setw(10) << "partition number"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(12) << " partition size"
              << std::setw(2) << "|" 
              << std::setfill(' ') << std::setw(12) << " partition status"
              << std::setw(2) << "|"
              << std::setfill(' ') << std::setw(5) << "PID"
              << std::setw(2) << "|" << "\n";
    
    // Print separator
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << "\n";
    
    for (int i = 0; i< 6; i++){
        buffer << "|"
                  << std::setfill(' ') << std::setw(16) << memory_paritions[i].partition_number
                  << std::setw(2) << "|"
                  << std::setw(15) << memory_paritions[i].size
                  << std::setw(2) << "|"; 
        
        if(memory_paritions[i].occupied == -1){
            buffer << std::setfill(' ') << std::setw(17) << "FREE"
                  << std::setw(2) << "|" 
                  << std::setw(7) << "NONE"
                << std::setw(2) << "|\n";
            total_free += memory_paritions[i].size;
            avail_free += memory_paritions[i].size;
        }
        else {
            buffer << std::setfill(' ') << std::setw(17) << "BUSY"
                << std::setw(2) << "|"
                << std::setw(7) << memory_paritions[i].occupied
                << std::setw(2) << "|\n";
            total_used += memory_paritions[i].size;
        }

    }
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    buffer << "total memory used at " << current_time << " : " << total_used<< std::endl;
    buffer << "total free memory at " << current_time << " : " << total_free<< std::endl;
    buffer << "total available free memory at " << current_time << " : " << avail_free<< std::endl;
    // Print bottom border
    buffer << "+" << std::setfill('-') << std::setw(tableWidth) << "+" << std::endl;

    return buffer.str();
}

std::tuple<std::string, std::string > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).
    std::vector<PCB> block_queue; // this is for memory management, what if memory is not available?

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;
    //bonus part
    std::string mem_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {//check if the AT = current time
                //if so, assign memory and put the process into the ready queue
                bool mem_assigned = assign_memory(process);
                if (mem_assigned) {
                    process.state = READY;  //Set the process state to READY
                    ready_queue.push_back(process); //Add the process to the ready queue
                    job_list.push_back(process); //Add it to the list of processes

                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                    mem_status += print_mem_status(current_time);
                } else {
                    process.state = NEW;
                    block_queue.push_back(process);
                }
            }
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready queue
        for(auto i = wait_queue.begin(); i != wait_queue.end();){
            if(current_time >= (*i).start_time + (*i).io_duration){
                (*i).state = READY;
                ready_queue.push_back(*i);
                execution_status += print_exec_status(current_time, (*i).PID, WAITING, READY);
                sync_queue(job_list, *i);
                i = wait_queue.erase(i);
            }else{
                ++i;
            }
        }

        //bonus mem management
        for(auto i = block_queue.begin(); i != block_queue.end();){
            bool mem_assigned = assign_memory(*i);
            if (mem_assigned){
                    (*i).state = READY;  //Set the process state to READY
                    ready_queue.push_back(*i); //Add the process to the ready queue
                    job_list.push_back(*i); //Add it to the list of processes
                    execution_status += print_exec_status(current_time, (*i).PID, NEW, READY);
                    mem_status += print_mem_status(current_time);
                    i = block_queue.erase(i);
            }else{
                ++i;
            }
        }

        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////
        //FCFS(ready_queue); //example of FCFS is shown here

        if(running.state == RUNNING){
            running.remaining_time--;

            //check for IO
            if(running.io_freq > 0 && running.remaining_time > 0 && 
                (running.processing_time - running.remaining_time) % running.io_freq == 0 &&
                (running.processing_time - running.remaining_time) > 0){
                    running.state = WAITING;
                    running.start_time = current_time;
                    wait_queue.push_back(running);
                    execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
                    sync_queue(job_list, running);
                    idle_CPU(running);
            }

            //check process complete
            else if (running.remaining_time == 0){
                execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
                terminate_process(running, job_list);
                mem_status += print_mem_status(current_time);
                idle_CPU(running);

            }
        }
        
        if(running.state == NOT_ASSIGNED && !ready_queue.empty()){
            EP(ready_queue);
            run_process(running, job_list, ready_queue, current_time);
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
        }
        current_time++;

        /////////////////////////////////////////////////////////////////

    }
    
    //Close the output table
    execution_status += print_exec_footer();

    return std::make_tuple(execution_status, mem_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec, mem] = run_simulation(list_process);

    write_output(exec, "execution.txt");
    write_output(mem, "memory_status.txt");

    return 0;
}