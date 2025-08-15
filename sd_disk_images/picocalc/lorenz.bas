'Lorenz attractor

Const S=10,R=28,B=8/3
Const DT=0.01,W=320,H=320
Const XMIN=-30,XMAX=30
Const YMIN=-30,YMAX=30
Const ZMIN=0,ZMAX=60

Dim x As FLOAT=1,y As FLOAT=1
Dim z As FLOAT=1,px,py,nx,ny,i

Function M(c,mi,ma,sm)
 M=Int((c-mi)/(ma-mi)*sm)
End Function

CLS :Color RGB(255,127,0)

For i=1 To 10000
 nx=M(x,XMIN,XMAX,W)
 ny=M(y,YMIN,YMAX,H)
 nz=M(z,ZMAN,ZMAX,H)

 If i>1 Then Line px,H-py,nx,H-ny
'If i>1 Then Line px,H-pz,nx,H-nz

 px=nx:py=ny:pz=nz
 x=x+DT*S*(y-x)
 y=y+DT*(x*(R-z)-y)
 z=z+DT*(x*y-B*z)
Next

End