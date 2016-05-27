int whiletest(int b)
{

  int i;
  int sum;

  i = 0;
  sum = 0;
  while ( i < b ) {
    sum += i;
    i = i + 1;
  }
  return sum;
}
