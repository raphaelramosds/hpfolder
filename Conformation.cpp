/*
 * Conformation.cpp
 *
 *  Created on: Apr 6, 2009
 *      Author: David Briemann
 */

#include <set>
#include <cstdlib>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <omp.h>
using namespace std;

#include "Conformation.hpp"
#include "Protein.hpp"


//init static counter to zero
//probably not needed
unsigned int Conformation::energyEvalSteps = 0;
thread_local unsigned int Conformation::seed = 0;


//default constructor
Conformation::Conformation() {
	this->setOfPoints = NULL;
	this->encoding = NULL;
	this->absPositions = NULL;
	this->length = 0;
	this->protein = NULL;
	this->fitness = 0;
	this->generation = 0;
	this->validState = false;
}
//constructor
Conformation::Conformation( Protein *prot, set<int> *sop ) {
	this->protein = prot;
	this->length = this->protein->getLength();
	this->encoding = new char[this->length - 2];
	this->absPositions = new int[this->length];
	this->generation = 0;
	this->fitness = 0;
	this->validState = false;
	this->setOfPoints = sop;

	this->generateRandomConformation(true);
}

//recombination constructor!
Conformation::Conformation(const Conformation &p1, const Conformation &p2, set<int> *sop) {
	this->setOfPoints = sop;
	this->protein = p1.protein;
	this->length = p1.length;
	this->generation = (p1.generation + p2.generation) / 2 + 1; //calc next generation

	//create random number for encoding.. range [2..len-1]
	int randI = (rand_r(&seed) % (this->protein->getLength() - 2));

	this->encoding = new char[this->length - 2];
	this->absPositions = new int[this->length];

	//set crossed encoding
	//first part
    char* enc = encoding, * p1enc = p1.encoding, * p2enc = p2.encoding;
	# pragma omp parallel for simd schedule(simd:static) default(none) shared(randI, p1enc, p2enc, enc)
	for(int i = 0; i < length - 2; i++) {
		enc[i] = i < randI ? p1enc[i] : p2enc[i];
	}

	if( this->setOfPoints != NULL ) {
		this->calcValidity();
	}
}

//destructor
Conformation::~Conformation() {
	delete [] this->encoding; //free memory
	this->encoding = NULL; //point to null

	delete [] this->absPositions; //free memory
	this->absPositions = NULL; //point to null

	this->protein = NULL;
}

//overload = operator for assignment
const Conformation & Conformation::operator=(const Conformation &right) {
	if( *this != right ) { //avoid self assignment
		this->protein = right.protein;
		this->fitness = right.fitness;
		this->length = right.length;
		this->validState = right.validState;
		this->generation = right.generation;
		this->setOfPoints = right.setOfPoints;

		//free memory
		delete [] this->encoding;
		this->encoding = NULL;
		delete [] this->absPositions;
		this->absPositions = NULL;

		//allocate new memory with given sizes
		this->encoding = new char[this->length-2];
		this->absPositions = new int[this->length];

		//copy absolute positions
		#pragma omp parallel default(none) shared(length, right, absPositions, encoding)
        {
            #pragma omp for simd schedule(simd:static) nowait
            for( int i = 0; i < length; i++) {
                absPositions[i] = right.absPositions[i];
            }
            //copy encoding
            char* rightenc = right.encoding;
            char* enc = encoding;
            #pragma omp for simd schedule(simd:static) nowait
            for( int i = 0; i < length - 2; i++) {
                enc[i] = rightenc[i];
            }
        }
		if( this->setOfPoints != NULL ) {
			this->setOfPoints->clear();
		}
	}
	return *this;
}

//overload == operator for comparison
bool Conformation::operator==(const Conformation &right) const {
	//compare length
	if(length != right.length) {
		return false;
	}
	//compare encoding
	for( int i = 0; i < length - 2; i++ ) {
		if(encoding[i] != right.encoding[i]) {
			return false;
		}
	}
	return true;
}

//overload != operator for comparison
//invert result of ==
bool Conformation::operator!=(const Conformation &right) const {
	return !(*this == right);
}

Protein * Conformation::getProtein() const {
	return this->protein;
}

bool Conformation::isValid() const {
	return this->validState;
}

int Conformation::getLength() const {
	return this->length;
}

