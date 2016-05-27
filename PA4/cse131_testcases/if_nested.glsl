
vec2 v;

float ifnested(float arg)
{
   float f;
   if ( arg > 1.0 ) {
      if ( v.x < 0.5 ) 
         f = v.x;
      else
         f= v.y;
   } else {
      f = arg;
   } 
   return f;
}

