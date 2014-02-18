#ifndef __MISC_LIB

  #define uabs(x)  ( ( (x) < 0) ? -(x) : (x) )
  
  uint32_t reduce(uint32_t value, uint8_t decrease)
  {
    uint16_t dec=1;
    for(;decrease>1;decrease--)
      dec*=10;

    if (value/dec%10 > 4)
      return value/(10*dec)+1;
    else
      return value/(10*dec);
  }

  char* itoa(uint32_t val, int base,int min)
  {
    static char buf[32] = {0};
    char neg=0;
    int i = 30;

    if (val==0)
    {
      buf[i--]='0';
    }

    else
    {
      if(val<0)
      {
        val=uabs(val);
        neg=1;
      }

      for (; (val && i); --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
     
      if(neg)
        buf[i--]='-';
    }

    for (; min+i>30; --i)
      buf[i] = ' ';

    return &buf[i+1];
  }
  #define array_length(A) sizeof(A)/sizeof(A[0])



  #define __MISC_LIB
#endif