void Conformation::calcFitness() {
	Conformation::energyEvalSteps++;
	int temp_fitness = 0;
	short oriX;
	short oriY;
	short destX;
	short destY;
	int distX;
	int distY;

	// for each amino acid except the last.. do:
	#pragma omp parallel for default(none)\
			reduction(+:temp_fitness)\
			shared(length, protein, absPositions)\
			private(oriX, oriY, destX, destY, distX, distY)
	for (int i = 0; i < length; i++) {

		// check if it is hydrophobic ("black")
		if (protein->getNth(i) == 'B') {
			// get coordinates of current origin
			oriX = Conformation::extractX(absPositions[i]);
			oriY = Conformation::extractY(absPositions[i]);

			// for every remaining amino acid
			// skipping the successor too
			for (int j = i + 2; j < length; j++) {
				// if amino acid is black=hydrophob
				if (protein->getNth(j) == 'B') {
					// get coordinates of current destination
					destX = Conformation::extractX(absPositions[j]);
					destY = Conformation::extractY(absPositions[j]);

					// check if destination is neighbor to origin
					// means distance is 1 in lattice
					distX = abs(static_cast<int>((oriX - destX)));
					distY = abs(static_cast<int>((oriY - destY)));
					if (((distX == 1) && (distY == 0))
							|| ((distX == 0) && (distY == 1))) {
						temp_fitness--;
					}
				}
			}
		}
	}

	this->fitness = temp_fitness;
}

int Conformation::getFitness() const {
	return this->fitness;
}

string Conformation::getConformationString() const {
	string result = "";
	for(int i=0; i<this->length-2; i++) {
		if(this->encoding[i] == FORWARD) {
			result += "F";
		} else if(this->encoding[i] == LEFT) {
			result += "L";
		} else if(this->encoding[i] == RIGHT) {
			result += "R";
		} else {
			result+="?"; //shouldnt happen
		}
	}
	return result;
}

void Conformation::generateRandomConformation( bool valid ) {
	char randInt;

	//init random conformation
	for(int i = 0; i < this->length-2; i++) {
		randInt = rand_r(&seed) % 3 - 1; //random numer -1, 0, 1
		this->encoding[i] = static_cast<char>(randInt);
	}

	if( valid ) {
		do {
			this->mutate(0.1); //mutate with high probability (10%)
			this->calcValidity(); //check if conformation is valid
		} while( !this->validState );
	}
}

void Conformation::calcValidity() {
	this->calcAbsolutePositions();
	if( this->setOfPoints != NULL ) {
		this->setOfPoints->clear();
	}
	this->validState = true;

	for(int i=0; i < this->length; i++) {
		if( !( (this->setOfPoints->insert(this->absPositions[i])).second ) ) {
			this->validState = false;
			return;
		}
	}
}

void Conformation::mutate( float probability ) {
	float randF[length - 2];
	char randC[length - 2];

    #pragma omp parallel for default(none)\
            shared(randF, randC, length)
    for (int i = 0; i < length - 2; i++) {
        randF[i] = randomFloat();
        randC[i] = static_cast<char>(rand_r(&seed) % 3 - 1);
    }

	// for each amino acid..
    char mask;
    char* enc = encoding;
	#pragma omp parallel for simd default(none)\
			shared(probability, enc, length, randF, randC)\
			private(mask)\
            schedule(simd:static)
	for (int i = 0; i < length - 2; i++) {
		// ..check mutation probability
        mask = (randF[i] <= probability) ? 0xFF : 0x00;
        enc[i] = (randC[i] & mask) | (enc[i] & ~mask);
	}
}

void Conformation::calcAbsolutePositions() {
	char lastDirection = 0;
	int pos = 0;
	short x = 0;
	short y = 0;

	//set first pos
	pos = Conformation::point(x, y);
	this->absPositions[0] = pos;
	//set second pos
	y = 1;
	pos = Conformation::point(x, y);
	this->absPositions[1] = pos;

	// find absolute position for relative encoding
	// 0=up,1=down,2=right,3=left
	for (int i = 0; i < this->length-2; i++) {
		if (lastDirection == 0) { // up
			if (this->encoding[i] == FORWARD) {
				y++;
			} else if (this->encoding[i] == RIGHT) {
				x++;
				lastDirection = 2;
			} else if (this->encoding[i] == LEFT) {
				x--;
				lastDirection = 3;
			}
		} else if (lastDirection == 1) { // down
			if (this->encoding[i] == FORWARD) {
				y--;
			} else if (this->encoding[i] == RIGHT) {
				x--;
				lastDirection = 3;
			} else if (this->encoding[i] == LEFT) {
				x++;
				lastDirection = 2;
			}
		} else if (lastDirection == 2) { // right
			if (this->encoding[i] == FORWARD) {
				x++;
			} else if (this->encoding[i] == RIGHT) {
				y--;
				lastDirection = 1;
			} else if (this->encoding[i] == LEFT) {
				y++;
				lastDirection = 0;
			}
		} else if (lastDirection == 3) { // left
			if (this->encoding[i] == FORWARD) {
				x--;
			} else if (this->encoding[i] == RIGHT) {
				y++;
				lastDirection = 0;
			} else if (this->encoding[i] == LEFT) {
				y--;
				lastDirection = 1;
			}
		}
		pos = Conformation::point(x, y);

		this->absPositions[i + 2] = pos;
	}
}

