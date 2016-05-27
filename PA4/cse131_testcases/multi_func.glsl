
vec2 v;

float foo(float a) 
{
  return v.x + a;
}

float bar(int a)
{
  float t;
  if ( a > 1 )
    t = v.y;
  else
    t = 0.25;
  return t;
}

