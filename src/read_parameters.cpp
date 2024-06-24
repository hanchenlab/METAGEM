/****************************************************************************
  ReadParameters.cpp reads parameters from a file
****************************************************************************/
#include "metagem.h"


void print_help();

// Function to process command line arguments
void CommandLine::processCommandLine(int argc, char* argv[]) {

    // GEM options. Details are printed from the print_help() function below.
    CLI::App app{""};

    // Defaults
    int metaOpt_in = 0;
    std::string outFile_in = "metagem.out";

    app.add_option("--input-files", fileNames, "")->expected(0, 1000000);
    app.add_option("--input-file-list", metaFileList, "")->expected(1);
    app.add_option("--exposure-names", intNames, "")->expected(1, 1000000)->required();
    app.add_option("--out", outFile_in, "")->expected(1);
    app.add_option("--meta-option", metaOpt_in, "");
    app.add_option("--additional-joint", additionalJointInfo, "") -> expected(0, 100);
    app.add_option("--additional-interaction", additionalInteractionInfo, "") -> expected(0, 100);
  
    try
    {
        app.parse( argc, argv);

        // Input files
        size_t fns = fileNames.size();
        size_t fls = metaFileList.length();
        if (fns == 0 && fls == 0) {
            cerr << "\nERROR: --input-files or --input-file-list is required.\n\n";
            exit(1);

        } else if (fns > 0 && fls > 0) {
            cerr << "\nERROR: Both --input-files and --input-file-list are specified.\n\n";
            exit(1);

        } else if (fns > 0) {
            if (fns < 2) {
                cerr << "\nERROR: METAGEM requires at least 2 input files.\n\n";
                exit(1);                
            }

            std::set<std::string> s(fileNames.begin(), fileNames.end());
            if (s.size() != fns) 
            {
                cerr << "\nERROR: There are duplicate input file names.\n\n";
                exit(1);
            }

        } else if (fls > 0) {
            std::ifstream file;
            std::string line;
            file.open(metaFileList);
            if (!file.is_open()) {
                cerr << "\nERROR: Cannot open the file: [" << metaFileList << "].\n\n";
                exit(1);
            }

            size_t nfiles = 0;
            while(getline(file, line)) {
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                fileNames.push_back(line);
                nfiles++;
            }

            if (nfiles <= 2)
            {
                cerr << "\nERROR: METAGEM requires at least 2 input files.\n\n";
                exit(1);
            }
        
            std::set<std::string> s(fileNames.begin(), fileNames.end());
            if (s.size() != nfiles) 
            {
                cerr << "\nERROR: There are duplicate input file names.\n\n";
                exit(1);
            }

        } else {
            cerr << "\nERROR: Unrecognized combination of --input-files and --input-file-list.\n\n";
            exit(1);
        }



        // Exposure names
        std::set<std::string> s(intNames.begin(), intNames.end());
        if (s.size() != intNames.size()) {
            cerr << "\nERROR: There are duplicate exposure names (--exposure-names).\n\n";
            exit(1);
        }
        nInt = intNames.size();

        lcIntNames = intNames;
        for(std::string &s : lcIntNames){
            std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
            s = "g-" + s;
        }
        lcIntNames.insert(lcIntNames.begin(), "g");



        // Output file
        outFile = outFile_in;
        std::ofstream results(outFile);
        if (!results) {
            printOpenFileError(outFile);
        }

        if (results.fail()) {
            printOpenFileError(outFile);
        }

        results << "test" << endl;
        if (results.fail()) {
            cerr << "\nERROR: Cannot write to output file.\n\n";
            results.close();
            
            if (std::remove(outFile.c_str()) != 0) {
                cerr << "\nERROR: Cannot delete output file.\n\n";
            }
            exit(1);
        }
        results.close();
        
        if (std::remove(outFile.c_str()) != 0) {
            cerr << "\nERROR: Cannot delete output file.\n\n";
            exit(1);
        }

        

        // Meta Option
        metaOpt = metaOpt_in;
        if (metaOpt < 0 || metaOpt > 2) {
            cerr << "\nERROR: The --meta-option integer value must be 0, 1, or 2.\n\n";
            exit(1);
        }

        if (metaOpt == 0) {
            mb = true;
            rb = true;
            cout << "Meta-analysis option: [model-based] and [robust].\n\n"; 
        } else if (metaOpt == 1) {
            mb = true;
            cout << "Meta-analysis option: [model-based].\n\n";
        } else if (metaOpt == 2) {
            rb = true;
            cout << "Meta-analysis option: [robust].\n\n";
        } else {
            cerr << "\nERROR: The --meta-option integer value must be 0, 1, or 2.\n\n";
            exit(1);
        }

        // Additional joint test
        if (!additionalJointInfo.empty()) {
          additionalJoint = true;
          
          if (additionalJointInfo.size() == 1) {
            cerr << "ERROR: Both of the variable name and full path of the additional joint test output file should be specified.\n\n";
            exit(1); 
          }

          // Check if the full path of the additional output file specified at the end
          const std::string& lastTestInfo = additionalJointInfo.back();
          for (const auto& name : intNames) {
            if (lastTestInfo == name) {
              cerr << "ERROR: Please specify the full path of the additional output file at the end of '--additional-joint' flag.\n\n";
              exit(1);
            }
          }
          
          intNames2.assign(additionalJointInfo.begin(), additionalJointInfo.end() - 1);
          outFile2 = additionalJointInfo.back();
          std::set<std::string> s(intNames2.begin(), intNames2.end());
          if (s.size() != intNames2.size()) {
              cerr << "\nERROR: There are duplicate exposure names in the additional joint test.\n\n";
              exit(1);
          }
          nInt2 = intNames2.size();
          
          lcIntNames2 = intNames2;
          for(std::string &s : lcIntNames2){
              std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
              s = "g-" + s;
          }
          lcIntNames2.insert(lcIntNames2.begin(), "g");

          // Additional output file
          std::ofstream results2(outFile2);
          if (!results2) {
              printOpenFileError(outFile2);
          }

          if (results2.fail()) {
              printOpenFileError(outFile2);
          }

          results2 << "test" << endl;
          if (results2.fail()) {
              cerr << "\nERROR: Cannot write to the additional joint test output file.\n\n";
              results2.close();
            
              if (std::remove(outFile2.c_str()) != 0) {
                  cerr << "\nERROR: Cannot delete the additional joint test output file.\n\n";
              }
              exit(1);
          }
          results2.close();
        
          if (std::remove(outFile2.c_str()) != 0) {
              cerr << "\nERROR: Cannot delete the additional joint test output file.\n\n";
              exit(1);
          }
        }

        // Additional interaction test
        if (!additionalInteractionInfo.empty()) {
          additionalInteraction = true;
          
          if (additionalInteractionInfo.size() == 1) {
            cerr << "ERROR: Both of the variable name and full path of the additional interaction-only test output file should be specified.\n\n";
            exit(1); 
          }

          // Check if the full path of the additional output file specified at the end
          const std::string& lastTestInfo = additionalInteractionInfo.back();
          for (const auto& name : intNames) {
            if (lastTestInfo == name) {
              cerr << "ERROR: Please specify the full path of the additional output file at the end of '--additional-interaction' flag.\n\n";
              exit(1);
            }
          }
          
          intNames3.assign(additionalInteractionInfo.begin(), additionalInteractionInfo.end() - 1);
          outFile3 = additionalInteractionInfo.back();
          std::set<std::string> s(intNames3.begin(), intNames3.end());
          if (s.size() != intNames3.size()) {
              cerr << "\nERROR: There are duplicate exposure names in the additional interaction-only test.\n\n";
              exit(1);
          }
          nInt3 = intNames3.size();
          
          lcIntNames3 = intNames3;
          for(std::string &s : lcIntNames3){
              std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
              s = "g-" + s;
          }

          // Additional output file
          std::ofstream results3(outFile3);
          if (!results3) {
              printOpenFileError(outFile3);
          }

          if (results3.fail()) {
              printOpenFileError(outFile3);
          }

          results3 << "test" << endl;
          if (results3.fail()) {
              cerr << "\nERROR: Cannot write to the additional interaction-only test output file.\n\n";
              results3.close();
            
              if (std::remove(outFile3.c_str()) != 0) {
                  cerr << "\nERROR: Cannot delete the additional interaction-only test output file.\n\n";
              }
              exit(1);
          }
          results3.close();
        
          if (std::remove(outFile3.c_str()) != 0) {
              cerr << "\nERROR: Cannot delete the additional interaction-only test output file.\n\n";
              exit(1);
          }
        }
    }
    catch( const CLI::CallForHelp &e )
    {
        print_help();
        exit(1);
    }
}





void print_help() {
    cout << "General Options: " << endl
        << "   --help \t\t Prints available options and exits." << endl;
    cout << endl << endl;

    cout << "Input File Options: " << endl
        << "   --input-files \t Output files from GEM 'meta' or 'full' option." << endl
        << "   --input-file-list \t A no header text file containing a single file name per line." << endl
        << "   --exposure-names \t The names of the exposure(s) to be included in the meta-analysis." << endl
        << "   --out \t\t Full path and extension to where METAGEM output results. \n \t\t\t    Default: metagem.out" << endl
        << "   --meta-option \t Integer value indicating which summary statistics should be used for meta-analysis. \n\t\t\t    0: Both model-based and robust summary statistics. \n \t\t\t    1: model-based summary statistics. \n \t\t\t    2: robust summary statistics. \n \t\t\t    Default: 0" << endl
        << "   --additional-joint \t The variable names and the full path of the output file for one additional joint test." << endl;
        << "   --additional-interaction \t The variable names and the full path of the output file for one additional interation-only test." << endl;
    cout << endl << endl;
    cout << endl << endl;
}
