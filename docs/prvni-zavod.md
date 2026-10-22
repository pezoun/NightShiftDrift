# První implementační cíl: jeden úplný závod

Návrh k 22. 10. 2026. Tento dokument specifikuje budoucí funkci; závodní systém tímto ještě není implementován.

## Proč začít tady

První závod propojí existující vozidlo s hlavní herní smyčkou a přímo pokryje bod 5 školního zadání. Krátká zkušební trať umožní otestovat logiku bez závislosti na dokončení velké realistické mapy. První varianta bude časová, protože na ní jednoduše ověřím pořadí kontrolních bodů. Následující varianta využije stejný průběh pro driftové skóre; výsledná hra zůstává zaměřená hlavně na drift.

## Rozsah první verze

- Jedna krátká trať se startem, třemi kontrolními body a cílem.
- Aktivace závodu pouze hráčovým vozem ve startovní oblasti a potvrzením hráče.
- Třísekundový odpočet a následné měření času.
- Přijímání kontrolních bodů pouze ve správném pořadí a od aktivního vozidla.
- HUD s časem a údajem „kontrolní bod 1/3“.
- Výsledková obrazovka a jednorázová odměna; výchozí testovací hodnota 500 herních jednotek, nikoli finální vyvážení.
- Možnost závod zrušit a opakovat jako nový pokus.

## Stavy a odpovědnosti

| Stav | Vstup | Výsledek |
|---|---|---|
| Volná jízda | Potvrzení startu v oblasti závodu | Nové ID pokusu, vynulování času, odpočet |
| Odpočet | Uplynutí 3 sekund | Zahájení jízdy a měření času |
| Aktivní závod | Správný další bod | Posun očekávaného kontrolního bodu |
| Aktivní závod | Cíl po všech bodech | Ukončení měření, výsledek a odměna právě jednou |
| Odpočet / aktivní závod | Zrušení nebo reset vozidla | Zrušený pokus bez odměny, návrat do volné jízdy |
| Dokončeno / zrušeno | Nový potvrzený start | Nový nezávislý pokus |

Řídicí objekt závodu bude jediným vlastníkem stavu a vyhodnocení dokončení. Kontrolní body pouze oznámí průjezd. Ekonomický systém provede připsání peněz na základě dokončeného pokusu. UI zobrazí stav prostřednictvím událostí. Ukládání se připojí až po stabilním průběhu závodu, se stejnými identifikátory výsledků a měny.

Systém musí filtrovat opakované overlap události komponent stejného vozu, průjezd cizího objektu a již dokončený pokus. Pozastavení hry nesmí přidávat čas do závodu. Pro první verzi bude reset vozidla závod rušit; pozdější návrat na poslední kontrolní bod je možné rozšíření.

## Rozšíření o drift

Body se budou připisovat jen v aktivním závodě, při jízdě po platné trase a při splnění podmínek smyku a minimální rychlosti. Detekce bude vycházet z ověřené telemetrie. Konkrétní vzorec, násobič a penalizace se navrhnou po změření chování auta. Kontrolní body zůstanou povinné, aby hráč nemohl získávat výsledek pouze kroužením na startu.

## Ověřovací scénáře

Všechny scénáře mají při vytvoření dokumentu stav **neprovedeno**.

| ID | Scénář | Očekávaný výsledek |
|---|---|---|
| R01 | Start, 3 body v pořadí, cíl | Čas se zastaví, odměna +500 právě jednou |
| R02 | Vynechat bod 2 a projet cílem | Závod se nedokončí, odměna 0 |
| R03 | Vrátit se do již projetého bodu | Postup ani odměna se nezmění |
| R04 | Projet cílem opakovaně po dokončení | Žádná další odměna |
| R05 | Dva overlapy stejného vozu v jednom bodě | Jediný posun postupu |
| R06 | Zrušit závod během odpočtu | Žádný aktivní časovač ani odměna |
| R07 | Resetovat vůz během závodu | Pokus se zruší, odměna 0 |
| R08 | Pozastavit na 10 sekund | Čas závodu se během pauzy nezvyšuje |
| R09 | Dokončit, spustit nový pokus a dokončit | Dva oddělené výsledky, celkem +1000 |
| R10 | Cizí objekt projede bodem | Žádný posun postupu hráče |
| R11 | Pokus o druhý souběžný závod | Spuštění je odmítnuto nebo vyžaduje zrušení prvního |
| R12 | Po zapojení SaveGame restartovat hru | Odměna a výsledek zachovány, starý pokus odměnu nezopakuje |

Hotovo bude znamenat implementaci, úspěšné herní testy a zaznamenané výsledky v konkrétním commitu; nikoli pouze existenci tříd nebo tohoto návrhu.
