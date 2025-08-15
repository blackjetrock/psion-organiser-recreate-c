'Bifurcation diagram of the logistic map
Const W = 320
Const H = 320

Const R_MIN = 2.5
Const R_MAX = 4.0
Const ITER = 200
Const LAST_ITER = 50

Dim x As FLOAT
Dim r As FLOAT

Color RGB(128, 128, 128)

For i = 0 To W
  r = R_MIN + (R_MAX - R_MIN) * i / W
  x = 0.5
  For j = 1 To ITER
    x = r * x * (1 - x)
    If j > (ITER - LAST_ITER) Then
      Pixel H - Int(x * H), i
    End If
  Next j
Next i