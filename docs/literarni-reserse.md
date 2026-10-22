# Literární a technická rešerše: Nightshift Drift

Pracovní verze k první konzultaci, 22. října 2026. Text shrnuje prostudované veřejné technické zdroje a vyvozuje z nich postup pro projekt. Vlastní harmonogram je samostatně v [plánu realizace](harmonogram.md). Nejde o tvrzení, že plánované systémy již byly dokončeny.

## 1. Vymezení tématu a přístup k rešerši

Nightshift Drift bude automobilová hra propojující volnou jízdu, závodění, drift a ekonomický postup. Hráč bude plnit závody, získávat peníze a nakupovat vozidla a úpravy. Projekt navazuje na existující základ automobilu. Hlavním úkolem proto nebude vytvořit celý engine, ale propojit řízení, prostředí, rozhraní a postup hráče do funkčního celku.

Rešerše se zaměřuje na vozidlovou fyziku, tvorbu terénu, procedurální vegetaci, rozhraní, správu herních dat, ukládání, zvuk a přípravu spustitelné verze. Upřednostněna je dokumentace výrobce Unreal Engine, protože přímo popisuje použité nástroje. Obecný kontext procedurálního obsahu doplňuje úvod odborné publikace Shakerové, Togeliuse a Nelsona [1]. Dokumentace je průběžně aktualizovaná, proto bude při implementaci nutné ověřovat dostupnost konkrétních funkcí v instalované verzi enginu.

## 2. Procedurální obsah a ruční návrh

Úvod publikace *Procedural Content Generation in Games* představuje procedurální tvorbu jako oblast propojující algoritmy s návrhem herního obsahu. Rozlišování mezi automatickým generováním a návrhářským záměrem je pro tuto práci podstatné: samotný algoritmus nezaručí zajímavou závodní trať [1].

Pro Nightshift Drift z toho vyvozuji kombinovaný postup. Klíčové zatáčky, křižovatky a závodní trasy navrhnu ručně a ověřím jízdou. Automatizaci využiji především pro vegetaci a opakující se prvky prostředí. Náhodnost tak nebude rozhodovat o průjezdnosti tratě. Jde o vlastní návrhový závěr, nikoli o hotové řešení převzaté z knihy.

## 3. Vozidlová fyzika a drift

Chaos Vehicles je systém Unreal Engine pro simulaci vozidel. Dokumentace rozděluje problematiku na sestavení vozidla, jeho fyzikální nastavení, těžiště a diagnostiku [2]. Pro projekt je výhodné rozvíjet již použitý systém místo zavádění druhé, souběžné simulace.

Ve zdrojích projektu jsou parametry detekce smyku a asistencí řízení. Při dalším vývoji budu odlišovat realistický vzhled od přesnosti simulace. Cílem bude uvěřitelné chování, které umožní ovládat drift na běžném herním vstupu; nebudu bez měření označovat hru za fyzikálně přesný simulátor.

Nastavení budu porovnávat na stejné krátké trati: rozjezd, brzdění, ustálená zatáčka, vracečka, navazující smyky a návrat do přímého směru. Zaznamenám použité hodnoty a výsledek. Tím omezím změny prováděné pouze podle okamžitého pocitu.

## 4. Krajina a silniční síť

Systém Landscape je určen pro rozsáhlejší venkovní terén a jeho úpravy v Unreal Engine [3]. Pro výslednou mapu jej budu posuzovat jako hlavní základ krajiny. Experimentální generované meshe mohou pomoci ověřit objemy a vedení cest, ale samy o sobě nepředstavují dokončenou produkční krajinu.

Vlastní návrh prostředí vychází z japonských horských silnic: údolí, dva svahy, průsmyk a propojené serpentinové trasy. Přibližně čtyři kilometry na stranu jsou počáteční pracovní rozsah, nikoli neměnná podmínka. Konečnou velikost určím podle délky jízd, výkonu a času na zpracování.

