int a;

int postfix(int b)
{
  int i;

  i = b--;
  i += a++;

  return i + 2 * (a + b);
}
