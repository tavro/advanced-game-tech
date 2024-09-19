# advanced-game-tech

## l1.1 frågor

### Hur ser högdagrarna ut i 1a? Varför?

högdagrarna i 1a är tydligt ljusa fläckar på modellen och detta beror på att shaders arbetar med värden över 1.

### Hur allokerar man en tom textur?

genom att allokera ett FBO och tilldela en textur till denna.

### Hur många pass körde du lågpassfiltret i 1c och 1d?

5

### Bör trunkeringen göras i egen shader eller som del av en shader som gör något mer? Varför?

trunkeringen bör göras i en egen shader. om varje shader gör en specifik sak kan man lättare optimera eller finjustera shadern. (blir en egen del i pipelinen eller vad man ska säga)

## l1.2 frågor

### Vilken bumpmappning tycker du är att föredra, vykoordinater eller texturkoordinater? Varför? Vad är skillnaden mellan att arbeta i vy- och texturkoordinater? Vilken bumpmappning (2a eller 2b) är lämplig för normalmapping?

bumpmappning i texturkoordinater är att föredra. för texturkoordinater är ett lokalt koordinatsystem för varje punkt detta ger ett mer konsekvent resultat.

i vykoordinater omvandlas normalerna till "kamerans" koordinatsystem. om kameran rör sig kan det vara problematiskt t.ex.

### Definierar du bumpmappen som avstånd in i eller ut ur objektet? Var spelar det in?

"ut" ur objektet. beroende på om normalerna har negativ
eller positiv riktning ser det ut som att ytan "höjer" sig eller "sänker" sig.

### Blev Mvt rätt med mat3 i 2b? Om inte, vad gjorde du åt det?

ja, om det inte skulle ha blivit rätt skulle vektorn transformerats fel. man skulle se det på bumpmap-effekten... allt skall vara normaliserat korrekt.

## l2 frågor

### I del 2, translaterade du med translationsmatris eller med addition på koordinater? Spelar det någon roll?

addition på positionsvektorer (koordinater). båda ger samma resultat men matriser kan lättare kombineras med rotation och skalning etc...

### Hur löste du GPU-skinningen? Vilken information behövde skickas till shadern?

flyttade beräkningen av vertextransformationer till shadern. skickade med rotations- och positionsmatriser

### Om man gör skinning med en mer komplex modell (armar och ben mm), behövs vilolägesrotationer egentligen?

ja, det är grunden för beräkningarna! annars kan modellen deformeras på lustiga vis