Nejprve vytvořím krátký jízdní úsek. Až podle chování auta upravím šířku, poloměry zatáček a sklon. Pět plánovaných závodů může sdílet část cest, ale každý musí nabídnout odlišnou kombinaci zatáček nebo pravidel. Výzdoba nesmí zakrývat chyby ve vedení trasy.

## 5. Vegetace pomocí PCG

PCG Framework pracuje s prostorovými daty a grafem, který vytváří a upravuje body pro rozmístění obsahu. Body mohou nést informace například o transformaci, hustotě či náhodném seedu [4]. Výhodou pro tento projekt je možnost opakovaně upravovat rozmístění vegetace podle stejných pravidel.

Navrhnu filtr podle sklonu, výšky a vzdálenosti od silnice. V okolí vozovky, křižovatek a startů vytvořím vylučovací pásma. Nejprve ověřím jednu malou oblast a její výkon. Teprve potom rozšířím generování na větší část mapy. Pevný seed použiji pro reprodukovatelné porovnávání výsledků. Tato konkrétní pravidla jsou návrhem pro Nightshift Drift.

## 6. Uživatelské rozhraní

Pokyny Epic Games pro UMG upozorňují na náklady zbytečně vytvářených widgetů a doporučují dělit složitější rozhraní podle potřeby jeho používání [5]. Pro projekt z toho vyplývá oddělení jízdního HUD, nabídky závodů, garáže a nastavení.

HUD bude přednostně zobrazovat rychlost, převodový stupeň a podle režimu čas, kontrolní body nebo driftové skóre. Stav peněz a nákupní nabídky mají hlavní význam v garáži. Změny peněz či stavu závodu budou vhodné pro aktualizaci prostřednictvím událostí; rychle se měnící jízdní hodnoty potřebují přiměřeně častou obnovu. Čitelnost ověřím v pohybu, na světlém i tmavém pozadí a ve více rozlišeních.

## 7. Závody, odměny a data vozidel

Dokumentace datově řízeného vývoje popisuje Data Tables a Curve Tables jako prostředky pro oddělení parametrů od vlastní logiky. U Data Table odpovídá řádek definované datové struktuře [6]. Pro Nightshift Drift je tento přístup vhodný pro ceny, charakteristiky vozů, úpravy a definice závodů.

Vlastní závodní systém navrhnu jako posloupnost stavů: dostupný závod, odpočet, aktivní jízda, dokončení nebo zrušení. Kontrolní body budou mít pevné pořadí. Časový závod vyhodnotí čas, driftová výzva skóre. Odměnu přidělí jediný řídicí systém právě jednou pro konkrétní dokončený pokus. Konkrétní pravidla ani částky nejsou výsledkem převzatým z dokumentace; budu je navrhovat a ladit v projektu.

Garáž nabídne nejméně tři vozy a pět kategorií úprav: motor, pneumatiky, podvozek, brzdy a diferenciál. U každé úpravy určím fyzikální parametr, který se skutečně mění, a jízdní test pro ověření účinku. Pouhá změna obrázku nebo textu nebude považována za splnění požadavku na vliv úprav.

## 8. Ukládání postupu

Unreal Engine nabízí vlastní třídy SaveGame a synchronní i asynchronní ukládání. Dokumentace doporučuje asynchronní variantu pro ukládání při běžící hře, kde by zápis mohl způsobit zásek [7]. Volbu způsobu zápisu přizpůsobím velikosti a četnosti ukládaných dat.

Navrhnu ukládat peníze, vlastněné vozy, zakoupené a aplikované úpravy, vybrané vozidlo, výsledky závodů a bezpečnou pozici návratu. Nastavení automatického ukládání bude součástí nabídky. Formát doplním verzí dat, aby bylo možné rozpoznat budoucí změny struktury. Při chybě načtení musí hra nabídnout srozumitelný výsledek a nepřepsat nečitelný soubor automaticky prázdným postupem. Bezpečnou pozici při načtení ověřím zvlášť, aby se vůz neobjevil uvnitř terénu.

## 9. Zvuk a atmosféra

