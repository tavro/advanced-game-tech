# 1

## Hur gjorde du för att söka igenom alla boids?

Alla sprites är strukturerade som en länkad lista där gSpriteRoot är föräldernoden. Vi loopar genom dem.

# 2

## Vilken funktion valde du för den bortstötande kraften? 

Det kan ändras linjärt/icke-linjärt.

## Är din separation bra eller kan du tänka dig finjusteringar?

Ja, går säkert att finjustera. T.ex. med hjälp av GUI.

# 3

## Vilken vikt lade du på alignment? Hur mycket rätar de upp sig efter varandra per iteration?

-

# 4

## Hur håller du reda på ditt "svarta får”?

En flagga i datastrukturern.

## Är ditt svarta får en "ledare" eller en "busig flockmedlem". Hur skulle du ha gjort för att göra den andra av dessa två?

En busig flockmedelem. För att göra fåret till ledaren vill man att den skall vara roten i funktionsanropet.