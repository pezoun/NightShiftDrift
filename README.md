# Nightshift Drift

Maturitní projekt Matěje Kadlčáka, Creative Hill College, školní rok 2026/2027.
Vedoucí práce: Ing. Jaromír Švejda, Ph.D.

Automobilová hra v Unreal Engine zaměřená na volnou jízdu, drift, závody a postupné vylepšování vozidel. Výtvarným cílem je realistická horská oblast inspirovaná japonskými silnicemi.

**Odevzdání podle zadání: 16. dubna 2027 do 14:00. Interní dokončení: 9. dubna 2027.**

## Podklady k první konzultaci – 23. října 2026

- [Literární a technická rešerše](docs/literarni-reserse.md)
- [Plán realizace a harmonogram](docs/harmonogram.md)
- [Požadavky a kritéria dokončení](docs/pozadavky.md)
- [Návrh prvního závodu a ověřovací scénáře](docs/prvni-zavod.md)
- [Stav projektu a témata konzultace](docs/konzultace-2026-10-23.md)

## Aktuální stav

- Volná jízda a drift v `MapOne` s fyzikou Chaos Vehicles, kamerou, respawnem, hudbou a zvukem motoru.
- Zelený digitální HUD inspirovaný dobovými přístrojovými štíty. Ukazuje rychlost, otáčky, převodový stupeň, skóre driftu a stav peněz. Závodní údaje se zobrazují během aktivního závodu.
- Hlavní nabídka, pauza a nastavení hlasitosti hudby a efektů. Pauzu otevřeš klávesou **Esc** nebo tlačítkem **Start** na ovladači.
- Ruční uložení z pauzy, pokračování z hlavní nabídky a volitelné automatické ukládání každou minutu jízdy. Uložení obsahuje pozici vozidla a nastavení.

Závody s odměnami, nákup vozů a jejich vylepšování jsou ve vývoji. Pro závody jsou připravené údaje pro HUD a napojení automatického uložení po dojetí do cíle. Horský prototyp `L_Touge_Region` a výsledky jízdní zkoušky popisuje [zpráva o mapě a testu vozidla](docs/mapa-a-test-vozidla.md).

## Otevření projektu

1. Naklonovat repozitář s nainstalovaným Git LFS a stáhnout LFS obsah příkazem `git lfs pull`.
2. Použít odpovídající instalaci Unreal Engine; `Maturita.uproject` aktuálně uvádí asociaci `5.8`.
3. Pro sestavení C++ mít nástroje pro vývoj Unreal projektů ve Windows a závislosti příslušné verze enginu.
4. Otevřít `Maturita.uproject`. Dostupnost požadovaných pluginů a sestavení ověřit na cílovém počítači.

Mapy a assety jsou verzované pomocí Git LFS. `Saved`, `Intermediate`, `Binaries` a cache nejsou zdrojovou částí projektu. Spustitelný distribuční build zatím není v tomto README deklarován jako hotový.