Přehled zvukového enginu popisuje nástroje pro přehrávání, prostorový zvuk a zpracování zvukových signálů [8]. Ve hře mají zvuky rovněž informační funkci: pomáhají rozpoznat otáčky motoru, prokluz nebo potvrzení akce.

Vlastní realizace bude navazovat na přítomné komponenty motoru a rádia. Doplním zpětnou vazbu závodu a rozhraní, samostatnou hlasitost hudby a efektů a ověřím smyčky během delší jízdy. Hudbu, zvuky i modely budu evidovat se zdrojem a licencí; dostupnost assetu v projektu sama o sobě neprokazuje oprávnění k jeho veřejnému šíření.

## 10. Testování, sestavení a verzování

Packaging zahrnuje sestavení kódu, přípravu obsahu pro cílovou platformu a vytvoření distribuovatelných souborů. Epic Games uvádí jeho využití i během výroby a testování [9]. Spustitelnou verzi proto neodložím až na poslední týden; bude součástí hlavních milníků.

Git LFS nahrazuje velké soubory v historii Gitu ukazateli a vlastní data ukládá odděleně [10]. Pro projekt již sleduje `.uasset` a `.umap`. Ověření repozitáře musí zahrnovat také dostupnost LFS obsahu, nikoli pouze přítomnost commitů.

Každou větší funkci ověřím herním scénářem a zaznamenám výsledek. Zvláštní pozornost dostane zkrácení tratě, opakované projetí cílem, zrušení závodu, nedostatek peněz a opětovné spuštění hry po uložení. Změny budou mít malé, popisné commity. Pokus o vytvoření mapy dne 22. října skončil pádem editoru, proto jej nelze vykazovat jako hotovou mapu.

## 11. Závěr rešerše a volba postupu

Zdroje poskytují technický základ pro simulaci vozidla, prostředí, rozhraní i perzistentní data. Neřeší však za autora konkrétní vedení silnic, obtížnost driftu nebo vyvážení ekonomiky. Tyto části budu rozhodovat pomocí malých prototypů a opakovaných herních testů.

Nejdříve propojím krátkou trať s jedním závodem, odměnou a uložením. Poté rozšířím obsah na další závody a vozidla. Realistické prostředí budu dopracovávat až nad ověřenou hratelností, aby náročná vegetace a materiály nepohltily čas potřebný ke splnění povinných systémů.

## Použité zdroje

Číselné odkazy v textu odpovídají seznamu níže. Online zdroje citovány 22. 10. 2026. Formát citací je pracovní; konečnou školní úpravu ověřím s vedoucím. U knihy [1] byl pro tuto verzi použit veřejný úvod, nikoli deklarováno prostudování celé publikace.

1. SHAKER, Noor; TOGELIUS, Julian; NELSON, Mark J. *Procedural Content Generation in Games: A Textbook and an Overview of Current Research*. Springer, 2016. [Stránka knihy](https://www.pcgbook.com/), [veřejný úvod](https://www.pcgbook.com/chapter01.pdf).
2. EPIC GAMES. *Chaos Vehicles*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/unreal-engine/chaos-vehicles).
3. EPIC GAMES. *Landscape Outdoor Terrain*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-outdoor-terrain-in-unreal-engine).
4. EPIC GAMES. *Procedural Content Generation Overview*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-overview).
5. EPIC GAMES. *Optimization Guidelines for UMG*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine?lang=en-US).
6. EPIC GAMES. *Data Driven Gameplay Elements*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine).
7. EPIC GAMES. *Saving and Loading Your Game*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine?lang=en-US).
8. EPIC GAMES. *Audio Engine Overview*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/en-us/unreal-engine/audio-engine-overview-in-unreal-engine).
9. EPIC GAMES. *Packaging Your Project*. Unreal Engine Documentation. Online. [Dostupné zde](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project).
10. GIT LFS PROJECT. *Git Large File Storage*. Oficiální dokumentace projektu. Online. [Dostupné zde](https://git-lfs.com/).
