int a;
float switchtest()
{
   float f;

   switch( a ) {
     case 0: f  = 0.0; break;
     case 1: f  = 1.0; break;
     case 2: f  = 2.0; break;
     default: f = 3.0; break;
   }

   return f;
}
