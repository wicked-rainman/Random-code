// ********************************************************************************
// Program : train_check                                                          *
// Purpose :                                                                      *
// Checks to see if the specified train is really a train or not. Definition of a *
// train is:                                                                      *
//                                                                                *
// 1. At the front of the train there may be one or more engines. Each engine     *
// may be towing a single tender wagon.                                           *
// 2. The middle of a train contains one or more carriages where each carriage    *
// is designed to carry freight or passengers.                                    *
// 3. The end of the train can be a single guards van, or one or more engines.    *
// As these engines are being used as "pushers", they have to be diesel because   *
// all steam trains have cow catchers on the front which means they can't push.   *
// Because of this, no tenders are allowed in the train end section.              *
//                                                                                *
// Parameters: A single train -                                                   *
//      E=engine                                                                  *
//      T=tender                                                                  *
//      V=guards van                                                              *
//      P=passenger carridge                                                      *
//      G=goods wagon                                                             *
//                                                                                *
// ********************************************************************************
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define GUARDS_VAN 'V' 
#define ENGINE 'E' 
#define TENDER 'T' 
#define PASSENGER_CARRIDGE 'P' 
#define GOODS_WAGON 'G' 
static int valid_train_puller(char *, int);
static int valid_train_load(char *, int, int);
static bool valid_train_end(char *,int,int);

// **************************
//
// Main routine
//
// **************************

int main(int argc, char **argv) {
	char train[150];		// No trains longer than 150 stock units
	int train_length;		// Length of the train
	int stock_position=0;		// Which stock unit is currently being checked
	if(argc !=2) {			// One argument to the program - the train definition
		printf("\n Specify a train when running this program:\n\n");
		printf(" V=guards van\n");
		printf(" T=tender\n");
		printf(" E=engine\n");
		printf(" P=passenger carriage\n");
		printf(" G=goods wagon\n");
		printf("\n");
		printf(" EG: ./train_check ETETEEGGGPPPV\n\n");
		return 1;
	}
	train_length= (int) strlen(argv[1]);	//Store the length of the train
	if(train_length>149) {			//Is the train too long ?
		printf("Get lost, that train is too long!\n");
		return 1;
	}
	strcpy(train,argv[1]);   //Save the train to local storage
	if( (bool) (stock_position=valid_train_puller(train,train_length))) {    	//Is the front of the train correct ?
		if((bool) (stock_position=valid_train_load(train,train_length, stock_position))) {  //Is load correct?
			if (valid_train_end(train,train_length,stock_position)) { 	//Is the train end correct ?
				printf("Good train\n");	
			}
			else {
				printf("Train end is wrong :%s\n",train);
			}
		}
		else {
			printf("Train load looks wrong : %s\n",train);
		}
	}
	else {
		printf("Train puller looks wrong: %s\n",train);
	}
	return 1;
}

// **********************************
//
// Function to check a train puller
//
// **********************************
int valid_train_puller(char *train, int train_length) {
	char stock;
	bool good_train=false;
	bool engine=false;
	int stock_position=0;
	stock=toupper(train[stock_position]);
	while((stock_position<train_length) && (stock==ENGINE || stock==TENDER)) {
		if(stock==ENGINE) {
			engine=true;
			good_train=true;
		}
		if(stock==TENDER) {
			if(engine==false) {
                                good_train=false;
				break;
                	}
			engine=false;
		}
		stock=toupper(train[++stock_position]);
	}
	return good_train ? stock_position : 0;
}

// *********************************
//
// Function to check a train payload
//
// *********************************
int valid_train_load(char *train, int train_length, int stock_position) {
	char stock;
	bool train_payload=false;
	stock=toupper(train[stock_position]);
	while((stock_position<train_length) && (stock==PASSENGER_CARRIDGE) || (stock==GOODS_WAGON)) {
		train_payload=true;
		stock=toupper(train[++stock_position]);
	}
	return train_payload ? stock_position : 0;
}

// ************************************
//
// Function to check the end of a train
//
// *************************************
bool valid_train_end(char *train, int train_length, int stock_position) {
	bool guards_van=false,engine=false,good_train=true;
	while(stock_position<train_length) {
		switch (toupper(train[stock_position++])) {
			case GUARDS_VAN : {
				if(engine) good_train=false;
				if(guards_van) good_train=false;
				guards_van=true;
				break;
			}
			case ENGINE : {
				if(guards_van) good_train=false;
				engine=true;
				break;
			}
			default : {
				good_train=false;
			}
		}
	}
	return good_train ? true : false;
}
