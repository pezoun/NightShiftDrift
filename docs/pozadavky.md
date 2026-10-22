# Přehled požadavků a důkazů splnění

Přepsáno a shrnuto z fotografie školního zadání. Stav k 22. 10. 2026. „Základ v kódu“ znamená přítomnost relevantních zdrojů, nikoli provedený herní test.

| Bod zadání | Výstup | Podmínka dokončení | Stav |
|---|---|---|---|
| 1. Literární rešerše | Teoretický text se zdroji | Odkazy v textu, bibliografie, závěry pro vlastní návrh, kontrola s vedoucím | Pracovní verze v tomto repozitáři |
| 2. UI, hlavní menu a HUD | Menu, pauza, nastavení a jízdní/závodní HUD | Všechny hlavní akce dosažitelné, text čitelný při jízdě | Základ HUD v kódu; úplná integrace neověřena |
| 3. Herní mapa a volná jízda se závody | Propojená horská oblast | Průjezdné trasy, kolize, bezpečný respawn a spustitelné závody | Nová mapa plánovaná |
| 4. Úprava fyziky | Vyladěné řízení a drift | Zaznamenané porovnání nastavení na stejných scénářích | Existující implementace; cílené testy plánované |
| 5. Závodní systém a odměny | Start, průběh, konec a připsání odměny | Pořadí kontrolních bodů, zrušení, odměna právě jednou za pokus | Navrženo v dokumentaci |
| 6. Ukládání a volba autosave | SaveGame a nastavení | Obnovení financí, vozů, úprav, výsledků a bezpečného místa po restartu | Plánováno |
| 7. Nejméně 3 vozy a 5 druhů úprav | Garáž, nákup a aplikace | Nedostatek peněz je ošetřen, vlastnictví se ukládá, změny ovlivňují jízdu | Plánováno |
| 8. Zvuky a hudba | Motor, smyk, UI, závody a hudba | Funkční a vyvážené přehrávání, evidence práv | Komponenty motoru a rádia v kódu; úplnost neověřena |
| 9. Spustitelná hra a krátké video | Windows build a video | Spuštění mimo editor a ukázka hlavní herní smyčky | Plánováno |

## Vlastní cíle nad rámec výslovných minim

- Pět závodních událostí, hlavně drift, doplněný časovým sprintem nebo okruhem.
- Realistická vizuální podoba japonské horské oblasti.
- Vegetace přes PCG s vyloučením silnic a míst, kde by omezovala hratelnost.
- Přibližný počáteční rozsah mapy 4 × 4 km, upravitelný podle kapacity a výkonu.

## Pět plánovaných kategorií úprav

| Kategorie | Zamýšlený skutečný účinek | Důkaz |
|---|---|---|
| Motor | Změna dostupného točivého momentu | Porovnání zrychlení za shodných podmínek |
| Pneumatiky | Změna přilnavosti | Průjezd stejnou zatáčkou a nástup smyku |
| Podvozek | Změna tuhosti/tlumení odpružení | Stejná nerovnost a přenos hmotnosti |
| Brzdy | Změna brzdného účinku | Brzdná dráha ze stejné rychlosti |
| Diferenciál | Změna rozdělení hnacího momentu podle možností použité simulace | Srovnání výjezdu ze zatáčky |

Konkrétní parametry se musí ověřit v použité implementaci Chaos Vehicles. Pokud vybraný typ diferenciálu neumožní požadované nastavení, návrh upravím a ověřím s vedoucím; pouhý název položky v obchodě nestačí.

## Průběžná evidence

U každého bodu povedu datum, commit, popis ověření a skutečný výsledek. Assety budu evidovat odděleně jako vlastní, převzaté nebo upravené s uvedením zdroje a licence. Fotografie zadání obsahuje osobní údaje a v tomto balíku dokumentace se na GitHub nekopíruje.
