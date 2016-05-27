vec2 a;

float unary(float f)
{
  float t;

  t = ++f;
  a.x = --t;
  a.y = ++t;

  return a.x + a.y + f--;
}
