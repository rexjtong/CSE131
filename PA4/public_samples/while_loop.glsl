int b;
int whiletest()
{

  int i;
  int sum;

  i = 0;
  sum = 0;
  while ( i < b ) {
    sum = sum + i;
    i = i + 1;
  }
  return sum;
}
