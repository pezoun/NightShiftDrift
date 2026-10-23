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

Projekt obsahuje C++ základ vozidla využívající Chaos Vehicles, komponenty ovládání, kamery, převodovky, telemetrie, respawnu a zvuku a základ HUD. Přítomnost implementace byla ověřena čtením zdrojů; kompletní funkčnost musí potvrdit herní test.

Závodní systém s odměnami, garáž se třemi vozy, pět kategorií vylepšení a ukládání postupu jsou plánované části. První topografie horské oblasti a síť cest jsou v mapě `/Game/NightshiftDrift/Maps/L_Touge_Region`. [Popis mapy a skutečné výsledky jízdní zkoušky](docs/mapa-a-test-vozidla.md). Obsah `TougePrototype/Development` je starší experiment.

## Otevření projektu

1. Naklonovat repozitář s nainstalovaným Git LFS a stáhnout LFS obsah příkazem `git lfs pull`.
2. Použít odpovídající instalaci Unreal Engine; `Maturita.uproject` aktuálně uvádí asociaci `5.8`.
3. Pro sestavení C++ mít nástroje pro vývoj Unreal projektů ve Windows a závislosti příslušné verze enginu.
4. Otevřít `Maturita.uproject`. Dostupnost požadovaných pluginů a sestavení ověřit na cílovém počítači.

Mapy a assety jsou verzované pomocí Git LFS. `Saved`, `Intermediate`, `Binaries` a cache nejsou zdrojovou částí projektu. Spustitelný distribuční build zatím není v tomto README deklarován jako hotový.
