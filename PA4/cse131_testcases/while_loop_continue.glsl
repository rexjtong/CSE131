int b;
int whiletest()
{

  int i;
  int sum;

  i = 0;
  sum = 0;
  while ( i < b ) {
    sum = sum + i;
    if ( i > 50 ) {
      i = 1;
      continue;
    }
    if ( sum > 1024 )
      break;
    i = i + 1;
  }
  return sum;
}
