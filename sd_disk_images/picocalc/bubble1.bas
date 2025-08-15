CLS

n=200
r=tau/235
u=0
v=0
x=0
y=0
t=0
Do
CLS
For i=0 To n
For j=0 To n
u=Sin(i+v)+Sin(r*i+x)
v=Cos(i+v)+Cos(r*i+x)
x=u+t
Pixel u*80+160,v*80+160,RGB(i,j,99)
Next j
Next i
t=t+0.25
Loop
