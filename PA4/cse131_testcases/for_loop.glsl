vec2 v;

float fortest(int a)
{
  int i;
  float sum;

  sum = v.x;
  for ( i = 0; i < a; i += 1 )
    sum *= v.y;

  return sum;
}
