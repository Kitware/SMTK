template <unsigned S> class MyTempl { public: char data[S]; }; 
template <> class MyTempl<0> { public: char value; }; 

int main()
{
  MyTempl<1> one; 
  MyTempl<0> zero; 
  one.data[0] = zero.value = '\0'; 

  return 0;
}
