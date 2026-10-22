# Plán realizace: říjen 2026 – duben 2027

Stav plánu: 22. 10. 2026. První konzultace: 23. 10. 2026.

**Oficiální termín na fotografii zadání je 16. 4. 2027 do 14:00 na sekretariátu školy.** Datum 17. 4. je pozdější; tento plán s ním jako termínem odevzdání nepočítá. Interní uzávěrka bude 9. 4., poslední týden zůstane na kontrolu a předání.

## Co chci vytvořit

Vytvořím realisticky působící automobilovou hru pro jednoho hráče na Windows. Základem bude volná jízda po horské oblasti inspirované Japonskem. Hráč bude spouštět driftové výzvy a časové závody, získávat peníze a nakupovat auta a úpravy. Plánuji nejméně pět závodních událostí, tři vozidla a pět kategorií vylepšení.

Pět závodů je vlastní obsahový cíl; školní zadání výslovně stanovuje minimum tří vozidel a pěti druhů vylepšení, nikoli pět závodů. Rozsah prostředí budu přizpůsobovat času a výkonu. Multiplayer, doprava řízená AI, dynamické počasí a rozsáhlé město nejsou součástí základního plánu. Druh soupeření u závodů potvrdím na konzultaci; základní návrh počítá s časem a skóre bez AI soupeřů.

## Prosincový milník: přibližně polovina projektu

Kontrolní datum stanovím na **18. 12. 2026**, aby byl čas na opravy ještě před koncem prosince. „Polovina“ znamená odhad vážených pracovních balíků a fungující základní herní smyčku, nikoli polovinu rozlohy hotové mapy. Nejde o školní hodnocení ani přesnou míru již odvedené práce.

| Balík | Váha celého projektu | Cíl splnění balíku k 18. 12. | Příspěvek |
|---|---:|---:|---:|
| Rešerše, návrh a průběžná dokumentace | 10 % | 60 % | 6 % |
| Jízdní vlastnosti a jejich ověření | 15 % | 80 % | 12 % |
| Mapa a pět závodních událostí | 20 % | 50 % | 10 % |
| Závodní systém a odměny | 15 % | 80 % | 12 % |
| Menu a HUD | 10 % | 50 % | 5 % |
| Ukládání a automatické ukládání | 10 % | 50 % | 5 % |
| Garáž, tři vozy a pět kategorií úprav | 15 % | 0 % | 0 % |
| Finální zvuk, optimalizace, build a video | 5 % | 0 % | 0 % |
| **Součet plánovaného postupu** | **100 %** | | **50 %** |

Základní zvuky a průběžné buildy budou vznikat dříve; poslední řádek hodnotí jejich finální dokončení. Existující vozidlový základ se nejprve ověří, nepočítá se automaticky jako otestovaný výsledek.

### Podmínky prosincového předvedení

- Z menu spustím volnou jízdu a mohu hru pozastavit a opustit.
- Projedu souvislý testovací úsek nové mapy s funkční kolizí a respawnem.
- Dokončím alespoň jeden driftový a jeden časový závod; zbývající tři mají navrženou trasu.
- Kontrolní body hlídají pořadí a po dokončení dostanu odměnu právě jednou za pokus.
- HUD rozlišuje časový a driftový režim.
- Peníze a alespoň výsledek závodu se zachovají po restartu hry; existuje základ nastavení automatického ukládání.
- Vytvořím a spustím testovací Windows build. Zaznamenám známé chyby a výsledek testu.

Pokud některá podmínka chybí, uvedu ji jako nedokončenou; procento samotné ji nenahradí.

## Etapy a termíny

