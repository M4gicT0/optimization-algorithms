/*
 * Algorithm.cpp
 * Copyright (C) 2017 transpalette <transpalette@arch-cactus>
 *
 * Distributed under terms of the MIT license.
 */

#include "Algorithm.h"
#include <math.h>
#include "CImg.h"

using namespace cimg_library;


Algorithm::Algorithm(HandWrittenNumbers *data) {
	input_data = data;
}

Algorithm::Algorithm(FacialImageSet *data) {
	input_data = data;
}

Algorithm::~Algorithm() {
	delete input_data;
}

void Algorithm::visualiseData() {

}

void Algorithm::visualiseImageVector(Eigen::VectorXd image_vector) {
    /* Build the matrix from the vector */
    int width = input_data->getWidth();
    int height = input_data->getHeight();
    double image_matrix[width][height];

    for (int i = 0; i < image_vector.size() / width; i++)
        for (int j = 0; j < height; j++)
            image_matrix[i][j] = image_vector(j + (width * i));

    CImg<double> image((const double*) image_matrix, width, height, 1, 1, true);
    image.display("Image");
}

float Algorithm::calculateAccuracy() {
    float positives = 0;

    for (auto &element : input_data->getTestingElements())
	if (element.label == element.given_class)
	    positives++;

    positives /= input_data->getTestingElements().size();

    return positives;
}

void Algorithm::nearestClassCentroid() {
    Eigen::IOFormat fmt(2, Eigen::DontAlignCols, "\t", " ", "", "", "", "");
    /* Training part: construct the mean vector of each class */
    std::vector<Eigen::VectorXd> mean_class_vectors(input_data->getNbClasses());

    std::cout << "\t-> Building mean class vectors..." << std::endl;
    for (int i = 0; i < input_data->getNbClasses(); i++) {
	/* Get class at index i: if class 0 is equal to "Blue", returns "Blue" */
	auto currentClass = input_data->getClass(i);
	mean_class_vectors.at(i).resize(input_data->getVectorSize()); //Set Eigen Vector size
	mean_class_vectors.at(i).setZero();
	int nb_samples = 0;
	/* Iterate through the training data, it is an iterator representing a training Element (see struct) */
	for (auto const& element : input_data->getTrainingElements()) {
	    /* Make sure we're updating the proper mean class vector */
	    if (element.label == currentClass) {
		nb_samples++;
		mean_class_vectors.at(i) += element.data; //Add up this picture's vector
	    }
	}

	/* Calculate the average of the mean vector */
	mean_class_vectors.at(i) = mean_class_vectors.at(i) / nb_samples;
    }

    /* Classification part: classify the element to the smallest distance between
     * itself and each mean vector 
     */
    std::cout << "\t-> Running classification..." << std::endl;
    for (auto &element : input_data->getTestingElements()) {
	/* Calculate the distance for each mean class vector */
	double distance = 0, minDistance = 0;
	int optimumClass = 0;

	for (int i = 0; i < input_data->getNbClasses(); i++) {
	    Eigen::VectorXd diffVector(element.data - mean_class_vectors.at(i));
	    distance = pow(diffVector.norm(), 2.0);

	    if (distance < minDistance || !minDistance) {
		minDistance = distance;
		optimumClass = i;
	    }
	}

	/* Classify the element by setting its label to the best match */
	element.given_class = input_data->getClass(optimumClass);
    }
	
    std::cout << "\t-> Visualizing first test element: label -> " << input_data->getTestingElements().at(0).label << " - class -> "
       << input_data->getTestingElements().at(0).given_class << std::endl;	
    visualiseImageVector(input_data->getTestingElements().at(0).data);
}