int Conformation::getGeneration() const {
	return this->generation;
}

void Conformation::olden() {
	(this->generation)++;
}

string Conformation::getStatusString() const {
	string s = /*"Conformation: " + this->getConformationString() +*/ "Fitness: ";
	stringstream ss1;
	stringstream ss2;

	ss1 << this->fitness;
	s += ss1.str() + "   Generation: ";

	ss2 << this->generation;
	s += ss2.str();

	return s;
}

void Conformation::printAsciiPicture() const {
	string result = "";
	int width = 0;
	int height = 0;
	short lowestX = 0;
	short highestX = 0;
	short lowestY = 0;
	short highestY = 0;
	short lastX = 0;
	short lastY = 0;
	short x = 0;
	short y = 0;

	//first pos is always (0,0) so start with 1
	for(int i = 1; i < this->length; i++) {
		x = Conformation::extractX(this->absPositions[i]);
		y = Conformation::extractY(this->absPositions[i]);

		//get maxima and minima
		if( x < lowestX) {
			lowestX = x;
		} else if(x > highestX) {
			highestX = x;
		}

		if(y < lowestY) {
			lowestY = y;
		} else if(y > highestY) {
			highestY = y;
		}
	}

	//get boundaries (stretch by 2)
	width = (highestX - lowestX) * 2 + 1;
	height = (highestY -lowestY) * 2 + 1;

	//allocate mem for field
	char **pField = new char*[height];
	# pragma omp parallel for default(none) shared(height, width, pField)
	for(int i = 0; i < height; i++) {
		pField[i] = new char[width];
	}

	//init with spaces
	#pragma omp parallel for
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			pField[i][j] = ' ';
		}
	}

	//normalize points
	//move all points into the positive
	//and stretch by 2
	for(int i = 0; i < this->length; i++) {
		x = Conformation::extractX(this->absPositions[i]);
		y = Conformation::extractY(this->absPositions[i]);

		pField[(y+abs(static_cast<int>(lowestY)))*2][(x+abs(static_cast<int>(lowestX)))*2]
		                                             = this->protein->getNth(i);
		if(lastX > x) {
			pField[(y+abs(static_cast<int>(lowestY)))*2][((x+abs(static_cast<int>(lowestX)))*2)+1]
					                                             = '-';
		} else if(lastX < x) {
			pField[(y+abs(static_cast<int>(lowestY)))*2][((x+abs(static_cast<int>(lowestX)))*2)-1]
					                                             = '-';
		}
		if(lastY < y) {
			pField[((y+abs(static_cast<int>(lowestY)))*2)-1][(x+abs(static_cast<int>(lowestX)))*2]
					                                             = '|';
		} else if(lastY > y) {
			pField[((y+abs(static_cast<int>(lowestY)))*2)+1][(x+abs(static_cast<int>(lowestX)))*2]
					                                             = '|';
		}

		lastX = x;
		lastY = y;
	}

	//print image..
	for(int i=0; i < height; i++) {
		for(int j=0; j < width; j++) {
			if( pField[i][j] == 'B') {
				cout << '#';
			} else if( pField[i][j] == 'W') {
				cout << 'O';
			} else {
				cout << pField[i][j];
			}
		}
		printf("\n");
	}

	//free memory
	for( int i = 0 ; i < height ; i++ ) {
		delete [] pField[i];
	}
	delete [] pField;
}

// combines two short coordinates into a single int.
int Conformation::point(short x, short y) {
	return ((static_cast<int>(x)) << 16) | static_cast<unsigned short>(y);
}

// extracts the x coordinate from a point int.
short Conformation::extractX(int point) {
	return static_cast<short>((point >> 16));
}

// extracts the y coordinate from a point int.
short Conformation::extractY(int point) {
	return static_cast<short>(point);
}

//generates a random float value in [0,1]
float Conformation::randomFloat() {
	float scale=RAND_MAX+1.;
	float base=rand_r(&seed)/scale;
	float fine=rand_r(&seed)/scale;

	return base+fine/scale;
}

int Conformation::getAbsAt(int i) const {
	return this->absPositions[i];
}

void Conformation::srand(unsigned int seed) {
	#pragma omp parallel default(none) shared(seed)
	{
		Conformation::seed = seed + omp_get_thread_num();
	}
}