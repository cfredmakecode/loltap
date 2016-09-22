#include "math.h"

typedef struct v2 { float x, y; } v2;

v2 operator+(v2 uno, v2 dos) {
  v2 result = {0, 0};
  result.x = uno.x + dos.x;
  result.y = uno.y + dos.y;
  return result;
}

v2 operator+(v2 uno, float dos) {
  v2 result = uno;
  result.x += dos;
  result.y += dos;
  return result;
}

v2 operator-(v2 uno, v2 dos) {
  v2 result = {0, 0};
  result.x = uno.x - dos.x;
  result.y = uno.y - dos.y;
  return result;
}

v2 operator*(v2 uno, v2 dos) {
  v2 result = {0, 0};
  result.x = uno.x * dos.x;
  result.y = uno.y * dos.y;
  return result;
}

v2 operator*(v2 uno, float dos) {
  v2 result = {0, 0};
  result.x = uno.x * dos;
  result.y = uno.y * dos;
  return result;
}

v2 operator/(v2 uno, v2 dos) {
  v2 result = {0, 0};
  result.x = uno.x / dos.x;
  result.y = uno.y / dos.y;
  return result;
}

float magnitude(v2 vec) {
  float result = 0.0f;
  result = sqrt(pow(vec.x, 2) + pow(vec.y, 2));
  return result;
}

v2 normalize(v2 vec) {
  float mag = magnitude(vec);
  v2 result;
  result.x = vec.x / mag;
  result.y = vec.y / mag;
  return result;
}
