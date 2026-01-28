#include <JuceHeader.h>

float dotProduct(const std::vector<float>& x, const std::vector<float>& y);
float magnitude(const std::vector<float>& x);
std::vector<float> difference(const std::vector<float>& x, const std::vector<float>& y);
const std::vector<float> scale(const std::vector<float>& x, float s);
const std::vector<float> normalize(const std::vector<float>& x);
float AngleBetweenVectors(const std::vector<float>& vector1, const std::vector<float>& vector2, bool direction);

// Dot product with another vector.
float dotProduct(const std::vector<float>& x, const std::vector<float>& y) {
	float dot = 0;
	for (int i = 0; i < x.size(); i++)
	{
		dot += x[i] * y[i];
	}
	return dot;
}
// Get the magnitude of this vector.
float magnitude(const std::vector<float>& x)
{
	return sqrt(dotProduct(x, x));
}
// Difference with another vector.
std::vector<float> difference(const std::vector<float>& x, const std::vector<float>& y)
{
	int i, s = x.size();
	std::vector<float> diff(s);
	for (i = 0; i < s; i++) diff[i] = x[i] - y[i];
	return diff;
}
// Get a copy of this vector multiplied by a scalar.
const std::vector<float> scale(const std::vector<float>& x, float s)
{
	int lengthX = x.size();
	std::vector<float> scaledX(lengthX);
	for (int i = 0; i < lengthX; i++)  scaledX[i] = s * x[i];
	return scaledX;
}
// Get normalized copy of this vector
const std::vector<float> normalize(const std::vector<float>& x)
{
	float m = magnitude(x);
	if (m == 0) return x;
	return scale(x, 1 / m);
}
// Compute the angle between two vectors. If direction matters, then negative 
// angles are those that are counterclockwise from first to second vector
float AngleBetweenVectors(const std::vector<float>& vector1, const std::vector<float>& vector2, bool direction)
{
	const std::vector<float>& v1norm = normalize(vector1);
	const std::vector<float>& v2norm = normalize(vector2);
	float dot = dotProduct(v1norm, v2norm);
	float angle = 180 * acos(dot) / juce::MathConstants<float>::pi;
	if (direction)
	  if (v1norm[0] * v2norm[1] - v1norm[1] * v2norm[0] > 0) angle *= -1;
	return angle;
}
int main()
{
	std::vector<float> x{ 3.0, 4.0, 0.0 };
	std::vector<float> y{ 2.0, 1.0, 0.0 };
	std::cout << dotProduct(x, y) << '\n';
	std::cout << magnitude(x) << '\n';
	std::cout << difference(x, y)[0] << ' ' << difference(x, y)[1] << '\n';
	std::cout << scale(x, 2.0)[0] << ' ' << scale(x, 2.0)[1] << '\n';
	std::cout << normalize(x)[0] << ' ' << normalize(x)[1] << '\n';
	float a = AngleBetweenVectors(x, y, true);
	float b = AngleBetweenVectors(x, y, false);
	float c = AngleBetweenVectors(y, x, true);
	float d = AngleBetweenVectors(y, x, false);
	std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
	return 0;
}