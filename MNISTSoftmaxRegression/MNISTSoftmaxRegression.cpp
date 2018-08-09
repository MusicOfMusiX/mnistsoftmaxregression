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

double cross_entropy(Ref<RowVectorXd> probabilities, Ref<RowVectorXd> label)
	{
	double loss = 0;
	for (int i = 0; i < 10; i++)
		{
		loss += -1 * log(probabilities(i)) * label(i);
		}
	return loss;
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

double test(Ref<MatrixXd> weights, Ref<RowVectorXd> biases, Ref<MatrixXi> examples, Ref<VectorXi> labels)
	{
	double accsum = 0;
	for (int j = 0; j < NUMBER_OF_TESTING_EXAMPLES; j++) //MAIN LOOP
		{
		MatrixXd features(1, 28 * 28); //THIS AS A MATRIX ENABLES TRANSPOSING
		copy_features(examples, features, j);
		
		RowVectorXd logits(10);
		logits = (features * weights);
		logits = logits + biases;

		RowVectorXd probabilities(10);
		softmax(logits, probabilities);

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
	streampos size;
	char *examplesblock, *labelsblock, *testingexamplesblock, *testinglabelsblock;

	ifstream file("MNIST\\EXAMPLES.bin", ios::in | ios::binary | ios::ate);

	if (file.is_open())
		{
		size = file.tellg();
		examplesblock = new char[size];
		file.seekg(0x10, ios::beg);
		file.read(examplesblock, size);
		file.close();
		file.clear();

		file.open("MNIST\\LABELS.bin", ios::in | ios::binary | ios::ate);
		size = file.tellg();
		labelsblock = new char[size];
		file.seekg(0x8, ios::beg);
		file.read(labelsblock, size);
		file.close();
		file.clear();

		file.open("MNIST\\TESTING_EXAMPLES.bin", ios::in | ios::binary | ios::ate);
		size = file.tellg();
		testingexamplesblock = new char[size];
		file.seekg(0x10, ios::beg);
		file.read(testingexamplesblock, size);
		file.close();
		file.clear();

		file.open("MNIST\\TESTING_LABELS.bin", ios::in | ios::binary | ios::ate);
		size = file.tellg();
		testinglabelsblock = new char[size];
		file.seekg(0x8, ios::beg);
		file.read(testinglabelsblock, size);
		file.close();
		file.clear();

		cout << "The entire file contents are now in memory\n";
		//FILE READ TEST
		/*
		for (int num = 0; num < 10; num++)
		{
		for (int i = 0; i < 28; i++)
		{
		for (int j = 0; j < 28; j++)
		{
		int pixel = *(memblock + 28 * 28 * num + 28 * i + j);
		if (pixel == 0)
		{
		cout << 0 << " ";
		}
		else
		{
		cout << 1 << " ";
		}
		}
		cout << "\n";
		}
		cout << "\n";
		}
		*/

		MatrixXi EXAMPLES(NUMBER_OF_EXAMPLES, 28 * 28); //Creating a fixed matrix causes a stack overflow....
		for (int i = 0; i < NUMBER_OF_EXAMPLES; i++)
			{
			for (int j = 0; j < 28 * 28; j++)
				{
				EXAMPLES(i, j) = *(examplesblock + 28 * 28 * i + j);
				}
			}
		delete[] examplesblock;
		cout << "Training examples ready" << endl;
		VectorXi LABELS(NUMBER_OF_EXAMPLES);
		for (int i = 0; i < NUMBER_OF_EXAMPLES; i++)
			{
			LABELS(i) = *(labelsblock + i);
			}
		delete[] labelsblock;
		cout << "Training labels ready" << endl;

		MatrixXi TESTING_EXAMPLES(NUMBER_OF_TESTING_EXAMPLES, 28 * 28);
		for (int i = 0; i < NUMBER_OF_TESTING_EXAMPLES; i++)
			{
			for (int j = 0; j < 28 * 28; j++)
				{
				TESTING_EXAMPLES(i, j) = *(testingexamplesblock + 28 * 28 * i + j);
				}
			}
		delete[] testingexamplesblock;
		cout << "Testing examples ready" << endl;
		VectorXi TESTING_LABELS(NUMBER_OF_TESTING_EXAMPLES);
		for (int i = 0; i < NUMBER_OF_TESTING_EXAMPLES; i++)
			{
			TESTING_LABELS(i) = *(testinglabelsblock + i);
			}
		delete[] testinglabelsblock;
		cout << "Testing labels ready" << endl;

		Matrix<double, 28 * 28, 10> WEIGHTS;
		RowVectorXd BIASES(10);

		init_matrix(WEIGHTS);
		init_matrix(BIASES);
		

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
			RowVectorXd LOGITS(10);
			LOGITS = (FEATURES * WEIGHTS);
			LOGITS = LOGITS + BIASES;

			#ifdef DEBUG //LOGITS OUTPUT TEST
				cout << "LOGITS: \n";
				for (int logit = 0; logit < 10; logit++)
					{
					cout << LOGITS(logit) << " ";
					}
				cout << "\n\n";
			#endif //DEBUG
			RowVectorXd PROBABILITIES(10);
			softmax(LOGITS, PROBABILITIES);

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

			//LOSS OUTPUT
			//cout << "LOSS: \n";
			if (j % 250 == 0)
				{
				double LOSS = cross_entropy(PROBABILITIES, LABEL); //Not really needed for regression
				
				double ACCURACY = test(WEIGHTS, BIASES, TESTING_EXAMPLES, TESTING_LABELS);
				cout << "LOG LOSS: " << LOSS << "		ACCURACY: " << ACCURACY << "%" << endl;
				}
			
			//############PREEEETY IMPORTANT STUFF##############
			WEIGHTS -= LEARINING_RATE * (FEATURES.transpose() * (PROBABILITIES - LABEL)); //Is this really... it?
			BIASES -= LEARINING_RATE * (PROBABILITIES - LABEL);
			}
		}
	else cout << "Unable to open file";
	cout << "PROGRAM FINIHSED";
	std::getchar();
	}

