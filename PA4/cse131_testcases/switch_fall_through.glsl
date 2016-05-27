vec2 v;
int a;

float switchtest2()
{
   float f;
   f = 0.0;

   switch( a ) {
     case 0: f  = 0.0; break;
     case 1: f  = f + 1.1;
     case 2: f  = f + 2.2; break;
     default: f = 3.0; break;
   }

   return f;
}
