* ENCODING=ISO-8859-1
NAME          .\SP1.mps
ROWS
 N  OBJ     
 L  prod    
 L  lines   
 G  demand  
COLUMNS
    t         lines                          -1
    p         prod                           -1
    z         OBJ                           1.5
    z         prod                          1.5
    z         lines                           1
    z         demand                          1
RHS
    rhs       demand                          2
BOUNDS
 FX bnd       t                               0
 FX bnd       p                               0
ENDATA
