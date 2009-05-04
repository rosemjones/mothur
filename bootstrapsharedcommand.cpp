/*
 *  bootstrapsharedcommand.cpp
 *  Mothur
 *
 *  Created by Sarah Westcott on 4/16/09.
 *  Copyright 2009 Schloss Lab UMASS Amherst. All rights reserved.
 *
 */

#include "bootstrapsharedcommand.h"
#include "sharedjabund.h"
#include "sharedsorabund.h"
#include "sharedjclass.h"
#include "sharedsorclass.h"
#include "sharedjest.h"
#include "sharedsorest.h"
#include "sharedthetayc.h"
#include "sharedthetan.h"
#include "sharedmorisitahorn.h"
#include "sharedbraycurtis.h"


//**********************************************************************************************************************

BootSharedCommand::BootSharedCommand(){
	try {
		globaldata = GlobalData::getInstance();
		format = globaldata->getFormat();
		convert(globaldata->getIters(), iters);
		validCalculator = new ValidCalculators();
		util = new SharedUtil();
		
		
		int i;
		for (i=0; i<globaldata->Estimators.size(); i++) {
			if (validCalculator->isValidCalculator("boot", globaldata->Estimators[i]) == true) { 
				if (globaldata->Estimators[i] == "jabund") { 	
					treeCalculators.push_back(new JAbund());
				}else if (globaldata->Estimators[i] == "sorabund") { 
					treeCalculators.push_back(new SorAbund());
				}else if (globaldata->Estimators[i] == "jclass") { 
					treeCalculators.push_back(new Jclass());
				}else if (globaldata->Estimators[i] == "sorclass") { 
					treeCalculators.push_back(new SorClass());
				}else if (globaldata->Estimators[i] == "jest") { 
					treeCalculators.push_back(new Jest());
				}else if (globaldata->Estimators[i] == "sorest") { 
					treeCalculators.push_back(new SorEst());
				}else if (globaldata->Estimators[i] == "thetayc") { 
					treeCalculators.push_back(new ThetaYC());
				}else if (globaldata->Estimators[i] == "thetan") { 
					treeCalculators.push_back(new ThetaN());
				}else if (globaldata->Estimators[i] == "morisitahorn") { 
					treeCalculators.push_back(new MorHorn());
				}else if (globaldata->Estimators[i] == "braycurtis") { 
					treeCalculators.push_back(new BrayCurtis());
				}
			}
		}
		
		ofstream* temp;
		for (int i=0; i < treeCalculators.size(); i++) {
			temp = new ofstream;
			out.push_back(temp);
		}
		
		//reset calc for next command
		globaldata->setCalc("");

	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the BootSharedCommand class Function BootSharedCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the BootSharedCommand class function BootSharedCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
}
//**********************************************************************************************************************

BootSharedCommand::~BootSharedCommand(){
	delete input;
	delete read;
	delete util;
}

//**********************************************************************************************************************

int BootSharedCommand::execute(){
	try {
		int count = 1;	
		EstOutput data;
		vector<SharedRAbundVector*> subset;
	
		//if the users entered no valid calculators don't execute command
		if (treeCalculators.size() == 0) { return 0; }

		if (format == "sharedfile") {
			read = new ReadOTUFile(globaldata->inputFileName);	
			read->read(&*globaldata); 
			
			input = globaldata->ginput;
			order = input->getSharedOrderVector();
		}else {
			//you are using a list and a groupfile
			read = new ReadOTUFile(globaldata->inputFileName);	
			read->read(&*globaldata); 
		
			input = globaldata->ginput;
			SharedList = globaldata->gSharedList;
			order = SharedList->getSharedOrderVector();
		}
		
		//set users groups
		util->setGroups(globaldata->Groups, globaldata->gGroupmap->namesOfGroups, "treegroup");
		numGroups = globaldata->Groups.size();
		
		//clear globaldatas old tree names if any
		globaldata->Treenames.clear();
		
		//fills globaldatas tree names
		globaldata->Treenames = globaldata->Groups;
		
		//create treemap class from groupmap for tree class to use
		tmap = new TreeMap();
		tmap->makeSim(globaldata->gGroupmap);
		globaldata->gTreemap = tmap;
			
		while(order != NULL){
		
			if(globaldata->allLines == 1 || globaldata->lines.count(count) == 1 || globaldata->labels.count(order->getLabel()) == 1){			
				
				cout << order->getLabel() << '\t' << count << endl;
				
				//open an ostream for each calc to print to
				for (int z = 0; z < treeCalculators.size(); z++) {
					//create a new filename
					outputFile = getRootName(globaldata->inputFileName) + treeCalculators[z]->getName() + ".boot" + order->getLabel() + ".tre";
					openOutputFile(outputFile, *(out[z]));
				}
				
				//create a file for each calculator with the 1000 trees in it.
				for (int p = 0; p < iters; p++) {
					
					util->getSharedVectorswithReplacement(globaldata->Groups, lookup, order);  //fills group vectors from order vector.
				
					//for each calculator												
					for(int i = 0 ; i < treeCalculators.size(); i++) {
					
						//initialize simMatrix
						simMatrix.clear();
						simMatrix.resize(numGroups);
						for (int m = 0; m < simMatrix.size(); m++)	{
							for (int j = 0; j < simMatrix.size(); j++)	{
								simMatrix[m].push_back(0.0);
							}
						}
				
						//initialize index
						index.clear();
						for (int g = 0; g < numGroups; g++) {	index[g] = g;	}
							
						for (int k = 0; k < lookup.size(); k++) { // pass cdd each set of groups to commpare
							for (int l = k; l < lookup.size(); l++) {
								if (k != l) { //we dont need to similiarity of a groups to itself
									subset.clear(); //clear out old pair of sharedrabunds
									//add new pair of sharedrabunds
									subset.push_back(lookup[k]); subset.push_back(lookup[l]); 
									
									//get estimated similarity between 2 groups
									data = treeCalculators[i]->getValues(subset); //saves the calculator outputs
									//save values in similarity matrix
									simMatrix[k][l] = data[0];
									simMatrix[l][k] = data[0];
								}
							}
						}
				
						//creates tree from similarity matrix and write out file
						createTree(out[i]);
					}
				}
				//close ostream for each calc
				for (int z = 0; z < treeCalculators.size(); z++) { out[z]->close(); }
			}
		
			//get next line to process
			if (format == "sharedfile") {
				order = input->getSharedOrderVector();
			}else {
				//you are using a list and a groupfile
				SharedList = input->getSharedListVector(); //get new list vector to process
				if (SharedList != NULL) {
					order = SharedList->getSharedOrderVector(); //gets new order vector with group info.
				}else {
					break;
				}
			}
			count++;
		}
		
		//reset groups parameter
		globaldata->Groups.clear();  globaldata->setGroups("");

		return 0;
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the BootSharedCommand class Function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the BootSharedCommand class function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}		
}
//**********************************************************************************************************************

void BootSharedCommand::createTree(ostream* out){
	try {
		//create tree
		t = new Tree();
		
		//do merges and create tree structure by setting parents and children
		//there are numGroups - 1 merges to do
		for (int i = 0; i < (numGroups - 1); i++) {
		
			float largest = -1.0;
			int row, column;
			//find largest value in sims matrix by searching lower triangle
			for (int j = 1; j < simMatrix.size(); j++) {
				for (int k = 0; k < j; k++) {
					if (simMatrix[j][k] > largest) {  largest = simMatrix[j][k]; row = j; column = k;  }
				}
			}

			//set non-leaf node info and update leaves to know their parents
			//non-leaf
			t->tree[numGroups + i].setChildren(index[row], index[column]);
		
			//parents
			t->tree[index[row]].setParent(numGroups + i);
			t->tree[index[column]].setParent(numGroups + i);
			
			//blength = distance / 2;
			float blength = ((1.0 - largest) / 2);
			
			//branchlengths
			t->tree[index[row]].setBranchLength(blength - t->tree[index[row]].getLengthToLeaves());
			t->tree[index[column]].setBranchLength(blength - t->tree[index[column]].getLengthToLeaves());
		
			//set your length to leaves to your childs length plus branchlength
			t->tree[numGroups + i].setLengthToLeaves(t->tree[index[row]].getLengthToLeaves() + t->tree[index[row]].getBranchLength());
			
		
			//update index 
			index[row] = numGroups+i;
			index[column] = numGroups+i;
			
			//zero out highest value that caused the merge.
			simMatrix[row][column] = -1.0;
			simMatrix[column][row] = -1.0;
		
			//merge values in simsMatrix
			for (int n = 0; n < simMatrix.size(); n++)	{
				//row becomes merge of 2 groups
				simMatrix[row][n] = (simMatrix[row][n] + simMatrix[column][n]) / 2;
				simMatrix[n][row] = simMatrix[row][n];
				//delete column
				simMatrix[column][n] = -1.0;
				simMatrix[n][column] = -1.0;
			}
		}

		//assemble tree
		t->assembleTree();
	
		//print newick file
		t->print(*out);
	
		//delete tree
		delete t;
	
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the BootSharedCommand class Function createTree. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the BootSharedCommand class function createTree. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
}
/***********************************************************/
void BootSharedCommand::printSims() {
	try {
		cout << "simsMatrix" << endl;
		for (int m = 0; m < simMatrix.size(); m++)	{
			for (int n = 0; n < simMatrix.size(); n++)	{
				cout << simMatrix[m][n] << '\t'; 
			}
			cout << endl;
		}

	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the BootSharedCommand class Function printSims. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the BootSharedCommand class function printSims. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}		
}
/***********************************************************/

