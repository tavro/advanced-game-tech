# 2

## Vad blir den förenklade formeln?

  -(e + 1) dot vREL
----------------------
(1/mi + 1/mj) * normal

## Vad behöver du göra för att inte bollarna skall fastna i varandra?

Vi behöver göra ytterligare en check för att se så att bollarna inte är i varandra och/eller rör sig åt samma håll.

## Vilket av testfallen var svårast att uppfylla, och varför?

Testfall 2, svårt att visualisera/se vad som händer

# 3

## Vad blir tröghetsmatrisen för denna modell?

float j = 2.0/5.0 * ball[i].mass * kBallSize * kBallSize;
mat3 I = { j, 0, 0, 
		   0, j, 0, 
		   0, 0, j };

## Hur kan denna tröghetsmatris på enklast möjliga sätt användas för att beräkna rotationshastigheten omega från rörelsemängdsmomentet L?

Multiplacera L med tröghetsmatrisens invers?
ball[i].omega = InvertMat3(I) * ball[i].L;

## För att nu beräkna en kraft måste vi beräkna hastighetsskillnaden i bollens kontaktpunkt med underlaget. Hur finner vi denna hastighetsskillnad?

-