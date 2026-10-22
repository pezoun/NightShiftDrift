# První konzultace – 23. října 2026

## Co předkládám

- Existující zdrojový základ automobilové hry v Unreal Engine.
- Pracovní rešerši s deseti zdroji a návrhovými závěry.
- Rozpad všech devíti bodů zadání na výstupy a ověření.
- Harmonogram do 16. dubna s interním dokončením 9. dubna.
- Prosincový milník: funkční smyčka jízda → závod → odměna → uložení.
- Návrh prvního závodu a dvanáct ověřovacích scénářů.

## Co je skutečně ověřeno k 22. 10.

Zdrojový kód obsahuje vozidlový pawn a komponenty řízení, kamery, převodovky, telemetrie, respawnu, zvuků a HUD. Tato kontrola neprokazuje bezchybnou funkčnost při jízdě. Původní commit `04536ec` je dostupný na vzdálené větvi `main`.

Horská mapa, závodní systém s odměnami, ukládání a garáž se tímto dokumentem nevykazují jako dokončené. Pokus o zálohu dočasné mapy 22. října skončil chybou World Partition a pádem editoru; vznikl jen testovací mesh. Před dalšími zásahy bude nutné obnovit stabilní uloženou mapu a ověřit její opětovné otevření.

## Krátké představení záměru

Chci navázat na současný základ auta a vytvořit hru s realisticky působící japonskou horskou oblastí. Zaměřím se hlavně na drift, ale doplním i časové závody. Do prosince chci předvést malou, skutečně hratelnou část se závodem, odměnou a uložením. V lednu rozšířím garáž a tuning, v únoru doplním mapu a závody. Březen a začátek dubna vyhradím na testování, dokumentaci a finální výstupy.

## Otázky pro vedoucího

1. Je kombinace driftových a časových závodů bez AI soupeřů dostatečná pro požadovaný závodní systém?
2. Je prosincový milník s dvěma hratelnými závody a základním ukládáním přiměřený?
3. Jaký rozsah a citační formát má mít závěrečná teoretická část?
4. Mohou tři vozy využívat společný technický základ, pokud mají odlišné parametry a modely dostupné v souladu s licencemi?
5. Jaké jsou školní požadavky na používání externích assetů, podpůrných nástrojů a jejich uvedení v dokumentaci?
6. Jak se předává hra, video, dokumentace a případné další prezentační soubory? Potvrdit termín 16. 4. do 14:00.

## Další praktický krok

Po potvrzení rozsahu ověřím současné ovládání na krátké bezpečně uložené testovací mapě a implementuji první časový závod podle [specifikace](prvni-zavod.md). Náročnou finální krajinu zatím nebudu považovat za prioritu před základní herní smyčkou.

## Záznam po konzultaci

- Závěry vedoucího: doplnit po konzultaci.
- Potvrzený rozsah: doplnit po konzultaci.
- Změny harmonogramu: doplnit po konzultaci.
- Následující termín konzultace: zatím nepotvrzen.
