float v;
int a;
float fortest()
{
  int i;
  float sum;

  sum = v;
  for ( i = 0; i < a; ++i ) {
    if ( i > 5 )
      break;
    sum *= 3.0;
  }

  return sum;
}
