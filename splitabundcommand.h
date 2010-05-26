#ifndef SPLITABUNDCOMMAND_H
#define SPLITABUNDCOMMAND_H

/*
 *  splitabundcommand.h
 *  Mothur
 *
 *  Created by westcott on 5/17/10.
 *  Copyright 2010 Schloss Lab. All rights reserved.
 *
 */


/* split.abund - given a list or name file and a number (cutoff), make two files - *rare* and *abund* 
- where rare has data for otus that have fewer sequences than the cutoff and abund has data for otus 
that have as many or more sequences as the cutoff. 
also allow an option where a user can give a group file with the list or names file and split the group file into rare and abund. */


#include "command.hpp"
#include "groupmap.h"
#include "inputdata.h"
#include "listvector.hpp"

/***************************************************************************************/

class SplitAbundCommand : public Command {
	
public:
	SplitAbundCommand(string);	
	~SplitAbundCommand();
	int execute();	
	void help();

	
private:
	int split(ListVector*);
	int split(); //namefile
	
	vector<string> outputNames;
	ListVector* list;
	GroupMap* groupMap;
	InputData* input;
	
	string outputDir, listfile, namefile, groupfile, label;
	set<string> labels;
	bool abort, allLines, accnos, wroteRareList, wroteAbundList;
	int cutoff;
	
	

};

/***************************************************************************************/

#endif

