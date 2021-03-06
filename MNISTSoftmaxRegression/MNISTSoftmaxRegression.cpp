// MNISTSoftmaxRegression.cpp : Defines the entry point for the console application.
//Multi-Class softmax regression + Stochastic Gradient Descent optimimsation for weights
//Why? Cause it's easy!

#include "stdafx.h"
#include <iostream>
#include <random>
#include <ctime>
#include "Eigen/Dense"
#include <fstream> //THIS #include must come AFTER 3rd party libs!

//#define DEBUG

using namespace Eigen;
using namespace std;

const int NUMBER_OF_EXAMPLES = 20000;
const int NUMBER_OF_TESTING_EXAMPLES = 1000;
const double LEARINING_RATE = 0.005;

char * read_file(const char * dir, int offset)
	{
	ifstream file;
	file.open(dir, ios::in | ios::binary | ios::ate);
	if (file.is_open())
		{
		streampos size = file.tellg();
		char * block = new char[size];
		file.seekg(offset, ios::beg);
		file.read(block, size);
		file.close();
		file.clear();
		return block;
		}
	}

void load_examples(char * block, Ref<MatrixXi> mat, int n)
	{
	for (int i = 0; i < n; i++)
		{
		for (int j = 0; j < 28 * 28; j++)
			{
			mat(i, j) = *(block + 28 * 28 * i + j);
			}
		}
	delete[] block;
	}

void load_labels(char * block, Ref<VectorXi> vec, int n)
	{
	for (int i = 0; i < n; i++)
		{
		vec(i) = *(block + i);
		}
	delete[] block;
	}

void copy_features(Ref<MatrixXi> examples, Ref<MatrixXd> features, int n) //Are these passed by reference?? - Yeah.
	{
	for (int i = 0; i < 28 * 28; i++)
		{
		if (examples(n, i) == 0) { features(0,i) = 0; } //int to double
		else { features(0,i) = 1; }
		}
	}

void copy_label(Ref<VectorXi> labels, Ref<RowVectorXd> label, int n)
	{
	for (int i = 0; i< 10; i++)
		{
		//Convert the integer value/label to a one-hot vector
		if (i == labels(n))	{ label(i) = 1; }
		else { label(i) = 0; }
		}
	}

void init_matrix(Ref<MatrixXd> mat) //Matrix matrix(m,n)
	{
	//Type of random number distribution
	
	std::uniform_real_distribution<double> dist(-0.4, 0.4);  //(min, max) NORMALISATION!!!
	std::mt19937 rng; //RNG
	rng.seed(std::random_device{}()); //Initialise with non-deterministic seeds
	
	for (int i = 0; i < mat.rows(); i++)
		{
		for (int j = 0; j < mat.cols(); j++)
			{
			//RNG TEST
			/*
			for (int i = 0; i < 10; i++)
				{
				std::cout << dist(rng) << std::endl;
				}
			*/
			mat(i, j) = 0;//dist(rng);
			}
		}
	}	

void softmax(Ref<RowVectorXd> logits, Ref<RowVectorXd> probabilities)
	{
	double sum = 0;
	for (int i = 0; i < 10; i++)
		{
		sum += exp(logits(i));
		}
	for (int i = 0; i < 10; i++)
		{
		probabilities(i) = exp(logits(i)) / sum;
		}
	}

int select(Ref<RowVectorXd> probabilities)
	{
	int maxindex = 0;
	for (int i = 0; i < 10; i++)
		{
		if (probabilities(i) > probabilities(maxindex))
			{
			maxindex = i;
			}
		}
	return maxindex;
	}

double cross_entropy(Ref<RowVectorXd> probabilities, Ref<RowVectorXd> label)
	{
	double loss = 0;
	for (int i = 0; i < 10; i++)
		{
		loss += -1 * log(probabilities(i)) * label(i);
		}
	return loss;
	}

RowVectorXd get_probabilities(Ref<MatrixXd> weights, Ref<RowVectorXd> biases, Ref<MatrixXd> features)
	{
	RowVectorXd logits(10);
	logits = (features * weights);
	logits = logits + biases;

	RowVectorXd probabilities(10);
	softmax(logits, probabilities);
	return probabilities;
	}

double test(Ref<MatrixXd> weights, Ref<RowVectorXd> biases, Ref<MatrixXi> examples, Ref<VectorXi> labels)
	{
	double accsum = 0;
	for (int j = 0; j < NUMBER_OF_TESTING_EXAMPLES; j++) //MAIN LOOP
		{
		MatrixXd features(1, 28 * 28); //THIS AS A MATRIX ENABLES TRANSPOSING
		copy_features(examples, features, j);
		
		RowVectorXd probabilities = get_probabilities(weights, biases, features);

		int result = select(probabilities);
		if (result == labels(j))
			{
			accsum += 1;
			}
		}
	return accsum/NUMBER_OF_TESTING_EXAMPLES*100;
	}

