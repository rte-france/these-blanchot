\Problem name: master                                                           

Minimize
 20000 C0001 + 10000 C0002 + 1000 C0003 + 100 C0004 + alpha

Subject To
R0001: C0001 <= 10000000 
          C0001 >= 0 
R0002: C0002 <= 10000000 
          C0002 >= 0 
R0003: C0003 <= 10000000 
          C0003 >= 0 
R0004: C0004 <= 10000000 
          C0004 >= 0 

Bounds
C0001 free
C0002 free
C0003 free
C0004 free
alpha >= -10000000000 

End
