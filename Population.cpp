/*
 * Population.cpp
 *
 *  Created on: Apr 6, 2009
 *      Author: David Briemann
 */

#include <cstdlib>
#include <iostream>
#include <set>
#include <algorithm>
#include <omp.h>

#include "Population.hpp"
#include "Protein.hpp"

Population::Population(int size, Protein prot, float mutProb, float crossProb) {
	this->parent1 = NULL;
	this->parent2 = NULL;
	this->mutProb = mutProb;
	this->crossProb = crossProb;
	this->protein = prot;
	this->size = size;

	//init population
	this->individuals = new Conformation[this->size];
	Conformation temp;

	//COMM
	std::cout << "Generate Population: " << '\n';

	// double start, finish;
	// start = omp_get_wtime();
	#pragma omp declare reduction(U:std::set<std::string>:\
		omp_out.insert(omp_in.begin(), omp_in.end())\
	)

	#pragma omp parallel for default(none)  \
			reduction(U:setOfConformations) \
		    private(temp, collisionSet)     \
		    shared (size, protein, individuals, std::cout)
	for (int i = 0; i < size; i++) {
		do {
			//create temp conformation
			temp = Conformation(&(protein), &(collisionSet));
			temp.calcFitness();

			//check if fitness is not 0 (and conformation is not already present in population)
			/*&& this->setOfConformations.insert(temp.getConformationString()).second*/
		} while (temp.getFitness() == 0);

		//add to individuals
		individuals[i] = temp;
		setOfConformations.insert(temp.getConformationString());

		//COMM
		// cout << flush;
		// cout << i << ".";
	}
	// finish = omp_get_wtime();
	// std::cout << "Elapsed: " << finish - start << '\n';
	
	this->theFittest = &(this->individuals[0]);
	this->setFittest();
	std::cout << '\n';
}

Population::~Population() {

	this->theFittest = NULL;

	this->parent1 = NULL;

	this->parent1 = NULL;

	//free some space
	delete[] this->individuals;
	this->individuals = NULL;
}

/*
 * checks if a conformation is not yet present in the individuals pool
 * if so returns true
 */
bool Population::isInsertable(const Conformation &candidate) {
	if ((this->setOfConformations.insert(candidate.getConformationString())).second) {
		return true;
	} else {
		return false;
	}
}

/*
 * determines and sets the fittest individual
 */
void Population::setFittest() {
	// double start, finish;
	// start = omp_get_wtime();
	#pragma omp declare reduction(min:Conformation*:\
		omp_out = omp_in->getFitness() < omp_out->getFitness() ? omp_in : omp_out\
	) initializer(omp_priv = omp_orig)

	#pragma omp parallel for default(none)\
			reduction(min:theFittest)\
			shared(size, individuals)
	for (int i = 0; i < size; i++) {
		if (individuals[i].getFitness() < theFittest->getFitness()) {
			theFittest = &(individuals[i]);
		}
	}
	// finish = omp_get_wtime();
	// std::cout << "Elapsed: " << finish - start << '\n';
}

/*
 * returns a pointer to the fittest individual
 */
Conformation * Population::getFittest() const {
	return this->theFittest;
}

/*
 * dumps all conformations to console
 */
void Population::dumpAll() const {
	for (int i = 0; i < this->size; i++) {
		std::cout << i << ": " + this->individuals[i].getStatusString() << '\n';
	}
}

/*
 * spins the wheel and selects upon probability, which is determined by fitness
 * returns a pointer to a Conformation
 */
Conformation * Population::rouletteWheelSelect() {
	int totalFitness = 0;
	float randF = 0.0;
	float currentProb = 0.0;
	int i = 0;

	//calculate total fitness
	# pragma omp parallel for reduction(+:totalFitness) default(none) shared(size, individuals)
	for (i = 0; i < size; i++) {
		totalFitness += individuals[i].getFitness();
	}

	randF = Conformation::randomFloat();
	i = 0;

	currentProb = static_cast<float> (this->individuals[i].getFitness()) / static_cast<float> (totalFitness);

	//be aware of rounding failures? float!
	while ( (currentProb < randF) && (i < (this->size - 1))) {
		currentProb += static_cast<float> (this->individuals[i].getFitness()) / static_cast<float> (totalFitness);
		i++;
	}

	return &(this->individuals[i]);
}

void Population::crossover() {
	float randF = 0.0;

	//find parents
	this->parent1 = this->rouletteWheelSelect(); // 1 loop
	randF = Conformation::randomFloat();
	if( this->crossProb < randF ) {
		return; //crossover rate
	}

	this->parent2 = this->rouletteWheelSelect(); // 1 loop
	randF = Conformation::randomFloat();
	if( this->crossProb < randF ) {
		return; //crossover rate
	}

	//recombinate parents to children
	this->child1 = Conformation(*(this->parent1), *(this->parent2), &(this->collisionSet)); // 2 loops (modificar para construtor de movimento)
	this->child2 = Conformation(*(this->parent2), *(this->parent1), &(this->collisionSet)); // 2 loops (modificar para construtor de movimento)

	//mutate child1
	this->child1.mutate(this->mutProb); // 1 loop
	this->child1.calcValidity();

	if (this->child1.isValid()) { //if child is valid
		//update fitness of child1
		this->child1.calcFitness(); // 1 loop

		//if conformation of child1 isnt present yet
		if (this->isInsertable(this->child1)) {
			//replace parent1 if child1 is fitter
			if (this->child1.getFitness() < this->parent1->getFitness()) {
				*(this->parent1) = this->child1;
			} else if (this->child1.getFitness() < this->parent2->getFitness()) { //replace parent2 if child1 is fitter
				*(this->parent2) = this->child1;
			}
		}
	} //else discard

	//mutate child2
	this->child2.mutate(this->mutProb);
	this->child2.calcValidity();

	if (this->child2.isValid()) { //if child 2 is valid
		//update fitness of child2
		this->child2.calcFitness();

		//if conformation of child2 isnt present yet
		if (this->isInsertable(this->child2)) {
			//replace parent2 if child2 is fitter
			if (this->child2.getFitness() < this->parent2->getFitness()) {
				*(this->parent2) = this->child2;
			} else if (this->child2.getFitness() < this->parent1->getFitness()) { //replace parent1 if child2 is fitter
				*(this->parent1) = this->child2;
			}
		}
	} //else discard


	if (this->parent1->getFitness() < this->theFittest->getFitness()) {
		this->theFittest = this->parent1;
		//call visualisation function
	/*	this->theFittest->printStatus();
		this->theFittest->printAsciiPicture();*/
	}

	if (this->parent2->getFitness() < this->theFittest->getFitness()) {
		this->theFittest = this->parent2;
		//call visualisation function
	/*	this->theFittest->printStatus();
		this->theFittest->printAsciiPicture();*/
	}
}