/*
 *  rarefactsharedcommand.cpp
 *  Dotur
 *
 *  Created by Sarah Westcott on 1/6/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */

#include "rarefactsharedcommand.h"
#include "sharedsobs.h"
#include "sharednseqs.h"

//**********************************************************************************************************************

RareFactSharedCommand::RareFactSharedCommand(string option){
	try {
		globaldata = GlobalData::getInstance();
		abort = false;
		allLines = 1;
		lines.clear();
		labels.clear();
		Estimators.clear();
		
		//allow user to run help
		if(option == "help") { validCalculator = new ValidCalculators(); help(); abort = true; }
		
		else {
			//valid paramters for this command
			string Array[] =  {"iters","line","label","calc","groups"};
			vector<string> myArray (Array, Array+(sizeof(Array)/sizeof(string)));
			
			OptionParser parser(option);
			map<string,string> parameters = parser.getParameters();
			
			ValidParameters validParameter;
			
			//check to make sure all parameters are valid for command
			for (map<string,string>::iterator it = parameters.begin(); it != parameters.end(); it++) { 
				if (validParameter.isValidParameter(it->first, myArray, it->second) != true) {  abort = true;  }
			}
			
			//make sure the user has already run the read.otu command
			if (globaldata->getSharedFile() == "") {
				if (globaldata->getListFile() == "") { cout << "You must read a list and a group, or a shared before you can use the collect.shared command." << endl; abort = true; }
				else if (globaldata->getGroupFile() == "") { cout << "You must read a list and a group, or a shared before you can use the collect.shared command." << endl; abort = true; }
			}

			
			//check for optional parameter and set defaults
			// ...at some point should added some additional type checking...
			line = validParameter.validFile(parameters, "line", false);				
			if (line == "not found") { line = "";  }
			else { 
				if(line != "all") {  splitAtDash(line, lines);  allLines = 0;  }
				else { allLines = 1;  }
			}
			
			label = validParameter.validFile(parameters, "label", false);			
			if (label == "not found") { label = ""; }
			else { 
				if(label != "all") {  splitAtDash(label, labels);  allLines = 0;  }
				else { allLines = 1;  }
			}
			
			//make sure user did not use both the line and label parameters
			if ((line != "") && (label != "")) { cout << "You cannot use both the line and label parameters at the same time. " << endl; abort = true; }
			//if the user has not specified any line or labels use the ones from read.otu
			else if((line == "") && (label == "")) {  
				allLines = globaldata->allLines; 
				labels = globaldata->labels; 
				lines = globaldata->lines;
			}
				
			calc = validParameter.validFile(parameters, "calc", false);			
			if (calc == "not found") { calc = "sharedobserved";  }
			else { 
				 if (calc == "default")  {  calc = "sharedobserved";  }
			}
			splitAtDash(calc, Estimators);
			
			groups = validParameter.validFile(parameters, "groups", false);			
			if (groups == "not found") { groups = ""; }
			else { 
				splitAtDash(groups, Groups);
				globaldata->Groups = Groups;
			}
			
			string temp;
			temp = validParameter.validFile(parameters, "iters", false);			if (temp == "not found") { temp = "1000"; }
			convert(temp, nIters); 
			
			if (abort == false) {
			
				string fileNameRoot = getRootName(globaldata->inputFileName);
//				format = globaldata->getFormat();

				
				validCalculator = new ValidCalculators();
				
				for (int i=0; i<Estimators.size(); i++) {
					if (validCalculator->isValidCalculator("sharedrarefaction", Estimators[i]) == true) { 
						if (Estimators[i] == "sharedobserved") { 
							rDisplays.push_back(new RareDisplay(new SharedSobs(), new SharedThreeColumnFile(fileNameRoot+"shared.rarefaction", "")));
						}else if (Estimators[i] == "sharednseqs") { 
							rDisplays.push_back(new RareDisplay(new SharedNSeqs(), new SharedThreeColumnFile(fileNameRoot+"shared.r_nseqs", "")));
						}
					}
				}
			}
				
		}

	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the RareFactSharedCommand class Function RareFactSharedCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the RareFactSharedCommand class function RareFactSharedCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
			
}

//**********************************************************************************************************************

void RareFactSharedCommand::help(){
	try {
		cout << "The rarefaction.shared command can only be executed after a successful read.otu command." << "\n";
		cout << "The rarefaction.shared command parameters are label, line, iters, groups and calc.  No parameters are required, but you may not use " << "\n";
		cout << "both the line and label parameters at the same time. The rarefaction command should be in the following format: " << "\n";
		cout << "rarefaction.shared(label=yourLabel, line=yourLines, iters=yourIters, calc=yourEstimators, groups=yourGroups)." << "\n";
		cout << "Example rarefaction.shared(label=unique-.01-.03, line=0-5-10, iters=10000, groups=B-C, calc=sharedobserved)." << "\n";
		cout << "The default values for iters is 1000, freq is 100, and calc is sharedobserved which calculates the shared rarefaction curve for the observed richness." << "\n";
		cout << "The default value for groups is all the groups in your groupfile." << "\n";
		validCalculator->printCalc("sharedrarefaction", cout);
		cout << "The label and line parameters are used to analyze specific lines in your input." << "\n";
		cout << "The groups parameter allows you to specify which of the groups in your groupfile you would like analyzed.  You must enter at least 2 valid groups." << "\n";
		cout << "Note: No spaces between parameter labels (i.e. freq), '=' and parameters (i.e.yourFreq)." << "\n" << "\n";
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the RareFactSharedCommand class Function help. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the RareFactSharedCommand class function help. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
}

//**********************************************************************************************************************

RareFactSharedCommand::~RareFactSharedCommand(){
	delete input;
	delete rCurve;
	delete read;
	delete validCalculator;
}

//**********************************************************************************************************************

int RareFactSharedCommand::execute(){
	try {
	
		if (abort == true) { return 0; }
		
		int count = 1;
		
		//if the users entered no valid calculators don't execute command
		if (rDisplays.size() == 0) { return 0; }

		read = new ReadOTUFile(globaldata->inputFileName);	
		read->read(&*globaldata); 
			
		input = globaldata->ginput;
		lookup = input->getSharedRAbundVectors();
		vector<SharedRAbundVector*> lastLookup = lookup;
		
		if (lookup.size() < 2) { 
			cout << "I cannot run the command without at least 2 valid groups."; 
			for (int i = 0; i < lookup.size(); i++) { delete lookup[i]; }
			return 0;
		}
		
		//if the users enters label "0.06" and there is no "0.06" in their file use the next lowest label.
		set<string> processedLabels;
		set<string> userLabels = labels;
		set<int> userLines = lines;
	
		//as long as you are not at the end of the file or done wih the lines you want
		while((lookup[0] != NULL) && ((allLines == 1) || (userLabels.size() != 0) || (userLines.size() != 0))) {
			
			if(allLines == 1 || lines.count(count) == 1 || labels.count(lookup[0]->getLabel()) == 1){
				
				rCurve = new Rarefact(lookup, rDisplays);
				rCurve->getSharedCurve(freq, nIters);
				delete rCurve;
			
				cout << lookup[0]->getLabel() << '\t' << count << endl;
				processedLabels.insert(lookup[0]->getLabel());
				userLabels.erase(lookup[0]->getLabel());
				userLines.erase(count);
			}
			
			if ((anyLabelsToProcess(lookup[0]->getLabel(), userLabels, "") == true) && (processedLabels.count(lastLookup[0]->getLabel()) != 1)) {
					cout << lastLookup[0]->getLabel() << '\t' << count << endl;
					rCurve = new Rarefact(lastLookup, rDisplays);
					rCurve->getSharedCurve(freq, nIters);
					delete rCurve;

					processedLabels.insert(lastLookup[0]->getLabel());
					userLabels.erase(lastLookup[0]->getLabel());
			}
				
			//prevent memory leak
			if (count != 1) { for (int i = 0; i < lastLookup.size(); i++) {  delete lastLookup[i];  } }
			lastLookup = lookup;
			
			//get next line to process
			lookup = input->getSharedRAbundVectors();
			count++;
		}
		
		//output error messages about any remaining user labels
		set<string>::iterator it;
		bool needToRun = false;
		for (it = userLabels.begin(); it != userLabels.end(); it++) {  
			cout << "Your file does not include the label "<< *it; 
			if (processedLabels.count(lastLookup[0]->getLabel()) != 1) {
				cout << ". I will use " << lastLookup[0]->getLabel() << "." << endl;
				needToRun = true;
			}else {
				cout << ". Please refer to " << lastLookup[0]->getLabel() << "." << endl;
			}
		}
		
		//run last line if you need to
		if (needToRun == true)  {
			cout << lastLookup[0]->getLabel() << '\t' << count << endl;
			rCurve = new Rarefact(lastLookup, rDisplays);
			rCurve->getSharedCurve(freq, nIters);
			delete rCurve;
		}
		
		for (int i = 0; i < lastLookup.size(); i++) {  delete lastLookup[i];  }

		for(int i=0;i<rDisplays.size();i++){	delete rDisplays[i];	}	
		
		//reset groups parameter
		globaldata->Groups.clear();  

		return 0;
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the RareFactSharedCommand class Function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the RareFactSharedCommand class function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
}


//**********************************************************************************************************************