| Období | Co budu dělat | Ověřitelný výstup |
|---|---|---|
| 22.–23. 10. 2026 | Zpracuji zadání, rešerši, priority a stav projektu. | Podklady na GitHubu, otázky pro vedoucího. |
| 24. 10.–6. 11. | Ověřím současné řízení, kameru, zvuk, respawn a vytvořím krátkou testovací trať. Zkusím první build. | Záznam jízdních testů, uložená testovací mapa, výsledek sestavení. |
| 7.–20. 11. | Implementuji jeden závod: start, odpočet, kontrolní body, čas/skóre, konec a odměna. | Dokončitelný závod bez opakovaného připsání odměny. |
| 21. 11.–4. 12. | Propojím HUD a menu se závodem, přidám základ SaveGame. Rozpracuji druhou trasu. | Odměna a výsledek přežijí ukončení hry. |
| 5.–18. 12. | Doplním druhý režim závodu, první oblast terénu a malý PCG test. Opravím chyby smyčky. | Prosincové předvedení podle seznamu výše. |
| 19.–31. 12. | Vyhradím rezervu, uspořádám dokumentaci a ověřím obnovitelnost projektu. | Zapsaný skutečný stav, plán oprav, záložní build. |
| 1.–17. 1. 2027 | Přidám katalog a nákup vozů, vlastnictví, výběr a uložení. | Tři dostupná vozidla s rozlišitelným nastavením. |
| 18.–31. 1. | Implementuji pět kategorií úprav a jejich dopad do fyziky, ceny a kontroly nákupu. | Nákup, aplikace, uložení a jízdní srovnání úprav. |
| 1.–14. 2. | Dokončím všech pět závodních událostí a propojení cest, doplním bezpečné respawny. | Všechny závody dokončitelné ze hry. |
| 15.–28. 2. | Dopracuji realistické materiály, vegetaci PCG, značení, zvuky a osvětlení. | Souvislá reprezentativní mapa a beta build. |
| 1.–14. 3. | Provedu systematické testování, profilování, kontrolu úspor a vyvážení cen/odměn. | Testovací protokol a opravené závažné chyby. |
| 15.–28. 3. | Uzavřu obsah, dopíšu dokumentaci, citace a seznam licencí. Připravím video. | Kandidát finální verze a úplný text práce. |
| 29. 3.–9. 4. | Ověřím instalaci a spuštění mimo editor, dokončím video a prezentační podklady. | Finální hra, dokumentace, video a záloha k 9. 4. |
| 10.–15. 4. | Opravím jen kritické chyby, zkontroluji soubory a formu předání se školou. | Připravený odevzdávací balíček. |
| **16. 4. do 14:00** | **Odevzdám práci podle školního zadání.** | **Předaná práce.** |

## Pracovní rytmus a kontrola rozsahu

Jako počáteční kapacitu plánuji přibližně 10–12 hodin týdně rozdělených mezi implementaci, herní test a dokumentaci. Tento odhad musím po prvních dvou týdnech porovnat se skutečně dostupným časem. Při menší kapacitě zmenším plochu mapy a množství dekorací, zachovám však povinné funkce.

Každý týden dokončím jeden malý ověřitelný výstup, zapíšu test a provedu commit. U větších milníků připravím build a krátký záznam. Průběžně povedu tabulku assetů a licencí a doplním zdroje do textu. Před větší úpravou mapy ji uložím a ověřím opětovné otevření.

Priorita je: stabilní jízda → první kompletní závod → odměna a uložení → garáž a úpravy → rozšíření obsahu → finální vzhled. Detailní les ani velká mapa nesmějí blokovat funkční závod a ukládání. Nové velké funkce po 28. 3. nepřidávám.

## Hlavní rizika

| Riziko | Jak ho budu omezovat |
|---|---|
| Příliš velká nebo drahá mapa | Nejdřív krátká trať, potom jedna oblast; rozsah zvýším až podle výkonu. |
| Nestabilní editor či plugin | Pravidelné ukládání a kontrolované změny, žádná migrace enginu těsně před odevzdáním. |
| Dlouhé ladění fyziky | Stejné jízdní scénáře, zapisované parametry, omezený čas každé iterace. |
| Garáž navázaná pozdě na save | Identifikátory vozů a úprav navrhnu před implementací ukládání. |
| Chybný nákup nebo opakovaná odměna | Výslovné testy financí a stavu pokusu. |
| Nedostupná práva k assetům | Evidence zdrojů a podmínek před veřejným sdílením a odevzdáním. |
| Záloha bez velkých souborů | Kontrola LFS přenosu a zkušební otevření z obnovené kopie. |

Termíny konzultací kromě první nejsou potvrzené; navržené kontrolní datum 18. 12. projednám s vedoucím.