int main()
	{
	char *examplesblock, *labelsblock, *testingexamplesblock, *testinglabelsblock;

	examplesblock = read_file("MNIST\\EXAMPLES.bin", 0x10);
	labelsblock = read_file("MNIST\\LABELS.bin", 0x8);
	testingexamplesblock = read_file("MNIST\\TESTING_EXAMPLES.bin", 0x10);
	testinglabelsblock = read_file("MNIST\\TESTING_LABELS.bin", 0x8);

	cout << "The entire file content is now in memory\n";

	MatrixXi EXAMPLES(NUMBER_OF_EXAMPLES, 28 * 28); //Creating a fixed matrix causes a stack overflow....
	load_examples(examplesblock, EXAMPLES, NUMBER_OF_EXAMPLES);
	cout << NUMBER_OF_EXAMPLES << " training examples ready" << endl;
	VectorXi LABELS(NUMBER_OF_EXAMPLES);
	load_labels(labelsblock, LABELS, NUMBER_OF_EXAMPLES);
	cout << NUMBER_OF_EXAMPLES << " training labels ready" << endl;

	MatrixXi TESTING_EXAMPLES(NUMBER_OF_TESTING_EXAMPLES, 28 * 28);
	load_examples(testingexamplesblock, TESTING_EXAMPLES, NUMBER_OF_TESTING_EXAMPLES);
	cout << NUMBER_OF_TESTING_EXAMPLES << " testing examples ready" << endl;
	VectorXi TESTING_LABELS(NUMBER_OF_TESTING_EXAMPLES);
	load_labels(testinglabelsblock, TESTING_LABELS, NUMBER_OF_TESTING_EXAMPLES);
	cout << NUMBER_OF_TESTING_EXAMPLES << " testing labels ready" << endl;

	Matrix<double, 28 * 28, 10> WEIGHTS;
	RowVectorXd BIASES(10);

	init_matrix(WEIGHTS);
	init_matrix(BIASES);

	cout << "Starting main loop; learning rate: " << LEARINING_RATE << endl << endl;

	for (int j = 0; j < NUMBER_OF_EXAMPLES; j++) //MAIN LOOP
		{
		MatrixXd FEATURES(1, 28 * 28); //THIS AS A MATRIX ENABLES TRANSPOSING
		copy_features(EXAMPLES, FEATURES, j);

		RowVectorXd LABEL(10);
		copy_label(LABELS, LABEL, j);

	#ifdef DEBUG //FEATURE COPY TEST
		for (int h = 0; h < 28; h++)
			{
			for (int j = 0; j < 28; j++)
				{
				double pixel = FEATURES(28 * h + j);
				cout << pixel << " ";
				}
			cout << "\n";
			}
		cout << "\n";
	#endif // DEBUG

		RowVectorXd PROBABILITIES = get_probabilities(WEIGHTS, BIASES, FEATURES);

	#ifdef DEBUG //PROBABILITIES OUTPUT TEST
		cout << "PROBABILITIES (POST-SOFTMAX): \n";
		for (int prob = 0; prob < 10; prob++)
			{
			cout << PROBABILITIES(prob) << " ";
			}
		cout << "\n\n";
	#endif // DEBUG

	#ifdef DEBUG //LABELS OUTPUT TEST
		cout << "LABELS: \n";
		for (int label = 0; label < 10; label++)
			{
			cout << LABEL(label) << " ";
			}
		cout << "\n\n";
	#endif // DEBUG

		//OUTPUT
		if (j % 250 == 0)
			{
			//double LOSS = cross_entropy(PROBABILITIES, LABEL); //Not really needed for regression

			double ACCURACY = test(WEIGHTS, BIASES, TESTING_EXAMPLES, TESTING_LABELS);
			cout << left << "Iteration #" << j << ":" << "\t\t" << "ACCURACY: " << ACCURACY << "%" << endl;
			}

		//############PREEEETY IMPORTANT STUFF##############
		WEIGHTS -= LEARINING_RATE * (FEATURES.transpose() * (PROBABILITIES - LABEL)); //Is this really... it?
		BIASES -= LEARINING_RATE * (PROBABILITIES - LABEL);
		}
	cout << "PROGRAM FINIHSED";
	std::getchar();
	}