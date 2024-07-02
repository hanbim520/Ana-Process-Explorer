/*	Project Name:	Ana Process Explorer
*	Written By	:	Ahmad			Siavashi 	->	Email: a.siavosh@yahoo.com,
*					Ali				Kianinejad	->	Email: af.kianinejad@gmail.com,
*				 	Farid			Amiri,
*				 	Mohammad Javad	Moein.
*	Course Title:	Principles of Programming.
*	Instructor	: 	Dr. Ali Hamze.
*	T.A			: 	Mr. Hojat Doulabi.
*	Shiraz University, Shiraz, Iran.
*	Spring 1391, 2012.
*/
#include <iostream>
#include <vector>
#include <fstream>
#include "AnaMainHeader.h"
#include "StartApplication.h"

void writeCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return;
    }

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

int  main(int argc, char* argv[]){

    if (argc >= 3) {
        std::string ProcessName = argv[1];
        std::string outFilePath = argv[2];

        std::vector<std::vector<std::string>> data = {
        {
            "Private Usage(MB)","WorkingSet(MB)", "PrivateWorkingSet(MB)", "SharedCommit Usage(MB)"}
        };

        std::string filename = outFilePath;

        writeCSV(filename, data);


        StartApplication(ProcessName, outFilePath);
    }
    else
    {
        StartApplication();
    }

    
	
	return EXIT_SUCCESS;
}