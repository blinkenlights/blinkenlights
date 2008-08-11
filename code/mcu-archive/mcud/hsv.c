#define RES    1000
#define INT(x) ((x/RES)*RES)


static void
hsv_to_rgb (unsigned char hue, unsigned char sat, unsigned char val,
			unsigned char *red, unsigned char *green, unsigned char *blue)
{
  long h, s, v;
  long f, p, q, t;
  int r, g, b;

  if (sat == 0)
    {
		r = val;
		g = val;
		b = val;
    }
  else
    {
      h = hue * RES * 6 / 255;
      s = sat * RES     / 255;
      v = val * RES     / 255;

      f = h - (INT(h));
      p = v * (RES - s);
      q = v * (RES - (s * f));
      t = v * (RES - (s * (RES - f)));

      switch (INT(h))
        {
        case 0:
          r = v * 255;
          g = t * 255;
          b = p * 255;
          break;

        case 1:
          r = q * 255;
          g = v * 255;
          b = p * 255;
          break;

        case 2:
          r = p * 255;
          g = v * 255;
          b = t * 255;
          break;

        case 3:
          r = p * 255;
          g = q * 255;
          b = v * 255;
          break;

        case 4:
          r = t * 255;
          g = p * 255;
          b = v * 255;
          break;

        case 5:
          r = v * 255;
          g = p * 255;
          b = q * 255;
          break;
        }
        r = r / RES;
        g = g / RES;
        b = b / RES;
    }
    
    *red   = r;
    *green = g;
    *blue  = b;
}

