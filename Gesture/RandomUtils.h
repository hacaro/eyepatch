#pragma once

/**
 * Utilities for generating random numbers,
 * distributed uniformly and as a gaussian
 */


double RandomDouble(double min, double max);
int RandomInt(int min, int max);
double RandomGaussian(double mean, double variance);
